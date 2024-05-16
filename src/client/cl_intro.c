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
#include "client.h"

int32_t intro_start_time = 0;
bool	intro_running;

void Intro_Start()
{
	intro_running = true; 

	intro_start_time = Sys_Milliseconds();
}

void Intro_Update()
{
	if (!intro_running)
		return;

	int32_t current_time = Sys_Milliseconds();
	int32_t end_time = intro_start_time + cl_intro1_time->value + cl_intro2_time->value;
	
	// end the intro if we reached the end time
	if (current_time > end_time)
		intro_running = false;

	// todo: fade

	// show first image
	if ((current_time - intro_start_time) <= cl_intro1_time->value)
	{
		re.DrawPicStretch(0, 0, viddef.width, viddef.height, cl_intro1->string);
	}
	else // show second image
	{
		re.DrawPicStretch(0, 0, viddef.width, viddef.height, cl_intro2->string);
	}
}

