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
#include "qmenu.h" // todo: reorg

extern int		num_uis;
extern ui_t		ui_list[MAX_UIS];
extern qboolean	ui_active;																				// This is so we know to turn on the mouse cursor.

// Current UI. A non-null value is enforced.
// Editing functions apply to this UI, as well as functions that are run on UI elements.
// You can only access UI elements through the current UI.
extern ui_t*	current_ui;

qboolean	UI_AddControl(char* name, int position_x, int position_y, int size_x, int size_y);			// Shared function that adds controls
ui_t*		UI_GetUI(char* name);																		// Returns a pointer so NULL can be indicated for failure

// Draw methods
void		UI_DrawText(ui_control_t text);
void		UI_DrawImage(ui_control_t image);
void		UI_DrawButton(ui_control_t button);
void		UI_DrawSlider(ui_control_t slider);
void		UI_DrawCheckbox(ui_control_t checkbox);

qboolean UI_Init()
{
	// set everything to 0 in the case of reinitialisation e.g. failed to run singleplayer
	memset(&ui_list, 0x00, sizeof(ui_list));
	num_uis = 0;

	qboolean successful;

	Com_Printf("ZombonoUI is running UI creation scripts\n");
	successful = UI_AddUI("TeamUI", UI_CreateTeamUI);

	return successful;
}

qboolean UI_AddUI(char* name, qboolean(*on_create)())
{
	current_ui = &ui_list[num_uis];

	if (num_uis > MAX_UIS)
	{
		Sys_Error("Tried to create a UI when there are more than %d UIs!", MAX_UIS);
		return false; 
	}

	num_uis++;

	if (strlen(name) > MAX_UI_STR_LENGTH)
	{
		Sys_Error("Tried to create a UI with name more than %d characters!", MAX_UI_STR_LENGTH);
		return false;
	}

	strcpy(current_ui->name, name);

	if (on_create == NULL)
	{
		Sys_Error("Tried to create a UI with no creation function!");
		return false;
	}

	current_ui->on_create = on_create;

	if (!on_create())
	{
		Sys_Error("UI create function failed for UI %s", name); // should these be fatal?
		return false; 
	}

	return true;
}

// return a pointer to a UI
ui_t* UI_GetUI(char* name)
{
	for (int ui_num = 0; ui_num < num_uis; ui_num++)
	{
		ui_t current_ui = ui_list[ui_num];

		if (!strcmp(current_ui.name, name))
		{
			return &ui_list[ui_num];
		}
	}

	return NULL;
}

qboolean UI_AddControl(char* name, int position_x, int position_y, int size_x, int size_y)
{
	if (current_ui->num_controls >= CONTROLS_PER_UI)
	{
		Sys_Error("Tried to add too many controls to the UI %s", current_ui->name);
		return false;
	}

	ui_control_t* current_control = &current_ui->controls[current_ui->num_controls];
	current_ui->num_controls++;

	strcpy(current_control->name, name);

	current_control->position_x = position_x;
	current_control->position_y = position_y;
	current_control->size_x = size_x;
	current_control->size_y = size_y;
	 
	return true;
}

qboolean UI_AddText(const char* name, char* text, int position_x, int position_y)
{
	ui_control_t* ui_control = &current_ui->controls[current_ui->num_controls];
	strcpy(ui_control->text, text);
	ui_control->type = ui_control_text;

	return UI_AddControl(name, position_x, position_y, 0, 0);
}

qboolean UI_AddImage(const char* name, char* image_path, int position_x, int position_y, int size_x, int size_y)
{
	ui_control_t* ui_control = &current_ui->controls[current_ui->num_controls];

	strcpy(ui_control->image_path, image_path);

	ui_control->type = ui_control_image;

	return UI_AddControl(name, position_x, position_y, size_x, size_y);
}

qboolean UI_AddButton(const char* name, int position_x, int position_y, int size_x, int size_y)
{
	ui_control_t* ui_control = &current_ui->controls[current_ui->num_controls];
	ui_control->type = ui_control_button;

	return UI_AddControl(name, position_x, position_y, size_x, size_y);
}

qboolean UI_AddSlider(const char* name, int position_x, int position_y, int size_x, int size_y, int value_min, int value_max)
{
	ui_control_t* ui_control = &current_ui->controls[current_ui->num_controls];
	ui_control->type = ui_control_slider;

	ui_control->value_min = value_min;
	ui_control->value_max = value_max;

	return UI_AddControl(name, position_x, position_y, size_x, size_y);
}

