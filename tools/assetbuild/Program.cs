// See https://aka.ms/new-console-template for more information

// This is a mess primarily due to the fact it was rushed,
// and due to the fact pak.exe sucks ass
using System.Diagnostics;

#region Constants & Variables


const string ASSETBUILD_VERSION = "1.3.1";
const string TOOLDIR = @"..\..\..\..\..\tools";
const string DEFAULT_GAME_NAME = "zombono";

string[] validConfigs = { "Debug", "Release", "Playtest" }; // Valid build configs.
string config = validConfigs[0]; // Current config.
string gameName = DEFAULT_GAME_NAME; // Name of the game to compile.
string gameDir = $@"..\..\..\..\..\game\{gameName}"; // Complete relative path to game dir
string cfgDir = $@"{gameDir}\basecfg"; // Config dir.
// temp - pak0 and pak1 likely be merged
string pak0Dir = $@"{gameDir}\content"; // Package 0 dir.
string qcDir = $@"{gameDir}\qc"; // QuakeC sources dir.
string gfxSourceDir = $@"{gameDir}raw\content\gfx"; // gfx source dir.
string gfxDestinationDir = $@"{gameDir}\content\gfx"; // gfx destination dir
string finalDir = string.Empty; //set later

#endregion

#region Strings
const string STRING_SIGNON = $"Assetbuild {ASSETBUILD_VERSION}";
const string STRING_DESCRIPTION = $"Builds assets for Zombono";
const string STRING_USAGE = $"Assetbuild <game> [release cfg]\n\n" +
    $"<game>: Path to directory contaning game files\n" +
    $"[directory]: Optional - base directory (default is ../../../../../game) ('raw' after it for gfx files) \n" +
    $"[release cfg]: Optional - release config (case insensitive), valid: Debug, Release - debug tools are used for debug, release for release";
const string STRING_BUILDING_GFX = "Converting gfx to LMP32...";
const string STRING_BUILDING_BASECFG = "Building configuration...";
const string STRING_BUILDING_QC = "Building QuakeC...";
const string STRING_BUILDING_PAK0 = "Building package 0...";
const string STRING_BUILDING_DONE = "Done!";

#endregion

