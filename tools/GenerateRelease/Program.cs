// Zombono Release Generation Tool
// Sunday, June 30, 2024

// Todo:
//  - Deploy update
//  - Obtain version number automatically

using System.Diagnostics;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace GenerateRelease
{
    internal class Program
    {
        private const string VERSION = "1.0.0";
        private const string UPDATEINFO_PATH = @"..\..\..\..\..\webservices\updater\updateinfo.json"; // todo: override!

        // classes for the serialiser
        // THESE ARE ALL NAMED AFTER THE C NAMES FOR COMPATIBILITY!
        public class UpdateChannel
        {
            /// <summary>
            /// The name of the update channel.
            /// </summary>
            public string? Name { get; set; }

            /// <summary>
            /// The description of the update channel.
            /// </summary>
            public string? Description { get; set; }

            /// <summary>
            /// The release date of the update. Converted from DateTime
            /// UNDERSCORE IS FOR COMPATIBILITY WITH C CODE WHILE BEING AS CLOSE TO C# LANGUAGE CONVENTIONS AS POSSIBLE
            /// </summary>
            public string? Release_Date { get; set; }

            /// <summary>
            /// The version of the update. Extracted from Zombono.exe version information
            /// </summary>
            public string? Version { get; set; }

            internal UpdateChannel(string name, string description, string releaseDate, string version)
            {
                Name = name;
                Description = description;
                Release_Date = releaseDate;  
                Version = version;
            }

            public UpdateChannel() { }
        }

        /// <summary>
        /// The list of channels
        /// </summary>
        static internal UpdateChannel[] channels = 
        {
            new("Debug", string.Empty, string.Empty, string.Empty),
            new("Playtest", string.Empty, string.Empty, string.Empty),
            new("Release", string.Empty, string.Empty, string.Empty)
        };

        /// <summary>
        /// Stores command-line information.
        /// </summary>
        internal static class CommandLine
        {
            internal static string Game = string.Empty;

            internal static string Channel = string.Empty;
        }

        // i hate c# for doing everything by value
        private static int currentChannelIndex = 0;

        internal static UpdateChannel? CurrentChannel
        {
            get
            {
                return channels[currentChannelIndex];
            }
        }

        internal static FileStream jsonStream;

        /// <summary>
        /// The options for the JSON serialiser to use.
        /// </summary>
        internal static JsonSerializerOptions options = new ()
        {
            NumberHandling = JsonNumberHandling.AllowReadingFromString | JsonNumberHandling.AllowNamedFloatingPointLiterals | JsonNumberHandling.WriteAsString,
            PropertyNameCaseInsensitive = true,
            PropertyNamingPolicy = JsonNamingPolicy.SnakeCaseLower,
        };

        private static void SetUpdateChannel()
        {
            for (int channel_index = 0; channel_index < channels.Length; channel_index++)
            {
                UpdateChannel channel = channels[channel_index];

                if (channel.Name.ToLowerInvariant() == CommandLine.Channel.ToLowerInvariant())
                    currentChannelIndex = channel_index;
            }
        }

        static void Main(string[] args)
        {
            try
            {
                Console.WriteLine($"GenerateRelease for Euphoria {VERSION} (June 30, 2024)");
                Console.WriteLine($"© 2024 starfrost");

                if (args.Length != 2)
                    PrintHelpAndExit(1);

                CommandLine.Game = args[0];
                CommandLine.Channel = args[1];

                // load existing updateinfo
                Console.WriteLine($"Loading existing UpdateInfo.json from {UPDATEINFO_PATH}...");

                if (!File.Exists(UPDATEINFO_PATH))
                {
                    PrintErrorAndExit("Updateinfo.json not found!", 7, false);
                }

                jsonStream = new(UPDATEINFO_PATH, FileMode.OpenOrCreate);

                channels = JsonSerializer.Deserialize<UpdateChannel[]>(jsonStream, options);        

                if (string.IsNullOrWhiteSpace(CommandLine.Game)
                    || string.IsNullOrWhiteSpace(CommandLine.Channel))
                {
                    PrintHelpAndExit(2);
                }

                // check the user picked a real channel
                SetUpdateChannel();

                if (CurrentChannel == null) // why not c syntax :(
                    PrintErrorAndExit("Invalid channel specified - valid options are debug, release, and playtest (case-insensitive)", 3);

                Run();
            }
            catch (Exception ex)
            {
                PrintErrorAndExit($"An exception occurred:\n\n{ex.Message}", 5);
            }
        }

        private static void Run()
        {
            Debug.Assert(CurrentChannel != null);

            Console.Write("Enter description for update: ");
            string? description = Console.ReadLine();

            if (string.IsNullOrWhiteSpace(description))
                PrintErrorAndExit("No description provided!", 4, false);

            CurrentChannel.Description = description;
            // ISO 8601
            CurrentChannel.Release_Date = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss");

            Console.Write("Enter version number: ");
            string? versionNumber = Console.ReadLine();

            if (string.IsNullOrWhiteSpace(versionNumber)) 
                PrintErrorAndExit("Version number not provided!", 6, false);

            CurrentChannel.Version = versionNumber;

            jsonStream.Seek(0, SeekOrigin.Begin);
            
            JsonSerializer.Serialize(jsonStream, channels, options);

            jsonStream.Close();

        }

        private static void PrintHelpAndExit(int exitCode)
        {
            Console.WriteLine("GenerateRelease <game> <config>\n\n" +
                "" + // newline padding
                "Generates a release for Euphoria-based games e.g. Zombono.\n");

            Environment.Exit(exitCode);
        }

        private static void PrintErrorAndExit(string errorMessage, int exitCode, bool printHelp = true)
        {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine(errorMessage);
            Console.ResetColor();

            if (printHelp)
            {
                PrintHelpAndExit(exitCode);
            }
            else
            {
                Environment.Exit(exitCode);
            }
        }
    }
}
