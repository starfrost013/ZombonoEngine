// See https://aka.ms/new-console-template for more information

#region Constants & Variables
const string ASSETBUILD_VERSION = "1.0.0";
const string TOOLDIR = @"..\..\..\tools";

string[] validConfigs = { "Debug", "Release " };
string config = validConfigs[0];
string gameDir = @"..\..\..\..\game";
string gameCfgDir = $@"{gameDir}\basecfg";
// temp - pak0 and pak1 likely be merged
string pak0Dir = $@"{gameDir}\content0";
string pak1Dir = $@"{gameDir}\content1";
string qcDir = $@"{gameDir}\qc";
string gfxDir = $@"{gameDir}\gfx";
string finalDir = $@"..\..\..\..\build\{config}\bin\";
#endregion

#region Strings
const string STRING_SIGNON = $"Assetbuild {ASSETBUILD_VERSION}";
const string STRING_DESCRIPTION = $"Builds assets for Zombono";
const string STRING_USAGE = $"Assetbuild <game> [base directory] [release cfg]\n\n" +
    $"<game>: Path to directory contaning game files\n" +
    $"[directory]: Optional - base directory (default is ../../../../game/)\n" +
    $"[release cfg]: Optional - release config (case insensitive), valid: Debug, Release";

#endregion

Console.WriteLine(STRING_SIGNON);
Console.WriteLine(STRING_DESCRIPTION);

#region Command-line parsing
if (args.Length < 1)
{
    Console.WriteLine("Must provide game directory!");
    PrintHelpAndExit(1);
}

if (args.Length >= 2)
{ 
    if (!Directory.Exists(args[2]))
    {
        Console.WriteLine($"The base directory {args[2]} does not exist!");
        PrintHelpAndExit(2);
    }

    gameDir = args[2];
}

if (args.Length >= 3)
{
    if (!validConfigs.Contains(args[3], StringComparer.InvariantCultureIgnoreCase))
    {
        Console.WriteLine($"Invalid build config provided (valid options: Debug, Release)");
        PrintHelpAndExit(4);
    }
}

if (!Directory.Exists($"{gameDir}\\{args[1]}"))
{
    Console.WriteLine($"The game directory {args[2]} does not exist!");
    PrintHelpAndExit(3);
}

#endregion

#region Main code

Console.WriteLine("Building gfx.wad...");

Console.WriteLine("Building basecfg...");
Console.WriteLine("Building qc..."); // progs.dat/qwprogs.dat -> root of pak0 
Console.WriteLine("Building pak 0...");
Console.WriteLine("Building pak 1...");
#endregion

#region Utility functions

void PrintHelpAndExit(int exitCode)
{
    Console.WriteLine(STRING_USAGE);
    Environment.Exit(exitCode);
}
#endregion