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
// netservices.h: Accounts, updating, master servers
// March 23, 2024

#pragma once
//CURL_STATICLIB MUST BE DEFINED ON COMMAND LINE OR JHERE
#include <qcommon/qcommon.h>
#include <qcommon/curl/curl.h>

#define UPDATER_BASE_URL			"https://updates.zombono.com"	// Base URL for the updater service
#define MAX_UPDATE_STR_LENGTH		256								// Maximum length of an update string

#define	ZOMBONO_USER_AGENT			"Zombono/" ZOMBONO_VERSION

extern char netservices_recv_buffer[CURL_MAX_WRITE_SIZE];			// The data actually received from the connect test.
// Cvars
// 
// All netservices cvars start with ns_* 
extern cvar_t*			ns_nointernetcheck;							// If true, no internet check will be performed.
extern cvar_t*			ns_noupdatecheck;							// If true, no update check will be performed.
extern cvar_t*			ns_disabled;								// If true, netservices will be entirely disabled. Zombono.com will not be contacted at all. Essentially acts as if Netservices_Init failed.

//
// netservices_base.c
//

// Globals
extern bool		netservices_connected;							// TRUE if you are connected to the internet and can use netservices, FALSE otherwise.
extern CURLM*	curl_obj;										// The curl multi object (used for multiple nonblocking transfers)

// Function
bool			Netservices_Init();								// Initialises Netservices and determines if we are connected to the internet.
// Sets up an easy curl object for use with a particular URL and the write callback write_callback
CURL*			Netservices_AddCurlObject(const char* url, bool multi, size_t write_callback(char* ptr, size_t size, size_t nmemb, char* userdata));
void			Netservices_DestroyCurlObject(CURL* object);	// Destroys the easy curl object represented by object
void			Netservices_StartTransfer();					// Starts the current netservices transfer
void			Netservices_Poll();								// Checks to see if a curl_multi_obj transfer is complete
void			Netservices_Shutdown();							// Shuts down netservices
//
// netservices_update.c
//

typedef enum game_update_channel_e
{
	update_channel_release = 0,								// Release update channel - regular players.

	update_channel_playtest = 1,							// Playtest update channel - invited testers / pre-0.1.0

	update_channel_debug = 2,								// Debug update channel - private tests / devs / custom builds to fix bugs
} game_update_channel;

// Defines a game version.
typedef struct game_version_s
{
	int32_t		major;										// The major version of the game.
	int32_t		minor;										// The minor version of the game.
	int32_t		revision;									// The revision version of the game.
	int32_t		build;										// Should we have this?
} game_version_t;


// Defines a game software update.
typedef struct game_update_s
{
	bool				update_available;					// When Netservices_UpdaterGetUpdate returns an instance of this struct, if this is TRUE, a newer version is available.
	game_update_channel	channel;							// The channel for which the update is available.
	game_version_t		version;							// The version of the game you are updating to.
	struct tm			release_date;						// Release date of the update.
	char				description[MAX_UPDATE_STR_LENGTH];	// A description of the update's features
} game_update_t;

extern game_update_channel	update_current_channel;			// The currently defined channel - corresponds with the engine's build config
extern game_update_t		update_info;					// The most recently obtained update information.

void			Netservices_UpdaterGetUpdate();				// Gets an Update. Returns a game_update_t structure containing update information.
void			Netservices_SetOnCompleteCallback(void on_complete(bool successful)); // Sets the current on-complete callback to use when performing a nonblocking Netservices transfer.

void			Netservices_UpdaterUpdateGame();			