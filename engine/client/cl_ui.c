/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2023      starfrost

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
// cl_ui.c -- ZombonoUI (December 9, 2023)

#include "client.h"

qboolean UI_Init()
{

}

qboolean UI_AddUI(char* name)
{

}

qboolean UI_AddText(const char* name, char* text, int position_x, int position_y)
{

}

qboolean UI_AddImage(const char* name, char* image_path, int position_x, int position_y, int size_x, int size_y)
{

}

qboolean UI_AddButton(const char* name, int position_x, int position_y, int size_x, int size_y)
{

}

qboolean UI_AddSlider(const char* name, int position_x, int position_y, int size_x, int size_y, int min_value, int max_value)
{

}

qboolean UI_AddCheckbox(const char* name, int position_x, int position_y, int size_x, int size_y, qboolean checked)
{

}

qboolean UI_SetOnClicked(const char* name, void (*func)())
{

}

qboolean UI_SetEnabled(const char* name, qboolean enabled)
{

}

qboolean UI_SetActive(const char* name, qboolean enabled)
{

}

void UI_Draw()
{

}

qboolean UI_Done()
{

}