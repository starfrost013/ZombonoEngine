//
// BMP2LMP Utility (version 2)
// Copyright © 2023 starfrost
// Converts bitmap files to Zombono LMP32 files
//

#region Constants
string WAL2TGA_VERSION = "2.0.0";
#endregion

#region Variables
string inputItem = string.Empty;
string outputItem = string.Empty;
// very bad, replace with something better later when we add actual arguments
bool folderMode = false;
string[] inputFiles;

#if DEBUG
Stopwatch timer = new Stopwatch();
timer.Start();
#endif
#endregion

#region Main code
try
{
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
        if (arg.Equals("-q", StringComparison.InvariantCultureIgnoreCase)) Util.QuietMode = true;
    }
    #endregion

    Print($"wal2tga {WAL2TGA_VERSION}");
    Print("Converts WAL to 32-bit TGA");

    if (folderMode)
    {
        // add all the files
        inputFiles = Directory.GetFiles(inputItem, "*.wal", SearchOption.AllDirectories);
    }
    else
    {
        // just one file so add input file to it
        inputFiles = new string[] { inputItem };
    }

    foreach (string inputFileName in inputFiles)
    {
        byte[] inputImageData = File.ReadAllBytes(inputFileName);

        int dataLocation = BitConverter.ToInt32(inputImageData.AsSpan()[10..14]);

        // all of the trillions of bitmap header versions share this information
        // ranges are not inclusive
        int widthStart = 0x12, widthEnd = widthStart + 4;
        int heightStart = 0x16, heightEnd = dataLocation + 4;

        // integer for compatibility with engine (won't be an issue unless we have a 2 gigabyte bmp file...)
        int bitmapWidth = BitConverter.ToInt32(inputImageData.AsSpan()[widthStart..widthEnd]);
        int bitmapHeight = BitConverter.ToInt32(inputImageData.AsSpan()[heightStart..heightEnd]);

        string outputFileName = outputItem;
        // bad

        BinaryWriter outputFileStream;

        if (folderMode)
        {
            outputFileName = $@"{outputItem}\{Path.GetFileName(inputFileName).Replace(".wal", ".tga", StringComparison.InvariantCultureIgnoreCase)}";
            outputFileStream = new(new FileStream(outputFileName, FileMode.OpenOrCreate));
        }
        else
        {
            outputFileStream = new(new FileStream(outputFileName, FileMode.OpenOrCreate));
        }

        Lmp32Header header = new()
        {
            Width = bitmapWidth,
            Height = bitmapHeight,
        };

        header.Write(outputFileStream);

        // no color indexes because 32-bit only!
        int imageDataStart = dataLocation;
        int imageDataEnd = inputImageData.Length - 1;

        // flip from BGRA to RGBA
        // TODO: Check format (this is NOT a resilient tool!)
        for (int imageByte = imageDataStart; imageByte < imageDataEnd; imageByte += 4)
        {
            byte current_b = inputImageData[imageByte];
            byte current_g = inputImageData[imageByte + 1];
            byte current_r = inputImageData[imageByte + 2];
            byte current_a = inputImageData[imageByte + 3];

            inputImageData[imageByte] = current_r;
            inputImageData[imageByte + 1] = current_g;
            inputImageData[imageByte + 2] = current_b;
            inputImageData[imageByte + 3] = current_a;
        }

        // write out in left-right, top-down order
        for (int y = 0; y < header.Height; y++)
        {
            for (int x = 0; x < header.Width; x++)
            {
                int pixelLocation = GetPixelLocation(imageDataStart, header, x, y);

                outputFileStream.Write(inputImageData[pixelLocation]);
                outputFileStream.Write(inputImageData[pixelLocation + 1]);
                outputFileStream.Write(inputImageData[pixelLocation + 2]);
                outputFileStream.Write(inputImageData[pixelLocation + 3]);

            }
        }

        Print($"Converted file {inputFileName} to {outputFileName}!", ConsoleColor.Green);
        Console.ResetColor();
    }

#if DEBUG
    timer.Stop();
    Util.PrintLoud($"Time taken to run: {(double)timer.ElapsedTicks / 10000d}ms");
#endif
    Util.Print($"Done!", ConsoleColor.Green);
    Environment.Exit(0);

    int GetPixelLocation(int imageDataStart, Lmp32Header header, int x, int y)
    {
        // we need to flip the Y order only
        return imageDataStart + (header.Width * ((header.Height - 1) - y) * 4) + (x * 4);
    }

}
catch (Exception ex)
{
    PrintLoud($"An exception occurred!\n\n{ex}");
    Environment.Exit(4);
}
#endregion