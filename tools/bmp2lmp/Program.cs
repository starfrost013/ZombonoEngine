// See https://aka.ms/new-console-template for more information
using bmp2lmp;
using IronSoftware.Drawing;
using System.Security.Cryptography.X509Certificates;

#region Constants
string BMP2LMP_VERSION = "1.2.0";
#endregion

#region Variables
string inputItem = string.Empty;
string outputItem = string.Empty;
// very bad, replace with something better later when we add actual arguments
bool folderMode = false;
bool quietMode = false;
string[] inputFiles;
#endregion

#region Command line parsing
switch (args.Length)
{
    case 0:
        PrintHelpAndExit("No file provided!", 1);
        break;
    default:

        inputItem = args[0];

        // set folder mode
        folderMode = !(inputItem.Contains('.', StringComparison.InvariantCultureIgnoreCase));

        if (folderMode)
        {
            if (!Directory.Exists(inputItem))
            {
                PrintHelpAndExit($"The input folder {inputItem} does not exist!", 3);
            }

            if (args.Length >= 2)
            {
                outputItem = args[1];

                if (!Directory.Exists(outputItem))
                {
                    PrintHelpAndExit($"The output folder {outputItem} does not exist!", 3);
                }
            }
            else
            {
                // default to current directory
                outputItem = inputItem;
            }
        }
        else
        {
            if (!File.Exists(inputItem))
            {
                PrintHelpAndExit($"The input file {inputItem} does not exist!", 2);
            }

            if (args.Length >= 2)
            {
                outputItem = args[1];
            }
            else
            {
                PrintHelpAndExit($"The output file {outputItem} does not exist!", 2);
            }
        }

        break;
}

// stuff you can put anywhere
foreach (string arg in args)
{
    if (arg.Equals("-q", StringComparison.InvariantCultureIgnoreCase)) quietMode = true;
}
#endregion


Print($"bmp2lmp {BMP2LMP_VERSION}");
Print("Converts 32-bit BMP to LMP32");

if (folderMode)
{
    // add all the files
    inputFiles = Directory.GetFiles(inputItem, "*.bmp", SearchOption.AllDirectories);    
}
else
{
    // just one file so add input file to it
    inputFiles = new string[] { inputItem }; 
}

foreach (string inputFileName in inputFiles)
{
    byte[] inputFileBytes = File.ReadAllBytes(inputFileName);

    string outputFileName = outputItem;

    AnyBitmap bitmap = new(inputFileBytes);

    // bad

    BinaryWriter outputFileStream;

    if (folderMode)
    {
        outputFileName = $@"{outputItem}\{Path.GetFileName(inputFileName).Replace(".bmp", ".lmp", StringComparison.InvariantCultureIgnoreCase)}";
        outputFileStream = new(new FileStream(outputFileName, FileMode.OpenOrCreate));
    }
    else
    {
        outputFileStream = new(new FileStream(outputFileName, FileMode.OpenOrCreate));
    }

    Lmp32Header header = new()
    {
        Height = bitmap.Height,
        Width = bitmap.Width,
    };

    header.Write(outputFileStream);

    for (int y = 0; y < bitmap.Height; y++)
    {
        for (int x = 0; x < bitmap.Width; x++)
        {
            IronSoftware.Drawing.Color color = bitmap.GetPixel(x, y);

            // RGBA format for quake
            outputFileStream.Write(color.R);
            outputFileStream.Write(color.G);
            outputFileStream.Write(color.B);
            outputFileStream.Write(color.A);
        }
    }

    Console.ForegroundColor = ConsoleColor.Green;
    Print($"Converted file {inputFileName} to {outputFileName}!");
    Console.ResetColor();
}

Console.ForegroundColor = ConsoleColor.Green;
Print($"Done!");
Console.ResetColor();

void PrintLoud(string text, ConsoleColor foreground = ConsoleColor.Gray)
{
    Console.ForegroundColor = foreground;
    Console.WriteLine(text);
    Console.ResetColor();
}

void Print(string text, ConsoleColor foreground = ConsoleColor.Gray)
{
    if (!quietMode) Print(text, foreground);
}

void PrintHelpAndExit(string error, int exitCode)
{
    PrintLoud("bmp2lmp [input file or folder] <output file>");
    PrintLoud("input file: input bmp file or folder");
    PrintLoud("output file: output lmp file or folder; optional if folder. if folder mode, files will be renamed to .bmp\n");
    PrintErrorAndExit(error, exitCode);
}

void PrintErrorAndExit(string error, int exitCode)
{
    PrintLoud(error);
    Environment.Exit(exitCode);
}
