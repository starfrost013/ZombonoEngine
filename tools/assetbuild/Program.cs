// See https://aka.ms/new-console-template for more information

// This is a mess primarily due to the fact it was rushed,
// and due to the fact pak.exe sucks ass

#region Constants & Variables
using System.Diagnostics;

const string ASSETBUILD_VERSION = "1.2.0";
const string TOOLDIR = @"..\..\..\..\tools";
const string DEFAULT_GAME_NAME = "zombono";

string[] validConfigs = { "Debug", "Release" }; // Valid build configs.
string config = validConfigs[0]; // Current config.
string gameName = DEFAULT_GAME_NAME; // Name of the game to compile.
string gameDir = $@"..\..\..\..\game\{gameName}"; // Complete relative path to game dir
string cfgDir = $@"{gameDir}\basecfg"; // Config dir.
// temp - pak0 and pak1 likely be merged
string pak0Dir = $@"{gameDir}\content"; // Package 0 dir.
string qcDir = $@"{gameDir}\qc"; // QuakeC sources dir.
string gfxDir = $@"{gameDir}\gfx"; // GFX.WAD source dir.
string finalDir = $@"..\..\..\..\build\{config}\bin\{gameName}"; // final directory
#endregion

#region Strings
const string STRING_SIGNON = $"Assetbuild {ASSETBUILD_VERSION}";
const string STRING_DESCRIPTION = $"Builds assets for Zombono";
const string STRING_USAGE = $"Assetbuild <game> [release cfg]\n\n" +
    $"<game>: Path to directory contaning game files\n" +
    $"[directory]: Optional - base directory (default is ../../../../game/)\n" +
    $"[release cfg]: Optional - release config (case insensitive), valid: Debug, Release";
const string STRING_BUILDING_GFX = "Building gfx.wad";
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
    }

    // could be first build of a new game so just create final dir if it exists
    if (!Directory.Exists(finalDir)) Directory.CreateDirectory(finalDir);   
    #endregion

    #region Main code

    /*
    does not generate valid gfx.wad files, and since we're removing it best to just work with what exists rn
    Console.WriteLine(STRING_BUILDING_GFX);

    // get all files in gfx dir
    string[] gfxFiles = Directory.GetFiles(gfxDir, "*.*", SearchOption.AllDirectories);

    // WHY DOES IT HAVE A PROPRIETARY FUCKING LIST FORMAT
    Process procGfxConv = new();
    procGfxConv.StartInfo.FileName = $"{TOOLDIR}\\pypaktools\\wad.exe";

    if (File.Exists($@"{pak0Dir}\gfx.wad")) File.Delete($@"{pak0Dir}\gfx.wad");

    foreach (string gfxFile in gfxFiles)
    {
        Console.WriteLine($"Replacing files...");

        procGfxConv.StartInfo.ArgumentList.Clear();
        procGfxConv.StartInfo.ArgumentList.Add($"{pak0Dir}\\gfx.wad");
        procGfxConv.StartInfo.ArgumentList.Add($"{gfxFile}");

        if (gfxFile.Contains(".lmp", StringComparison.InvariantCultureIgnoreCase)) procGfxConv.StartInfo.ArgumentList.Add("-tQPIC");

        procGfxConv.Start();
        procGfxConv.WaitForExit();

        if (procGfxConv.ExitCode != 0)
        {
            PrintErrorAndExit($"Error replacing gfx.wad!", 5);
        }
    }
    */

    Console.WriteLine(STRING_BUILDING_BASECFG);

    // get all files in gfx dir
    string[] baseCfgFiles = Directory.GetFiles(cfgDir, "*.*", SearchOption.AllDirectories);

    foreach (string baseCfgFile in baseCfgFiles)
    {
        int start = baseCfgFile.LastIndexOf(Path.DirectorySeparatorChar);
        int end = baseCfgFile.Length;

        string justFileName = baseCfgFile.Substring(start, end - start);
        File.Copy(baseCfgFile, $@"{finalDir}\{justFileName}", true);
    }

    Console.WriteLine(STRING_BUILDING_QC); // progs.dat/qwprogs.dat -> root of pak0

    // until we have our own fteqcc fork
    Process procFteqcc = new();
    procFteqcc.StartInfo.FileName = $@"{TOOLDIR}\fteqcc64.exe";
    procFteqcc.StartInfo.WorkingDirectory = qcDir; // must be set (-src doesn't work with relative paths???)
    procFteqcc.StartInfo.ArgumentList.Add("-O2"); // -O3 not recommended
    procFteqcc.StartInfo.ArgumentList.Add("-Dzombono=1"); // Set zombono
    procFteqcc.StartInfo.ArgumentList.Add("-Tstandard"); // Set 'standard' QC type
    procFteqcc.StartInfo.ArgumentList.Add("-Wno-mundane"); // Remove mundane warnings

    procFteqcc.Start();
    procFteqcc.WaitForExit();

    if (procFteqcc.ExitCode != 0)
    {
        PrintErrorAndExit("An error occurred while running FTEQCC", 6);
    }

    File.Move($@"{qcDir}\progs.dat", $@"{pak0Dir}\progs.dat", true); // copy to pak0 where it belongs

    // Lists don't work because the dirs are all wrong???

    Console.WriteLine(STRING_BUILDING_PAK0);
    // get temp file name for list (easier to just replace files for wad2)

    string[] pak0Files = Directory.GetFiles(pak0Dir, "*.*", SearchOption.AllDirectories);

    // create process
    Process procPaktool = new();
    procPaktool.StartInfo.FileName = Path.GetFullPath($"{TOOLDIR}\\pypaktools\\pak.exe"); // for some reason it loves to append ..\..\..\ to everything unles you do this
    procPaktool.StartInfo.WorkingDirectory = Path.GetFullPath($"{pak0Dir}");

    File.Delete($"{finalDir}\\pak0.pak");

    foreach (string pak0File in pak0Files)
    {
        // stupid stupid tool breaks the entire fucking game if it's not precisely right (TODO: WRITE NON SHITTY REPLACEMENT!!!)

        string pak0FileNonFucked = pak0File.Replace(@"..\", "");
        pak0FileNonFucked = pak0FileNonFucked.Replace($@"game\{gameName}\content\", "");

        procPaktool.StartInfo.ArgumentList.Clear();
        procPaktool.StartInfo.ArgumentList.Add(Path.GetFullPath($@"{finalDir}\pak0.pak"));
        procPaktool.StartInfo.ArgumentList.Add($@"{pak0FileNonFucked}");

        Trace.WriteLine(procPaktool.StartInfo.Arguments);

        procPaktool.Start();
        procPaktool.WaitForExit();

        if (procPaktool.ExitCode != 0)
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