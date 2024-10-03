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
#include <sys_api.h>

// sys_api.h: Provides system-specific APIs, so euphoriacommon can use them
// // September 21, 2024

sys_api_t system;

void SystemAPI_Init()
{
	system.Sys_ConsoleInput = Sys_ConsoleInput;
	system.Sys_ConsoleOutput = Sys_ConsoleOutput;
	system.Sys_Error = Sys_Error; 
	system.Sys_Init = Sys_Init;
	system.Sys_Milliseconds = Sys_Milliseconds;
	system.Sys_Msgbox = Sys_Msgbox;
	system.Sys_Nanoseconds = Sys_Nanoseconds;
	system.Sys_Quit = Sys_Quit;
}

sys_api_t SystemAPI_Get()
{
	return system;
}