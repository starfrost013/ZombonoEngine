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
// netservices_update.c: Updater

#include "netservices.h"

// Defines
#define UPDATE_JSON_URL UPDATER_BASE_URL		"/updateinfo.json"	// URL for update info json file
#define UPDATE_BINARY_BASE_URL UPDATER_BASE_URL "/updates"			// base URL for update binary
#define UPDATE_PROMPT_STR_LENGTH				2048				// Length of the update prompt string
#define DOWNLOAD_URL_STR_LENGTH					256					// Length of the download URL string
#define DOWNLOAD_URL_FORMAT						"Zombono-v%d.%d.%d.%d-%s-%s.exe"	// Format for the update package binary.

// Functions only used in this file
size_t	Netservices_UpdateInfoJsonReceive(char* ptr, size_t size, size_t nmemb, char* userdata);		// Callback function for when updateinfo.json stuff is received.
void	Netservices_UpdateInfoJsonComplete();															// Callback function for when updateinfo.json stuff is completed.
void	Netservices_UpdaterUpdateGame();																// Actually perform the game update

void	Netservices_UpdateInfoBinaryReceive(char* ptr, size_t size, size_t nmemb, char* userdata);		// Callback function for when binary stuff is received.
void	Netservices_UpdateInfoBinaryComplete();															// Callback function for when binary stuff is received.

// Globals
// 
// Set the update channel and a string for it based on the build configuration
#if !NDEBUG // gcc+msvc portable
game_update_channel current_update_channel = update_channel_debug;
const char* selected_update_channel_str = "debug";
#elif PLAYTEST
game_update_channel current_channel = update_channel_playtest;
const char* selected_update_channel_str = "playtest";
#else
game_update_channel current_channel = update_channel_release;
const char* selected_update_channel_str = "release";
#endif

CURL*			update_json_curl_obj = { 0 };
CURL*			update_binary_curl_obj = { 0 };			// see how fast this is and if we need multiple simultaneous connections (curl_multitransfer_t)

int				netservices_running_transfers;

// tmpfile() handle
FILE*			update_json_handle;						// updateinfo.json file handle
char			update_json_file_name[L_tmpnam];		// updateinfo.json temp file name

FILE*			update_binary_handle;					// Update binary file handle
char			update_binary_file_name[DOWNLOAD_URL_STR_LENGTH] = { 0 };	// Update binary file name after downloaded

// stores available update information
// if update_available is true, it will ask the user to update
game_update_t	update_info = { 0 };

// Does not block
void Netservices_UpdaterGetUpdate()
{
	// don't try and update if updating or netservices is disabled
	if (ns_noupdatecheck->value
		|| ns_disabled->value)
			return;

	// initialise the updateinfo struct
	memset(&update_info, 0x00, sizeof(game_update_t));

	Com_Printf("Checking for updates...\n");

	update_json_curl_obj = Netservices_AddCurlObject(UPDATE_JSON_URL, true, Netservices_UpdateInfoJsonReceive);

	// create a temporary file
	tmpnam(&update_json_file_name);
	
	// tell the user about this but not stop them from playing the game
	if (update_json_file_name[0] == '\0')
	{
		Sys_Msgbox("Non-Fatal Error", 0, "Failed to create Netservices UpdateInfo.json - couldn't get a temp file name!");
		return;
	}

	// open it
	update_json_handle = fopen(&update_json_file_name, "w");

	if (update_json_handle == NULL)
	{
		Sys_Msgbox("Non-Fatal Error", 0, "Failed to create Netservices UpdateInfo.json - couldn't create a temp file!");
		return;
	}

	// tell netservices to call the "is an update available?" function
	Netservices_SetOnCompleteCallback(Netservices_UpdateInfoJsonComplete);
	
	// and start the transfer...
	Netservices_StartPendingTransfers();
}

size_t Netservices_UpdateInfoJsonReceive(char* ptr, size_t size, size_t nmemb, char* userdata)
{
	// write it
	// we close it in the complete callback because this can be called many times (only 16kb is transferred at a time in CURL)
	fwrite(ptr, nmemb, size, update_json_handle);
	return nmemb;
}