qboolean UI_AddCheckbox(const char* name, int position_x, int position_y, int size_x, int size_y, qboolean checked)
{
	UI_AddControl(name, position_x, position_y, size_x, size_y);
	ui_control_t* ui_control = &current_ui->controls[current_ui->num_controls];

	ui_control->checked = checked;
	ui_control->type = ui_control_checkbox;

	return UI_AddControl(name, position_x, position_y, size_x, size_y);
}

qboolean UI_SetEventOnClick(void (*func)(int x, int y))
{
	ui_control_t* ui_control = &current_ui->controls[current_ui->num_controls];

	if (func == NULL)
	{
		Sys_Error("Tried to set UI clicked event callback to null!");
		return false;
	}

	ui_control->on_click = func;
	return true; 
}

qboolean UI_HandleEventOnClick(int x, int y)
{
	// Current UI only
	for (int ui_num = 0; ui_num < current_ui->num_controls; ui_num++)
	{
		ui_control_t* ui_control = &current_ui->controls[current_ui->num_controls];
		
		if (ui_control->on_click)
		{
			// todo: scale/??
			if (x >= ui_control->position_x
				&& y >= ui_control->position_y
				&& x <= ui_control->position_x + ui_control->size_x
				&& y <= ui_control->position_y + ui_control->size_y)
			{
				ui_control->on_click(x, y);
			}
		}
	}
}

qboolean UI_SetEnabled(const char* name, qboolean enabled)
{
	ui_t* ui_ptr = UI_GetUI(name);

	if (ui_ptr != NULL)
	{
		ui_ptr->enabled = enabled;
		current_ui = ui_ptr;
		return true; 
	}

	return false;
}

qboolean UI_SetActive(const char* name, qboolean active)
{
	ui_t* ui_ptr = UI_GetUI(name);

	if (ui_ptr != NULL)
	{
		ui_ptr->active = active;
		ui_active = active;

		// hack!
		IN_Activate(!active);

		current_ui = ui_ptr;
		return true; 
	}

	return false;
}

void UI_Draw()
{

// playtest indicator
	
// this is NOT!! efficient don't do this (esp. getting the length of the string every frame and stuff) but not used in release 
#if defined(PLAYTEST) || defined(_DEBUG)
	time_t		raw_time;
	struct tm*	local_time;

	time(&raw_time);
	local_time = localtime(&raw_time);
	char		time[128];
#ifdef PLAYTEST
	strftime(&time, 128, "Playtest Build v" ZOMBONO_VERSION " (%b %d %Y %H:%M:%S)", local_time);
#elif _DEBUG
	strftime(&time, 128, "Debug Build v" ZOMBONO_VERSION " (%b %d %Y %H:%M:%S)", local_time);
#endif

	Menu_DrawString(viddef.width - (8 * strlen(time)), 0, time);
	Menu_DrawStringDark(viddef.width - 144, 10, "Pre-release build!");

#endif

	for (int ui_num = 0; ui_num < num_uis; ui_num++)
	{
		// draw the current UI if enabled (*ACTIVE* means it's receiving input events0
		ui_t current_ui = ui_list[ui_num];

		if (current_ui.enabled)
		{
			for (int ui_control_num = 0; ui_control_num < current_ui.num_controls; ui_control_num++)
			{
				ui_control_t current_ui_control = current_ui.controls[ui_control_num];
				
				switch (current_ui_control.type)
				{
					case ui_control_text:
						UI_DrawText(current_ui_control);
						break;
					case ui_control_image:
						UI_DrawImage(current_ui_control);
						break;
					case ui_control_button:
						UI_DrawButton(current_ui_control);
						break;
					case ui_control_checkbox:
						UI_DrawCheckbox(current_ui_control);
						break;
					case ui_control_slider:
						UI_DrawSlider(current_ui_control);
						break;
				}
			}
		}
	}
}

void UI_DrawText(ui_control_t text)
{
	Menu_DrawString(text.position_x, text.position_y, text.text); 
}

void UI_DrawImage(ui_control_t image)
{
	re.DrawPic(image.position_x, image.position_y, image.image_path);
}

void UI_DrawButton(ui_control_t button)
{

}

void UI_DrawSlider(ui_control_t slider)
{

}

void UI_DrawCheckbox(ui_control_t checkbox)
{

}