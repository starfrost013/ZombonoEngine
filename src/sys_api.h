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

#pragma once
#include <common/common.h>

// sys_api.h: Provides system-specific APIs, so euphoriacommon can use them
// // September 21, 2024

#define SYS_API_VERSION		1

typedef struct sys_api_s
{
	char*	(*Sys_ConsoleInput);
	void	(*Sys_ConsoleOutput)(char* string);
	void	(*Sys_Error)(char* error, ...);
	void	(*Sys_Init)();
	int32_t	(*Sys_Milliseconds)();
	int32_t	(*Sys_Msgbox)(char* title, uint32_t buttons, char* text, ...);
	int64_t	(*Sys_Nanoseconds)();
	void	(*Sys_Quit)();
} sys_api_t;

extern sys_api_t system;

void SystemAPI_Init();
sys_api_t SystemAPI_Get();