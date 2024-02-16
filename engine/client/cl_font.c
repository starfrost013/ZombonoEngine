/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2023-2024 starfrost

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include "client.h"

// cl_font.c: 
// Font Engine (February 8, 2024)

// Functions not exposed in headers
qboolean Font_LoadFont(char file_name[FONT_MAX_FILENAME_LEN]);

qboolean Font_Init()
{
	FILE*	font_list_stream;
	char	file_name_list[FONT_MAX_FILENAME_LEN] = { 0 };
	char	file_name_font[FONT_MAX_FILENAME_LEN] = { 0 };
	long	file_length = 0;
	long	file_location = 0;
	char*	token;
	char*	file;

	Com_Printf("Loading fonts...\n");

	// open up fonts.txt
	snprintf(file_name_list, FONT_MAX_FILENAME_LEN, "%s\\%s", FS_Gamedir(), "fonts\\fonts.txt");

	font_list_stream = fopen(file_name_list, "rb");

	if (font_list_stream == NULL)
	{
		Sys_Error("Failed to initialise font engine: Couldn't open %s", FONT_LIST_FILENAME);
		return false; // doesn't return but its for communication
	}

	// get the file length

	fseek(font_list_stream, 0, SEEK_END);

	file_length = ftell(font_list_stream);

	// msvc does not support VLAs
	file = calloc(1, file_length);

	assert(file != NULL);

	// return to start and read the file
	fseek(font_list_stream, 0, SEEK_SET);
	
	fread(file, 1, file_length, font_list_stream);
	
	// start at the first byte
	token = file;

	// parse fonts.txt
	while (file_location < file_length)
	{
		file_location = token - file;

		int n = 0; // initialise length variable

		// if its not a comment por newline, parse the filename
		// otherwise go to next line
		if (token[0] != '/'
			&& token[0] != '\n'
			&& token[0] != '\r'
			&& ((file_length - file_location > 1) && token[1] != '/')) // don't overflow if we're on he last byte
		{
			while (token[0] != '\n'
				&& token[0] != '\r') // prevent /r from being added to filenames
			{
				// copy 1 byte at a time (memory alignment to word size?), ignore windows newlines
				memcpy(file_name_font + n, token, 1);
				n++;
				token++;

				if (n > FONT_MAX_FILENAME_LEN - 2) // -2 because of expression below
				{
					file_name_font[FONT_MAX_FILENAME_LEN - 1] = '\0'; // terminate string

					Com_Printf("Tried to load font with path name > 256 chars (not allowed): truncated name %s", file_name_font);
				}
			}

			file_name_font[n] = '\0'; // terminate string
			Font_LoadFont(file_name_font);

			// n re-initialised above
		}
		else // go to next line
		{
			while (token[0] != '\n') token++;
			token++; // go past the newline
		}
		// token at this point is the character after the newline

	}

	// close it
	fclose(font_list_stream); 
	free(file);
	return true; 
}

qboolean Font_LoadFont(char file_name[FONT_MAX_FILENAME_LEN])
{
	Com_DPrintf("Loading font %s\n", file_name);

	// create the font object.

	FILE* tga_stream;
	FILE* json_stream;
	//+4 for extension
	char tga_filename[FONT_MAX_FILENAME_LEN+4] = {0};
	char json_filename[FONT_MAX_FILENAME_LEN+4] = {0};

	// open up json, load targa as a texture.
	// .tga is *assumed* by LoadPic!!
	snprintf(&tga_filename, FONT_MAX_FILENAME_LEN + 4, "fonts/%s", file_name);
	snprintf(&json_filename, FONT_MAX_FILENAME_LEN + 4, "fonts/%s.json", file_name);

	Com_DPrintf("Font_LoadFont: Loading Font TGA %s.tga\n", tga_filename);
	// TODO: MERGE PICS AND IMAGES!!!
	re.LoadPic(tga_filename);

	Com_DPrintf("Font_LoadFont: Loading Font JSON %s\n", json_filename);

	if (FS_FOpenFile(json_filename, &json_stream) == -1)
	{
		Sys_Error("Failed to load Font JSON %s!", json_filename);
		return false;
	}

	fclose(json_stream);
	return true; 
}

void Font_Shutdown()
{

}