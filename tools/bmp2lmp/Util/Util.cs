
namespace wal2tga
{
    /// <summary>
    /// Core WAL2TGA utilities
    /// </summary>
    internal static class Util
    {
        internal static bool QuietMode { get; set; }    

        public static void PrintLoud(string text, ConsoleColor foreground = ConsoleColor.Gray)
        {
            Console.ForegroundColor = foreground;
            Console.WriteLine(text);
            Console.ResetColor();
        }

        public static void Print(string text, ConsoleColor foreground = ConsoleColor.Gray)
        {
            if (QuietMode) PrintLoud(text, foreground);
        }

        public static void PrintHelpAndExit(string error, int exitCode)
        {
            PrintLoud("wal2tga [input file or folder] <output file>");
            PrintLoud("input file: input bmp file or folder");
            PrintLoud("output file: output lmp file or folder; optional if folder. if folder mode, files will be renamed to .bmp\n");
            PrintErrorAndExit(error, exitCode);
        }

        public static void PrintErrorAndExit(string error, int exitCode)
        {
            PrintLoud(error);
            Environment.Exit(exitCode);
        }
    }
}