try
{
    // stupid hack for running under MSBuild
    Directory.SetCurrentDirectory(AppDomain.CurrentDomain.BaseDirectory);

    Console.WriteLine(STRING_SIGNON);
    Console.WriteLine(STRING_DESCRIPTION);

    #region Command-line parsing

    if (args.Length < 1)
    {
        gameName = DEFAULT_GAME_NAME;
    }

    if (args.Length >= 1)
    {
        if (!Directory.Exists(gameDir))
        {
            Console.WriteLine($"The base directory {gameDir} does not exist!");
            PrintHelpAndExit(2);
        }

        gameName = args[0];
    }
    
    if (args.Length >= 2)
    {
        if (!validConfigs.Contains(args[1], StringComparer.InvariantCultureIgnoreCase))
        {
            Console.WriteLine($"Invalid build config provided (valid options: Debug, Release)");
            PrintHelpAndExit(4);
        }

        config = args[1];
    }

    // set the final directory
    finalDir = $@"..\..\..\..\..\build\{config}\bin\{gameName}"; // final directory

    // could be first build of a new game so just create final dir if it exists
    if (!Directory.Exists(finalDir)) Directory.CreateDirectory(finalDir);
    #endregion

    #region Main code

    Console.WriteLine(STRING_BUILDING_GFX);

    Process process = new();

    process.StartInfo.FileName = $@"{TOOLDIR}\bmp2lmp\bin\{config}\net7.0\bmp2lmp.exe";
    process.StartInfo.ArgumentList.Add(Path.GetFullPath(gfxSourceDir));
    process.StartInfo.ArgumentList.Add(Path.GetFullPath(gfxDestinationDir));

    process.Start();
    process.WaitForExit();

    if (process.ExitCode != 0)
    {
        PrintErrorAndExit($"An error occurred while running bmp2lmp", 9);
    }

    Console.WriteLine(STRING_BUILDING_BASECFG);

    // get all files in gfx dir
    string[] baseCfgFiles = Directory.GetFiles(cfgDir, "*.*", SearchOption.AllDirectories);

    foreach (string baseCfgFile in baseCfgFiles)
    {
        int start = baseCfgFile.LastIndexOf(Path.DirectorySeparatorChar);
        int end = baseCfgFile.Length;

        string justFileName = baseCfgFile[start..end];
        File.Copy(baseCfgFile, $@"{finalDir}\{justFileName}", true);
    }

    Console.WriteLine(STRING_BUILDING_QC); // progs.dat/qwprogs.dat -> root of pak0

    // until we have our own fteqcc fork
    process = new(); 
    process.StartInfo.FileName = $@"{TOOLDIR}\fteqcc64.exe";
    process.StartInfo.WorkingDirectory = qcDir; // must be set (-src doesn't work with relative paths???)
    process.StartInfo.ArgumentList.Add("-O2"); // -O3 not recommended
    process.StartInfo.ArgumentList.Add("-Dzombono=1"); // Set zombono
    process.StartInfo.ArgumentList.Add("-Tstandard"); // Set 'standard' QC type
    process.StartInfo.ArgumentList.Add("-Wno-mundane"); // Remove mundane warnings

    process.Start();
    process.WaitForExit();

    if (process.ExitCode != 0)
    {
        PrintErrorAndExit("An error occurred while running FTEQCC", 6);
    }

    File.Move($@"{qcDir}\progs.dat", $@"{pak0Dir}\progs.dat", true); // copy to pak0 where it belongs

    // Lists don't work because the dirs are all wrong???

    Console.WriteLine(STRING_BUILDING_PAK0);
    // get temp file name for list (easier to just replace files for wad2)

    string[] pak0Files = Directory.GetFiles(pak0Dir, "*.*", SearchOption.AllDirectories);

    // create process (use same variable for optimisation
    process = new();
    process.StartInfo.FileName = Path.GetFullPath($"{TOOLDIR}\\pypaktools\\pak.exe"); // for some reason it loves to append ..\..\..\ to everything unles you do this
    process.StartInfo.WorkingDirectory = Path.GetFullPath($"{pak0Dir}");

    File.Delete($"{finalDir}\\pak0.pak");

    foreach (string pak0File in pak0Files)
    {
        // stupid stupid tool breaks the entire fucking game if it's not precisely right (TODO: WRITE NON SHITTY REPLACEMENT!!!)

        string pak0FileNonFucked = pak0File.Replace(@"..\", "");
        pak0FileNonFucked = pak0FileNonFucked.Replace($@"game\{gameName}\content\", "");

        process.StartInfo.ArgumentList.Clear();
        process.StartInfo.ArgumentList.Add(Path.GetFullPath($@"{finalDir}\pak0.pak"));
        process.StartInfo.ArgumentList.Add($@"{pak0FileNonFucked}");

        Trace.WriteLine(process.StartInfo.Arguments);

        process.Start();
        process.WaitForExit();

        if (process.ExitCode != 0)
        {
            PrintErrorAndExit("Pak0.pak creation failed!", 7);
        }
    }

    Console.ForegroundColor = ConsoleColor.Green;
    Console.WriteLine(STRING_BUILDING_DONE);
    Console.ResetColor();
    #endregion

    #region Utility functions

    void PrintHelpAndExit(int exitCode)
    {
        Console.WriteLine(STRING_USAGE);
        Environment.Exit(exitCode);
    }
    #endregion
}
catch (Exception ex)
{
    PrintErrorAndExit($"Exception occurred: \n\n{ex}", 8);
}

void PrintErrorAndExit(string errorString, int errorId)
{
    Console.ForegroundColor = ConsoleColor.Red;
    Console.WriteLine($"Error: {errorString}");
    Console.ResetColor();
    Environment.Exit(errorId);
}