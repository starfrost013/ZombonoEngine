#include "netservices.h"

// Defines
#define UPDATE_JSON_URL UPDATER_BASE_URL "/updateinfo.json"

// Functions only used in this file
size_t	Netservices_UpdateInfoJsonReceive(char* ptr, size_t size, size_t nmemb, char* userdata);
void	Netservices_UpdateInfoJsonComplete(bool successful);

// Globals

// Set the update channel based on the build configuration
#if defined(_DEBUG) || !defined(NDEBUG) // gcc
game_update_channel current_channel = update_channel_debug;
#elif PLAYTEST
game_update_channel current_channel = update_channel_playtest;
#else
game_update_channel current_channel = update_channel_release;
#endif

CURL*			curl_transfer_update_json = { 0 };
CURL*			curl_transfer_update_binary = { 0 }; // see how fast this is and if we need multiple simultaneous connections (curl_multitransfer_t)

int				netservices_running_transfers;

// tmpfile() handle
FILE*			update_json_handle;						// updateinfo.json file handle
char*			update_json_file_name[L_tmpnam];		// updateinfo.json temp file name

// stores available update information
game_update_t	update_info = { 0 };

// Does not block
void Netservices_UpdaterGetUpdate()
{
	Com_Printf("Checking for updates...\n");

	curl_transfer_update_json = Netservices_AddCurlObject(UPDATE_JSON_URL, true, Netservices_UpdateInfoJsonReceive);

	// create a temporary file
	tmpnam(update_json_file_name);
	
	// tell the user about this but not stop them from playing the game
	if (update_json_file_name == NULL)
	{
		Sys_Msgbox("Non-Fatal Error", 0, "Failed to create Netservices UpdateInfo.json - couldn't get a temp file name!");
		return;
	}

	update_json_handle = fopen(update_json_file_name, "w");

	if (update_json_handle == NULL)
	{
		Sys_Msgbox("Non-Fatal Error", 0, "Failed to create Netservices UpdateInfo.json - couldn't create a temp file!");
		return;
	}

	// tell netservices to call the "is an update available?" function
	Netservices_SetOnCompleteCallback(Netservices_UpdateInfoJsonComplete);
	
	// and start the transfer...
	Netservices_StartTransfer();
}

size_t Netservices_UpdateInfoJsonReceive(char* ptr, size_t size, size_t nmemb, char* userdata)
{
	if (nmemb >= CURL_MAX_WRITE_SIZE)
	{
		Com_Printf("Netservices_Init_UpdaterOnReceiveJson: nmemb > CURL_MAX_WRITE_SIZE");
		return nmemb;
	}

	strncpy(&netservices_recv_buffer, ptr, nmemb);
	netservices_recv_buffer[nmemb] = '\0'; // null terminate string (curl does not do that by default)

	// write it
	// we close it in the complete callback because this can be called many times (only 16kb is transferred at a time in CURL)
	fwrite(ptr, nmemb, size, update_json_handle);
	return nmemb;
}

// sets update_json.update_available to true
void Netservices_UpdateInfoJsonComplete(bool successful)
{
	Com_DPrintf("Downloaded update information to tmpfile %s", update_json_file_name);

	JSON_stream update_json_stream;
	// close the file

	fclose(update_json_handle);

	if (!successful)
	{
		Sys_Msgbox("Non-Fatal Error", 0, "Update failed - failed to download updateinfo");
		return;
	}
	else
	{
		// see if there really is an update availale
	}
}

// Does not return
#ifdef _MSC_VER
__declspec(noreturn) void Netservices_UpdaterUpdateGame()
#else // gcc
__attribute((noreturn)) void Netservices_UpdaterUpdateGame()
#endif
{

}