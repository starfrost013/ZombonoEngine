#include "netservices.h"

#define UPDATE_JSON_URL UPDATER_BASE_URL "/updateinfo.json"

#if defined(_DEBUG) || !defined(NDEBUG) // gcc
game_update_channel current_channel = update_channel_debug;
#elif PLAYTEST
game_update_channel current_channel = update_channel_playtest;
#else
game_update_channel current_channel = update_channel_release;
#endif

CURL*	curl_transfer_update_json = { 0 };
CURL*	curl_transfer_update_binary = { 0 }; // see how fast this is and if we need multiple simultaneous connections (curl_multitransfer_t)

size_t	Netservices_UpdaterOnReceiveJson(char* ptr, size_t size, size_t nmemb, char* userdata);

int		netservices_running_transfers;

// tmpfile() handle
FILE*	update_json_handle;

// Does not block
game_update_t Netservices_UpdaterGetUpdate()
{
	game_update_t update_info = { 0 };

	curl_transfer_update_json = curl_easy_init();

	if (!curl_transfer_update_json)
		Sys_Error("Netservices_UpdaterGetUpdate: failed to initialise curl_transfer_update_json object");

	Netservices_AddCurlObject(UPDATE_JSON_URL, Netservices_UpdaterOnReceiveJson);
	Netservices_StartTransfer();
}

size_t Netservices_UpdaterOnReceiveJson(char* ptr, size_t size, size_t nmemb, char* userdata)
{
	if (nmemb >= CURL_MAX_WRITE_SIZE)
	{
		Com_Printf("Netservices_Init_UpdaterOnReceiveJson: nmemb > CURL_MAX_WRITE_SIZE");
		return nmemb;
	}

	strncpy(&netservices_recv_buffer, ptr, nmemb);
	netservices_recv_buffer[nmemb] = '\0'; // null terminate string (curl does not do that by default)
	
	// open a temporary file
	
	return nmemb;
}

// DOES NOT RETURN
void Netservices_Update()
{

}

void Netservices_UpdaterUpdateGame()
{

}