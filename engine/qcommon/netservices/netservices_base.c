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
char netservices_recv_buffer[CURL_MAX_WRITE_SIZE];	// Buffer to use for receiving data from curl

// debug build only (both msvc and gcc)
#ifndef NDEBUG
char connect_test_error_buffer[CURL_ERROR_SIZE];	// Error string buffer returned by CURL functions
#endif

cvar_t* ns_disabled;								// If true, don't e
cvar_t* ns_nointernetcheck;							// If true, don't perform an internet check
cvar_t* ns_noupdatecheck;							// If true, don't perform an update check

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

	// just easy init for this test
	CURL* curl_obj = curl_easy_init();

	if (!curl_obj)
	{
		Sys_Error("CURL failed to initialise. You can run the game by adding the line\n\"ns_disabled 1\"\nto your config.cfg or default.cfg files"
			", but updating, master servers, and accounts won't be available.");
		return false;
	}

	// 40 byte text file should take maximum of 2 seconds to download to minimise user wait time
	if (curl_easy_setopt(curl_obj, CURLOPT_TIMEOUT_MS, 2000))
		return false;

	if (curl_easy_setopt(curl_obj, CURLOPT_URL, connect_test_url))
		return false;

	if (curl_easy_setopt(curl_obj, CURLOPT_WRITEFUNCTION, Netservices_Init_WriteCallback))
		return false;

#ifndef NDEBUG
	if (curl_easy_setopt(curl_obj, CURLOPT_ERRORBUFFER, &connect_test_error_buffer))
		return false;
#endif
	// get the file - blocking for now - this should take a max of 2s
	CURLcode error_code = curl_easy_perform(curl_obj);

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

	netservices_connected = true;
	return true;
}

size_t Netservices_Init_WriteCallback(char* ptr, size_t size, size_t nmemb, char* userdata)
{
	if (nmemb >= CURL_MAX_WRITE_SIZE)
	{
		Com_Printf("Netservices_Init_WriteCallback: nmemb > CURL_MAX_WRITE_SIZE");
 		return nmemb;
	}

	strncpy(&netservices_recv_buffer, ptr, nmemb);
	netservices_recv_buffer[nmemb] = '\0'; // null terminate string (curl does not do that by default)

	return nmemb;
}

void Netservices_Shutdown()
{
	curl_easy_cleanup(curl_obj);
}