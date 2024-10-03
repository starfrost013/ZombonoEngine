/*
Euphoria Game Engine
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
// client_api.h: Client interface object for euphoriacommon import
// September 30, 2024

#pragma once 

#include <stdint.h>
#include <stdbool.h>
#include <client/client.h>

#define CLIENT_API_VERSION	1

typedef struct client_api_export_s
{
	int32_t	api_version;
	
	void	(*CL_Init)();						// Initialise the client
	void	(*CL_Frame)(int32_t msec);			// Run a frame.
	void	(*CL_Drop)();						// Drop a client
	void	(*CL_Shutdown)();					// Shutdown the client
	void	(*CL_ForwardCmdToServer)();			// Forward a client command to the server

	void	(*Con_Print)();						// Print to the console - will move this to common eventually but the console is basically a graphical subsystem and is separate to logging

} client_api_export_t;

extern client_api_export_t client;

void ClientAPI_Init();
client_api_export_t ClientAPI_Get();