// See https://aka.ms/new-console-template for more information

#region Constants
const string ASSETBUILD_VERSION = "1.0.0";
#endregion

#region Strings
const string STRING_SIGNON = $"Assetbuild {ASSETBUILD_VERSION}";
const string STRING_DESCRIPTION = $"Builds assets for Zombono";
const string STRING_USAGE = $"Assetbuild <game>\n\n" +
    $"<game>: Path to directory contaning game files";
#endregion

Console.WriteLine(STRING_SIGNON);
Console.WriteLine(STRING_DESCRIPTION);