// sets update_json.update_available to true
void Netservices_UpdateInfoJsonComplete()
{
	JSON_stream update_json_stream;

	Com_DPrintf("Downloaded update information to tmpfile %s\n", &update_json_file_name);

	// close and reopen the file as r+ for stupid msvc tmpfile() related reasons
	fclose(update_json_handle);
	update_json_handle = fopen(&update_json_file_name, "r+");

	// open the file as a json stream
	JSON_open_stream(&update_json_stream, update_json_handle);

	// set channel to current
	update_info.channel = current_update_channel;

	// empty string - invalid channel (failsafe)
	if (strlen(selected_update_channel_str) == 0)
	{
		Sys_Msgbox("Update Error", 0, "Invalid update channel selected. Can't update...");
		fclose(update_json_handle);
		remove(&update_json_file_name);
		return;
	}

	// are we parsing the selected update channel?
	bool current_channel_selected = false;

	// parse the json stream
	// A PARSE FAILURE IS A SYS_ERROR CONDITION

	enum JSON_type next_object = JSON_next(&update_json_stream);

	char* json_string;

	while (next_object != JSON_DONE)
	{
		switch (next_object)
		{
			// there is only one array in this json file (it's an array of update channels),
			// so we can use object objects and ignore the array objects to reduce code complexity
			case JSON_OBJECT:
				
				// iterate through array
				while (next_object != JSON_OBJECT_END)
				{	
					switch (next_object)
					{
						case JSON_STRING:
							json_string = JSON_get_string(&update_json_stream, NULL);

							// update channel name - check that the current channel is selected
							if (!strcmp(json_string, "name"))
							{
								next_object = JSON_next(&update_json_stream);
								char* current_update_channel_string = JSON_get_string(&update_json_stream, NULL);

								if (!strcmp(current_update_channel_string, selected_update_channel_str))
									current_channel_selected = true;
							}
							// Version information - only parsed if current channel is selected
							else if (!strcmp(json_string, "version")
								&& current_channel_selected)
							{
								next_object = JSON_next(&update_json_stream);
								char* version_string = JSON_get_string(&update_json_stream, NULL);

								// parse the version information
								char* token = strtok(version_string, ".");
								if (token == NULL)
									Sys_Error("Malformed version information in obtained UpdateInfo.json (1) (THIS IS A BUG)"); 
								update_info.version.major = atoi(token);

								token = strtok(NULL, ".");
								if (token == NULL)
									Sys_Error("Malformed version information in obtained UpdateInfo.json (2) (THIS IS A BUG)");
								update_info.version.minor = atoi(token);

								token = strtok(NULL, ".");
								if (token == NULL)
									Sys_Error("Malformed version information in obtained UpdateInfo.json (3) (THIS IS A BUG)");
								update_info.version.revision = atoi(token);

								token = strtok(NULL, ".");
								if (token == NULL)
									Sys_Error("Malformed version information in obtained UpdateInfo.json (4) (THIS IS A BUG)");
								update_info.version.build = atoi(token);


							}
							// Release date: ISO 8601 format date string for release of version
							else if (!strcmp(json_string, "release_date"))
							{
								next_object = JSON_next(&update_json_stream);
								char* release_date_string = JSON_get_string(&update_json_stream, NULL);

								// parse date format
								int32_t year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;

								if (sscanf(release_date_string, "%04d-%04d-%04d %02d:%02d:%02d", &year, &month, &day, &hour, &minute, &second))
								{
									// parse it
									update_info.release_date.tm_year = year;
									update_info.release_date.tm_mon = month;
									update_info.release_date.tm_mday = day;
									update_info.release_date.tm_hour = hour;
									update_info.release_date.tm_min = minute;
									update_info.release_date.tm_sec = second;
								}
								else // kill it
									Sys_Error("Malformed release date information in obtained UpdateInfo.json (THIS IS A BUG)");
							}
							// A description of the update
							else if (!strcmp(json_string, "description"))
							{
								next_object = JSON_next(&update_json_stream);
								char* description_string = JSON_get_string(&update_json_stream, NULL);

								strncpy(update_info.description, description_string, MAX_UPDATE_STR_LENGTH);
							}
							break;
					}

					next_object = JSON_next(&update_json_stream);
				}

				break;
		}

		next_object = JSON_next(&update_json_stream);
	}

	// If we got here, we assume the update was correctly parsed.
	// Run a check on the version number.

	// Only build has to be explicitly higher than the current version.
	if (update_info.version.major >= ZOMBONO_VERSION_MAJOR
		&& (update_info.version.minor >= ZOMBONO_VERSION_MINOR)
		&& (update_info.version.revision >= ZOMBONO_VERSION_REVISION)
		&& (update_info.version.build > ZOMBONO_VERSION_BUILD))
	{
		// make sure the update has been released yet,
		// this gives us time for testing jic we fuck it up and allows us to rollout at specific times
		// just normalise to time_t

		time_t new_version_time = mktime(&update_info.release_date);

		time_t current_time = time(NULL);

		// if the update is actually released a new version is available
		if (current_time > new_version_time)
		{
			Com_DPrintf("Update available for current build configuration: new version %d.%d.%d.%d, description: %s!\n", 
				update_info.version.major, update_info.version.minor, update_info.version.revision, update_info.version.build, update_info.description);

			update_info.update_available = true;
		}
		
	}

	// in any case we need to delete the tempfile so close it first
	JSON_close(&update_json_stream);
	fclose(update_json_handle);
	remove(&update_json_file_name);

	// destroy the curl object

	Netservices_DestroyCurlObject(update_json_curl_obj, true);
}

