// See https://aka.ms/new-console-template for more information
using bmp2lmp;
using IronSoftware.Drawing;  

Console.WriteLine("bmp2lmp");
Console.WriteLine("Converts 32-bit BMP to LMP32");

#region Variables
string inputFile = string.Empty;
string outputFile = string.Empty;
#endregion

#region Command line parsing
switch (args.Length)
{
    case 0:
        PrintHelpAndExit("No file provided!", 1);
        break;
    default:
        if (!File.Exists(args[0]))
        {
            PrintHelpAndExit($"The input file {args[0]} does not exist!", 2);
        }

        inputFile = args[0];

        if (args.Length >= 2)
        {
            outputFile = args[1];
        }
        else
        {
            outputFile = inputFile.Replace(".bmp", ".lmp", StringComparison.InvariantCultureIgnoreCase);
        }

        break;
}
#endregion

byte[] inputFileBytes = File.ReadAllBytes(inputFile);

AnyBitmap bitmap = new(inputFileBytes);

BinaryWriter outputFileStream = new(new FileStream(outputFile, FileMode.OpenOrCreate));

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
Console.WriteLine("Done!");
Console.ResetColor();

void PrintHelpAndExit(string error, int exitCode)
{
    Console.WriteLine("bmp2lmp [input file] <output file>");
    Console.WriteLine("input file: input bmp file");
    Console.WriteLine("output file: output lmp file (optional). If not provided, will be saved to <input file name>.bmp\n");
    PrintErrorAndExit(error, exitCode);
}

void PrintErrorAndExit(string error, int exitCode)
{
    Console.WriteLine(error);
    Environment.Exit(exitCode);
}
