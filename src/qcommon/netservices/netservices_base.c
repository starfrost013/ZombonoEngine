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
// netservices_base.c: Netservices base and connection test

#include "netservices.h"

// Set to true in Netservices_Init()
bool netservices_connected = false;					// Determines if netservices is initialised and you are connected to the internet

// String downloaded from the below url to 
const char* connect_test_string = "This is a connect test file for Zombono Network Services";
const char* connect_test_url = UPDATER_BASE_URL "/connecttest.txt"; // Just use updater service for thi
char	netservices_recv_buffer[CURL_MAX_WRITE_SIZE];			// Buffer to use for receiving data from curl
	
char	connect_test_error_buffer[CURL_ERROR_SIZE];			// Error string buffer returned by CURL functions

//TODO: EXTREMELY NON-REENTRANT BUT WE DO NOT HAVE MULTITHREADING ANYWAY

cvar_t* ns_disabled;										// If true, don't ever contact zombono.com
cvar_t* ns_nointernetcheck;									// If true, don't perform an internet check
cvar_t* ns_noupdatecheck;									// If true, don't perform an update check

CURL*	curl_obj_easy;										// The single blocking transfer curl interface object
CURLM*	curl_obj;											// The multi nonblocking transfer curl interface object

int32_t	netservices_running_transfers;						// The number of curl transfers currently running.

void	(*netservices_on_complete_callback)(bool successful);	// The callback to use when the current transfer is complete.

// functions only used within this file
size_t Netservices_Init_WriteCallback(char *ptr, size_t size, size_t nmemb, char* received_data);				// Callback function on CURL receive

// This function runs before SV_Init and CL_Init, so take that into account
bool Netservices_Init()
{
	ns_nointernetcheck = Cvar_Get("ns_nointernetcheck", "0", CVAR_ARCHIVE);
	ns_noupdatecheck = Cvar_Get("ns_noupdatecheck", "0", CVAR_ARCHIVE);
	ns_disabled = Cvar_Get("ns_disabled", "0", CVAR_ARCHIVE);

	if (ns_nointernetcheck->value
		|| ns_disabled->value)
	{
		// assume we are connected unless ns_disabled is set
		if (ns_disabled->value)
		{
			return false;
		}

		return true;
	}

	Com_Printf("Netservices_Init: Determining if you are connected to the Internet...\n");

	// init both curl objects

	curl_obj_easy = Netservices_AddCurlObject(connect_test_url, false, Netservices_Init_WriteCallback);
	curl_obj = curl_multi_init();

	if (!curl_obj_easy)
	{
		Sys_Error("CURL failed to initialise. Updating, master servers, and accounts won't be available.");
		return false;
	}

	// get the file - blocking for now - this should take a max of 2s
	CURLcode error_code = curl_easy_perform(curl_obj_easy);

	if (error_code != CURLE_OK)
	{
		Com_Printf("Internet connection check failed. Disabling netservices... (%s)\n", connect_test_error_buffer);
		return false; 
	}

	if (strncmp(&netservices_recv_buffer, connect_test_string, strlen(connect_test_string)))
	{
		Com_Printf("Received invalid connect test string from updates.zombono.com (%s, not %s)\n", netservices_recv_buffer, connect_test_string);
		return false;
	}
	
	// destroy the 

	Netservices_DestroyCurlObject(curl_obj_easy);

	Com_Printf("Netservices_Init: Connected to netservices successfully.\n");
	netservices_connected = true;
	return true;
}

CURL* Netservices_AddCurlObject(const char* url, bool multi, size_t write_callback(char* ptr, size_t size, size_t nmemb, char* userdata))
{
	CURL* new_obj = curl_easy_init();

	// 40 byte text file should take maximum of 2 seconds to download to minimise user wait time
	if (curl_easy_setopt(new_obj, CURLOPT_TIMEOUT_MS, 2000))
		return NULL;

	if (curl_easy_setopt(new_obj, CURLOPT_URL, url))
		return NULL;

	if (curl_easy_setopt(new_obj, CURLOPT_WRITEFUNCTION, write_callback))
		return NULL;

	if (curl_easy_setopt(new_obj, CURLOPT_USERAGENT, ZOMBONO_USER_AGENT))
		return NULL;

	if (curl_easy_setopt(new_obj, CURLOPT_ERRORBUFFER, &connect_test_error_buffer))
		return NULL;

	if (multi)
	{
		if (curl_multi_add_handle(curl_obj, new_obj))
		{
			return NULL;
		}
	}

	return new_obj;
}

void Netservices_DestroyCurlObject(CURL* object)
{
	// this is a function as we intend to do more stuff here later
	curl_easy_cleanup(object);
}

void Netservices_SetOnCompleteCallback(void on_complete(bool successful))
{
	netservices_on_complete_callback = on_complete;
}

void Netservices_StartTransfer()
{
	// perform one perform to get things going
	CURLMcode error_code = curl_multi_perform(curl_obj, &netservices_running_transfers);

	if (error_code)
	{
		Com_Printf("Initial curl_multi_perform failed %d", error_code);
	}
}

// Poll
void Netservices_Poll()
{
	// if there is nothing to return (NULL means it was never set)
	if (netservices_running_transfers == 0
		|| netservices_on_complete_callback == NULL)
	{
		return;
	}
		
	CURLMcode err_code = curl_multi_perform(curl_obj, &netservices_running_transfers);

	if (err_code != CURLM_OK)
	{
		Com_Printf("curl_multi_perform failed %d (%d running transfers). Stopping transfers...", err_code);
		return;
	}

	if (netservices_running_transfers > 0)
	{
		err_code = curl_multi_poll(curl_obj, NULL, 0, 0, NULL); // we cannot wait as this runs while the game is running as a part of the game loop

		if (err_code)
		{
			Com_Printf("curl_multi_poll failed %d (%d running transfers). Stopping transfers...", err_code);
		}
	}
	// if we have just finished run the oncomplete function
	else
	{
		netservices_on_complete_callback(err_code == CURLE_OK);
	}
}

size_t Netservices_Init_WriteCallback(char* ptr, size_t size, size_t nmemb, char* userdata)
{
	if (nmemb >= CURL_MAX_WRITE_SIZE)
	{
		Com_Printf("Netservices_Init_WriteCallback: nmemb (%d) > CURL_MAX_WRITE_SIZE (%d)", nmemb, CURL_MAX_WRITE_SIZE);
 		return nmemb;
	}

	strncpy(&netservices_recv_buffer, ptr, nmemb);
	netservices_recv_buffer[nmemb] = '\0'; // null terminate string (curl does not do that by default)

	return nmemb;
}

void Netservices_Shutdown()
{
	Netservices_DestroyCurlObject(curl_obj_easy);
	curl_multi_cleanup(curl_obj);
}