// Prompts for an update. Returns true if the user wanted to update



// The update prompt text.
char* update_prompt_format =
"An update is available for Zombono:\n"
"\n"
"Version %d.%d.%d.%d is now available.\n"
"Description: %s\n"
"\n"
"Would you like to update?\n"
"\n"
"(This UI is temporary and will be improved in the future.)";

bool Netservices_UpdaterPromptForUpdate()
{
	// Temporary UI
	char update_prompt[UPDATE_PROMPT_STR_LENGTH] = { 0 };

	snprintf(&update_prompt, UPDATE_PROMPT_STR_LENGTH, update_prompt_format,
		update_info.version.major, update_info.version.minor, update_info.version.revision, update_info.version.build,
		update_info.description);

	// cannot use constants, they are windows specific and this is platform independent code
	// TODO: convert to enum...
	int32_t buttons = Sys_Msgbox("Update Available", 4, &update_prompt); // 4 = MB_YESNO

	return (buttons == 6);
}

// Starts the update process. We can still get out at this point
void Netservices_UpdaterStartUpdate()
{
	char update_binary_path[DOWNLOAD_URL_STR_LENGTH] = { 0 };

	// generate a build url
	snprintf(&update_binary_path, DOWNLOAD_URL_STR_LENGTH, UPDATE_BINARY_BASE_URL "/" DOWNLOAD_URL_FORMAT,
		update_info.version.major, update_info.version.minor, update_info.version.revision, update_info.version.build,
		PLATFORMSTRING, selected_update_channel_str);

	// generate the local filename to be opened when we're done
	snprintf(&update_binary_file_name, DOWNLOAD_URL_STR_LENGTH, DOWNLOAD_URL_FORMAT,
		update_info.version.major, update_info.version.minor, update_info.version.revision, update_info.version.build,
		PLATFORMSTRING, selected_update_channel_str);

	Com_Printf("Downloading update package %s...\n", update_binary_path);

	// cannot be called if noupdatecheck is not set so dont bother
	update_binary_curl_obj = Netservices_AddCurlObject(update_binary_path, true, Netservices_UpdateInfoBinaryReceive);

	// override timeout because it's a large file
	curl_easy_setopt(update_binary_curl_obj, CURLOPT_TIMEOUT, 120000);
	
	// remove it if it already exists (we don't care about the result)
	remove(update_binary_file_name);

	update_binary_handle = fopen(update_binary_file_name, "wb");

	if (!update_binary_handle)
	{
		Sys_Msgbox("Update failed", 0, "Failed to update Zombono. Could not create update binary file!");
		return;
	}

	Netservices_SetOnCompleteCallback(Netservices_UpdateInfoBinaryComplete);
	Netservices_StartPendingTransfers();
}

void Netservices_UpdateInfoBinaryReceive(char* ptr, size_t size, size_t nmemb, char* userdata)
{
	// write to the update bvinary
	fwrite(ptr, nmemb, size, update_binary_handle);
	return nmemb;
}

void Netservices_UpdateInfoBinaryComplete()
{
	Com_Printf("Update package downloaded!\n");
	fclose(update_binary_handle);
	
	Netservices_UpdaterUpdateGame();
}

// Does not return so mark ti as such for the compiler
#ifdef _MSC_VER
__declspec(noreturn) void Netservices_UpdaterUpdateGame()
#else // gcc
__attribute((noreturn)) void Netservices_UpdaterUpdateGame()
#endif
{

}