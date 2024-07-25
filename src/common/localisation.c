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

#include <common/common.h>

// cvars
cvar_t* language;

// globals
localisation_entry_t localisation_entries[LOCALISATION_ENTRIES_MAX];
localised_string_t localised_strings[LOCALISATION_ENTRIES_MAX];

uint32_t localisation_entries_count;
uint32_t localised_strings_count;

// functions only used in this translation unit
void Localisation_LoadCurrentLanguage();

// ran when localisation system initialised
// AND when changing language
void Localisation_Init()
{
	language = Cvar_Get("language", "english", CVAR_ARCHIVE);
	Localisation_LoadCurrentLanguage();
}

#define LINE_BUFFER_LENGTH	LOCALISATION_MAX_LENGTH_KEY + LOCALISATION_MAX_LENGTH_VALUE + 1

// Loads the current localisation language.
void Localisation_LoadCurrentLanguage()
{
	FILE* localisation_lang_ptr;
	char loc_filename[MAX_QPATH] = { 0 };
	char line_buf[LINE_BUFFER_LENGTH] = { 0 }; // +1 for "=" sign
	char* token_ptr;

	localisation_entries_count = 0;

	snprintf(loc_filename, MAX_QPATH, "text/%s/%s", language->string, LOCALISATION_DICTIONARY_FILENAME);

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

			// don't search forever if there is only one line
			if (token_ptr == ((uint8_t*)file_ptr + file_size))
				break;
		}

		// don't clobber random memory if there is no newline or anything
		if (string_length >= file_size)
			return; 

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

		if (equals_ptr - line_buf > LOCALISATION_MAX_LENGTH_KEY)
		{
			Com_DPrintf("Localisation key too long, skipping...\n");
			continue;
		}

		// figure out the length after the key
		int32_t value_length = strlen((const char*)equals_ptr + 1);

		if (value_length >= LOCALISATION_MAX_LENGTH_VALUE)
		{
			Com_DPrintf("Localisation value too long, skipping...\n");
			continue;
		}

		// copy it
		strncpy(localisation_entries[localisation_entries_count].key, line_buf, (equals_ptr - line_buf));
		strncpy(localisation_entries[localisation_entries_count].value, equals_ptr + 1, value_length); // +1 to skip equals sign

		localisation_entries_count++;
	}

	Com_Printf("Loaded %d localisation strings\n", localisation_entries_count);
}

// Returns the value for the key key.
localisation_entry_t* Localisation_GetString(char* key)
{
	// if language has changed, reload localisation files
	if (language->modified)
	{
		Localisation_LoadCurrentLanguage();
		language->modified = false;
	}

	for (int32_t text_string_id = 0; text_string_id < localisation_entries_count; text_string_id++)
	{
		localisation_entry_t* value = &localisation_entries[text_string_id];

		if (!strcmp(value->key, key))
			return value;
	}

	return NULL;
}

// Shamelessly stolen from Lightning
#define LOCALISATION_KEY_START	"["
#define LOCALISATION_KEY_START_CHAR '['
#define LOCALISATION_KEY_END	"]"
#define LOCALISATION_KEY_END_CHAR	']' // needed for some comparisons

// really big buffer for strtok to use
#define STRING_TEMP_BUF_SIZE	0x10000
char string_temp_buf[STRING_TEMP_BUF_SIZE] = { 0 };

