// See https://aka.ms/new-console-template for more information
using bmp2lmp;
using IronSoftware.Drawing;

#region Constants
string BMP2LMP_VERSION = "1.1.0";
#endregion

#region Variables
string inputItem = string.Empty;
string outputItem = string.Empty;
// very bad, replace with something better later when we add actual arguments
bool folderMode = false;
#endregion

Console.WriteLine($"bmp2lmp {BMP2LMP_VERSION}");
Console.WriteLine("Converts 32-bit BMP to LMP32");

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
#endregion

string[] inputFiles;

if (folderMode)
{
    // add all the files
    inputFiles = Directory.GetFiles(inputItem, "*", SearchOption.AllDirectories);    
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

    if (inputFileName.Contains(".bmp", StringComparison.InvariantCultureIgnoreCase))
    {
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
        Console.WriteLine($"Converted file {inputFileName} to {outputFileName}!");
        Console.ResetColor();
    }
}

Console.ForegroundColor = ConsoleColor.Green;
Console.WriteLine($"Done!");
Console.ResetColor();

void PrintHelpAndExit(string error, int exitCode)
{
    Console.WriteLine("bmp2lmp [input file or folder] <output file>");
    Console.WriteLine("input file: input bmp file or folder");
    Console.WriteLine("output file: output lmp file or folder; optional if folder. if folder mode, files will be renamed to .bmp\n");
    PrintErrorAndExit(error, exitCode);
}

void PrintErrorAndExit(string error, int exitCode)
{
    Console.WriteLine(error);
    Environment.Exit(exitCode);
}
