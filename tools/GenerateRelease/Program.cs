// Copyright © 2024 starfrost. 
//
// GenerateRelease
// 
// Generates an UpdateInfo.json file for Zombono.
//
// Created: April 20, 2024

namespace GenerateRelease
{
    internal static class Program
    {
        /// <summary>
        ///  The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            // To customize application configuration such as set high DPI settings or default font,
            // see https://aka.ms/applicationconfiguration.
            ApplicationConfiguration.Initialize();
            Application.Run(new Form1());
        }
    }
}