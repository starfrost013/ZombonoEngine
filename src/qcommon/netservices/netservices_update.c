#include "netservices.h"


#if defined(_DEBUG) || !defined(NDEBUG) // gcc
game_update_channel current_channel = update_channel_debug;
#elif PLAYTEST
game_update_channel current_channel = update_channel_playtest;
#else
game_update_channel current_channel = update_channel_release;
#endif

CURL* curl_transfer_update_json = { 0 };
CURL* curl_transfer_update_binary = { 0 }; // see how fast this is and if we need multiple simultaneous connections (curl_multitransfer_t)

// Does not block
game_update_t Netservices_UpdaterGetUpdate()
{
	game_update_t update_info = { 0 };

	curl_transfer_update_json = curl_easy_init();

	if (!curl_transfer_update_json)
		Sys_Error("Netservices_UpdaterGetUpdate: failed to initialise curl_transfer_update_json object");

	curl_multi_add_handle(curl_obj_multi, curl_transfer_update_json);
}

// DOES NOT RETURN
void Netservices_Update()
{

}