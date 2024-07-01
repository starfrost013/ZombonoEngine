/*
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

// localisation.c
// Text localisation and dictionary system (July 1, 2024)

#include <qcommon/qcommon.h>

text_string_t text_strings[TEXT_STRINGS_MAX] = { 0 };
int32_t text_strings_num;								// number loaded for current language

cvar_t* language;

void Localisation_LoadCurrentLanguage();

// ran when localisation system initialised
// AND when changing language
void Localisation_Init()
{
	language = Cvar_Get("language", "english", CVAR_ARCHIVE);
	Localisation_LoadCurrentLanguage();
}

#define LINE_BUFFER_LENGTH	TEXT_STRING_MAX_LENGTH_KEY + TEXT_STRING_MAX_LENGTH_VALUE + 1

// Loads the current localisation language.
void Localisation_LoadCurrentLanguage()
{
	FILE* localisation_lang_ptr;
	char loc_filename[MAX_QPATH] = { 0 };
	char line_buf[LINE_BUFFER_LENGTH] = { 0 }; // +1 for "=" sign
	char* token_ptr;

	text_strings_num = 0;

	snprintf(&loc_filename, MAX_QPATH, "text/%s/%s", language->string, TEXT_DICTIONARY_FILENAME);

	Com_Printf("Loading localisation information for language %s from file %s...\n", language->string, loc_filename);

	int32_t file_size = FS_FOpenFile(loc_filename, &localisation_lang_ptr);

	if (file_size < 0)
		Sys_Error("Failed to load language strings file %s!", loc_filename);

	// temporarily allocate some storage
	// this function is rarely called, so we can CALLOC it

	void* file_ptr = calloc(1, file_size);

	token_ptr = file_ptr;

	if (!file_ptr)
	{
		Sys_Error("Failed to allocate memory to read language file ***BUG - OUT OF MEMORY???***");
		return; // shut up compiler
	}

	fread(file_ptr, 1, file_size, localisation_lang_ptr);
	fseek(localisation_lang_ptr, 0, SEEK_SET);

	while (token_ptr < ((uint8_t*)file_ptr + file_size))
	{
		int32_t string_length = 0;

		// find the newline
		while (*token_ptr != '\r'
			&& *token_ptr != '\n')
		{
			token_ptr++;
			string_length++;
		}

		// cut off the line
		*token_ptr = '\0';
		token_ptr++;

		// skip empty lines
		if (string_length <= 1)
			continue; 

		strncpy(line_buf, token_ptr - string_length - 1, LINE_BUFFER_LENGTH);

		// handle comment lines
		char* comment_ptr = strstr(line_buf, "//");
		char* line_no_comment_ptr = line_buf;

		// obliterate the comment
		if (comment_ptr)
		{
			*comment_ptr = '\0';

			// only commented
			if (strlen(comment_ptr) == 0)
				continue;
		}

		char* equals_ptr = strstr(line_buf, "=");

		// malformed
		if (!equals_ptr)
		{
			Com_DPrintf("Malformed localisation string, skipping...\n");
			continue;
		}

		if (equals_ptr - line_buf > TEXT_STRING_MAX_LENGTH_KEY)
		{
			Com_DPrintf("Localisation key too long, skipping...\n");
			continue;
		}

		// figure out the length after the key
		int32_t value_length = strlen((const char*)equals_ptr + 1);

		if (value_length >= TEXT_STRING_MAX_LENGTH_VALUE)
		{
			Com_DPrintf("Localisation value too long, skipping...\n");
			continue;
		}

		// copy it
		strncpy(text_strings[text_strings_num].key, line_buf, (equals_ptr - line_buf));
		strncpy(text_strings[text_strings_num].value, equals_ptr + 1, value_length); // +1 to skip equals sign

		text_strings_num++;
	}

	Com_Printf("Loaded %d localisation strings\n", text_strings_num);
}

char* Localisation_GetString()
{
	if (language->modified)
		Localisation_LoadCurrentLanguage(); // reload localisation files
}

char* Localisation_ProcessString()
{

}