// This code sucks
char* Localisation_ProcessString(char* value)
{
	// if language has changed, reload localisation files
	if (language->modified)
	{
		Localisation_LoadCurrentLanguage();
		language->modified = false;
	}

	// don't continuously clear the big buffer we use for the localisation string, just clear whatever is left
	uint32_t clear_amount = 0;

	while (clear_amount < STRING_TEMP_BUF_SIZE
		&& string_temp_buf[clear_amount] != 0x00)
	{
		clear_amount++;
	}

	memset(string_temp_buf, 0x00, clear_amount);

	strncpy(string_temp_buf, value, strlen(value));

	size_t localisation_string_length = 0;
	char loc_string_key_buf[LOCALISATION_MAX_LENGTH_KEY] = { 0 };
	size_t string_length = strlen(string_temp_buf);
	size_t string_length_original = string_length;

	// strstr can only find the first use of the character so we can't use that
	// however we still need to start the string temp without breaking further [ charcters,
	// so just check if thw first character is a localisation start character and set the buffer to the start of the string
	// and use first_token below to determine if we need to call strtok or not

	bool first_char_is_key = (string_temp_buf[0] == LOCALISATION_KEY_START_CHAR);

	char* token_ptr = string_temp_buf;

	if (!first_char_is_key)
		token_ptr = strtok(string_temp_buf, LOCALISATION_KEY_START);

	if (!token_ptr)
		return value; // no string here

	if (string_length > STRING_TEMP_BUF_SIZE)
		Sys_Error("Passed string more than 0x10000 bytes in length to Localisation_ProcessString?!");

	// determine the length of the string
	while (token_ptr < (string_temp_buf + string_length))
	{
		char* key_start_ptr = NULL;

		if (first_char_is_key)
		{
			key_start_ptr = strtok(string_temp_buf, LOCALISATION_KEY_START);
		}
		else
		{
			key_start_ptr = strtok(NULL, LOCALISATION_KEY_START);
		}

		// no localisation to do
		if (!key_start_ptr)
			return value;

		// iterate through each part of the string

		int32_t localisation_key_length = 0;

		char* key_end_ptr = strchr(key_start_ptr, LOCALISATION_KEY_END_CHAR);

		localisation_key_length = key_end_ptr - key_start_ptr;

		if (localisation_key_length > LOCALISATION_MAX_LENGTH_KEY)
		{
			Com_Printf("Warning: Localisation string key length of %d, more than %d\n", localisation_key_length, LOCALISATION_MAX_LENGTH_KEY);
			continue;
		}
		else if (localisation_key_length <= 0)
		{
			Com_Printf("Warning: Empty localisation string key, skipping\n");
			continue;
		}

		// get the localisation string
		strncpy(loc_string_key_buf, key_start_ptr, localisation_key_length);

		// was this string already localised?
			// if so, return
		for (uint32_t localisation_string_id = 0; localisation_string_id < localisation_entries_count; localisation_string_id++)
		{
			if (!strcmp(localisation_entries[localisation_string_id].key, loc_string_key_buf))
				return localisation_entries[localisation_string_id].value;
		}

		char* loc_string = Localisation_GetString(loc_string_key_buf);

		if (!loc_string)
			return value;
		
		localisation_string_length = strlen(loc_string);

		// figure out the new length of the string
		string_length = string_length - localisation_key_length + localisation_string_length;

		token_ptr = strtok(NULL, LOCALISATION_KEY_START);

		// done so break
		if (!token_ptr)
			break;
	}

	if (localised_strings_count >= LOCALISATION_ENTRIES_MAX)
	{
		Com_Printf("Too many localisation strings! (%d > %d)", localised_strings_count, LOCALISATION_ENTRIES_MAX);
		return NULL;
	}

	// allocate memory for the string
	// zero it since loading this happens rarely
	
	localised_strings[localised_strings_count].value = (char*)calloc(1, string_length);

	if (!localised_strings[localised_strings_count].value)
	{
		Sys_Error("Failed to allocate memory for localisation string information");
		return NULL;
	}

	// reset pointer
	// this code might suck
	token_ptr = strtok(string_temp_buf, LOCALISATION_KEY_START);

	// if we got to this point we need to un-terminate the string as modified by strtok
	// so do that
	while (token_ptr < (string_temp_buf + string_length_original))
	{
		if (*token_ptr == '\0')
			*token_ptr = LOCALISATION_KEY_START_CHAR;

		token_ptr++;
	}

	token_ptr = string_temp_buf;

	// actually copy it
	while (token_ptr < (string_temp_buf + string_length_original))
	{
		// get the part actually after the LOCALISATION_KEY_START string
		char* key_start_ptr = strtok(NULL, LOCALISATION_KEY_START);
		char* key_end_ptr = key_start_ptr;

		// copy part before the localisation string
		strncpy(localised_strings[localised_strings_count].value, token_ptr, (key_start_ptr - token_ptr) - 1); // -1 to cut off the [

		// iterate through each part of the string

		int32_t localisation_key_length = 0;

		key_end_ptr = strchr(key_start_ptr, LOCALISATION_KEY_END_CHAR);

		localisation_key_length = key_end_ptr - key_start_ptr;

		// get the localisation string
		strncpy(loc_string_key_buf, key_start_ptr, localisation_key_length);

		localisation_entry_t* loc_string = Localisation_GetString(loc_string_key_buf);

		if (!loc_string)
			return value;

		localised_strings[localised_strings_count].key = loc_string->key;

		uint32_t loc_string_current_length = strlen(localised_strings[localised_strings_count].value);

		// copy the localisation string
		strncpy(localised_strings[localised_strings_count].value + loc_string_current_length, loc_string->value, strlen(loc_string->value));

		token_ptr = strtok(NULL, LOCALISATION_KEY_START);

		// we are done
		if (!token_ptr)
			break;
	}

	// we are done
	localised_strings_count++;
	return localised_strings[localised_strings_count - 1].value;
}

// Frees all localisation strins
void Localisation_Shutdown()
{
	for (int32_t localisation_string_id = 0; localisation_string_id < localisation_entries_count; localisation_string_id++)
	{
		if (localised_strings[localisation_string_id].value != NULL)
			free((void*)localised_strings[localisation_string_id].value);
	}
}