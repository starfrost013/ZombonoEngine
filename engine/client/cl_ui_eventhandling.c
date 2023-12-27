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

// cl_ui_eventhandling.c : Split UI Event Handling from UI Code (December 27, 2023)
#include "client.h"

qboolean UI_SetEventOnClick(char* ui_name, char* name, void (*func)(int btn, int x, int y))
{
	ui_control_t* ui_control_ptr = UI_GetControl(ui_name, name);

	if (!ui_control_ptr)
	{
		Com_Printf("ERROR: Tried to set unknown control on-click handler %s for UI %s!", name, current_ui->name);
		return false;
	}

	if (func == NULL)
	{
		Com_Printf("ERROR: Tried to set UI clicked event callback for control %s on UI %s to null!", ui_control_ptr->name, current_ui->name);
		return false;
	}

	ui_control_ptr->on_click = func;
	return true;
}

qboolean UI_SetEventOnKeyDown(char* ui_name, char* name, void (*func)(int btn))
{
	ui_control_t* ui_control_ptr = UI_GetControl(ui_name, name);

	if (!ui_control_ptr)
	{
		Com_Printf("ERROR: Tried to set unknown control on-click handler %s for UI %s!", name, current_ui->name);
		return false;
	}

	if (func == NULL)
	{
		Com_Printf("ERROR: Tried to set UI clicked event callback for control %s on UI %s to null!", ui_control_ptr->name, current_ui->name);
		return false;
	}

	ui_control_ptr->on_key_down = func;
	return true;
}

void UI_HandleEventOnClick(int btn, int x, int y)
{
	if (current_ui == NULL) return;

	for (int ui_control_num = 0; ui_control_num < current_ui->num_controls; ui_control_num++)
	{
		ui_control_t* ui_control_ptr = &current_ui->controls[ui_control_num];

		// Handle focus changes for key events
		// TODO: Scaling
		if (x >= ui_control_ptr->position_x
			&& y >= ui_control_ptr->position_y
			&& x <= ui_control_ptr->position_x + ui_control_ptr->size_x
			&& y <= ui_control_ptr->position_y + ui_control_ptr->size_y)
		{
			ui_control_ptr->focused = true;

			// if the UI has an onclick event handler, call it
			if (ui_control_ptr->on_click)
			{
				ui_control_ptr->on_click(btn, x, y);
			}

			// UI may be disabled
			if (current_ui == NULL) return;
		}
		else
		{
			ui_control_ptr->focused = false;
		}
	}
}

void UI_HandleEventOnKeyDown(int btn)
{
	if (current_ui == NULL) return;

	for (int ui_control_num = 0; ui_control_num < current_ui->num_controls; ui_control_num++)
	{
		ui_control_t* ui_control_ptr = &current_ui->controls[ui_control_num];

		// checking for focusing is the choice of each individual UI
		if (ui_control_ptr->on_key_down)
		{
			ui_control_ptr->on_key_down(btn);
		}
	}
}