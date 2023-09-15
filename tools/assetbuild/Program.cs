// Assetbuild Utility
// Copyright © 2023 starfrost
// 
// This is a mess primarily due to the fact it was rushed,
// and due to the fact pak.exe sucks ass
using System.Diagnostics;

#region Constants & Variables

const string ASSETBUILD_VERSION = "1.5.0"; // Version
const string TOOLDIR = @"..\..\..\..\..\tools"; // Tool compile directory relative to the Assetbuild CWD (janky)
const string DEFAULT_GAME_NAME = "zombono"; // Default engine game name folder to use

string[] validConfigs = { "Debug", "Release", "Playtest" }; // Valid build configs.
string config = validConfigs[0]; // Current config.
string gameName = DEFAULT_GAME_NAME; // Name of the game to compile.
string gameDir = $@"..\..\..\..\..\game\{gameName}"; // Complete relative path to game dir
string cfgDir = $@"{gameDir}\basecfg"; // Config dir.
string pak0Dir = $@"..\..\..\..\..\game\{gameName}\content"; // Package 0 dir. (relative to mkpak build directory - also janky)
string qcDir = $@"{gameDir}\qc"; // QuakeC sources dir.
string gfxSourceDir = $@"{gameDir}raw\content\gfx"; // gfx source dir.
string gfxDestinationDir = $@"{gameDir}\content\gfx"; // gfx destination dir
string finalDir = string.Empty; //set later

bool quietMode = false; // If true, everything except errors are hushed

#endregion

// Localise here!
#region Strings
const string STRING_SIGNON = $"Assetbuild {ASSETBUILD_VERSION}";
const string STRING_DESCRIPTION = "Builds assets for Zombono";
const string STRING_USAGE = "Assetbuild <game> [release cfg] [-q]\n\n" +
    "<game>: Path to directory contaning game files\n" +
    "[directory]: Optional - base directory (default is ../../../../../game) ('raw' after it for gfx files) \n" +
    "[release cfg]: Optional - release config (case insensitive), valid: Debug, Playtest, Release - this config is ALSO used for the asset build tools, so make sure to recompile them for ALL 3 configs when changing them!\n" +
    "[-q]: Optional - quiets everything except errors";
const string STRING_BUILDING_GFX = "Building LMP files (using bmp2lmp)...";
const string STRING_BUILDING_BASECFG = "Building configuration files...";
const string STRING_BUILDING_QC = "Building QuakeC source...";
const string STRING_BUILDING_PAK0 = "Building package 0...";
const string STRING_BUILDING_DONE = "Done!";

const string STRING_ERROR_NO_CONFIG = "Invalid build config provided (valid options: Debug, Playtest, Release)";
      string STRING_ERROR_NO_GAMEDIR = $"The base directory {gameDir} does not exist!"; // includes variable in variables section
const string STRING_ERROR_FTEQCC = "An error occurred while running FTEQCC!";
const string STRING_ERROR_BMP2LMP = "An error occurred while running bmp2lmp";
const string STRING_ERROR_MKPAK = "An error occurred while running mkpak!";
const string STRING_ERROR_GENERIC = "An exception occurred: ";
#endregion

try
{
    // stupid hack for running under MSBuild
    Directory.SetCurrentDirectory(AppDomain.CurrentDomain.BaseDirectory);

    Print(STRING_SIGNON);
    Print(STRING_DESCRIPTION);

    #region Command-line parsing

    if (args.Length < 1)
    {
        gameName = DEFAULT_GAME_NAME;
    }

    if (args.Length >= 1)
    {
        if (!Directory.Exists(gameDir))
        {
            PrintLoud(STRING_ERROR_NO_GAMEDIR);
            PrintHelpAndExit(2);
        }

        gameName = args[0];
    }
    
    if (args.Length >= 2)
    {
        if (!validConfigs.Contains(args[1], StringComparer.InvariantCultureIgnoreCase))
        {
            PrintLoud(STRING_ERROR_NO_CONFIG);
            PrintHelpAndExit(4);
        }

        config = args[1];
    }

    // set the final directory
    finalDir = $@"..\..\..\..\..\engine\build\{config}\bin\{gameName}"; // final directory

    // could be first build of a new game so just create final dir if it exists
    if (!Directory.Exists(finalDir)) Directory.CreateDirectory(finalDir);
    #endregion

    #region Main code

    Console.WriteLine(STRING_BUILDING_QC); // progs.dat/qwprogs.dat -> root of pak0

    // until we have our own fteqcc fork
    Process process = new();
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
        PrintErrorAndExit(STRING_ERROR_FTEQCC, 6);
    }

    File.Move($@"{qcDir}\progs.dat", $@"{pak0Dir}\progs.dat", true); // copy to pak0 where it belongs (relative to thist ool. need to make it all absolute)
     
    Print(STRING_BUILDING_GFX);

    process = new();

    process.StartInfo.FileName = $@"{TOOLDIR}\bmp2lmp\bin\{config}\net7.0\bmp2lmp.exe";
    process.StartInfo.ArgumentList.Add(Path.GetFullPath(gfxSourceDir));
    process.StartInfo.ArgumentList.Add(Path.GetFullPath(gfxDestinationDir));
    process.StartInfo.ArgumentList.Add("-q");

    process.Start();
    process.WaitForExit();

    if (process.ExitCode != 0)
    {
        PrintErrorAndExit(STRING_ERROR_BMP2LMP, 9);
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

    // Lists don't work because the dirs are all wrong???

    Print(STRING_BUILDING_PAK0);
    // create process (use same variable for optimisation
    process = new();
    process.StartInfo.FileName = Path.GetFullPath($"{TOOLDIR}\\mkpak\\build\\{config}\\mkpak.exe"); // for some reason it loves to append ..\..\..\ to everything unles you do this
    process.StartInfo.WorkingDirectory = Path.GetFullPath($@"{pak0Dir}");

    File.Delete($"{finalDir}\\pak0.pak");
    File.Delete($"{pak0Dir}\\pak0.pak");

    process.StartInfo.ArgumentList.Add(Path.GetFullPath($"{pak0Dir}"));
    process.StartInfo.ArgumentList.Add("pak0.pak");

    process.Start();
    process.WaitForExit();

    if (process.ExitCode != 0)
    {
        PrintErrorAndExit(STRING_ERROR_MKPAK, 7);
    }

    // MKPak doesn't do this yet
    File.Move($@"{pak0Dir}\pak0.pak", $@"{finalDir}\pak0.pak");

    /*
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
            PrintErrorAndExit(STRING_ERROR_MKPAK, 7);
        }
    }
    */

    Console.ForegroundColor = ConsoleColor.Green;
    Print(STRING_BUILDING_DONE);
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
    PrintErrorAndExit($"{STRING_ERROR_GENERIC} \n\n{ex}", 8);
}

void PrintLoud(string text, ConsoleColor foreground = ConsoleColor.Gray)
{
    Console.ForegroundColor = foreground;
    Console.WriteLine(text);
    Console.ResetColor();
}

void Print(string text, ConsoleColor foreground = ConsoleColor.Gray)
{
    if (!quietMode) PrintLoud(text, foreground);
}

void PrintErrorAndExit(string errorString, int errorId)
{
    Console.ForegroundColor = ConsoleColor.Red;
    Console.WriteLine($"Error: {errorString}");
    Console.ResetColor();
    Environment.Exit(errorId);
}

