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
extern qboolean	ui_active;																					// This is so we know to turn on the mouse cursor when a UI is being displayed.

// Current UI. A non-null value is enforced.
// Editing functions apply to this UI, as well as functions that are run on UI elements.
// You can only access UI elements through the current UI.
extern ui_t*	current_ui;

qboolean		UI_AddControl(char* name, int position_x, int position_y, int size_x, int size_y);			// Shared function that adds controls
ui_t*			UI_GetUI(char* name);																		// Returns a pointer so NULL can be indicated for failure
ui_control_t*	UI_GetControl(char* name);																	// Gets the control with name name in the current UI.

// Draw methods
void			UI_DrawText(ui_control_t text);																// Draws a text control.
void			UI_DrawImage(ui_control_t image);															// Draws an image control.
void			UI_DrawButton(ui_control_t button);															// Draws a button control.
void			UI_DrawSlider(ui_control_t slider);															// Draws a slider control.
void			UI_DrawCheckbox(ui_control_t checkbox);														// Draws a checkbox control.
void			UI_DrawBox(ui_control_t box);

qboolean UI_Init()
{
	// set everything to 0 in the case of reinitialisation e.g. failed to run singleplayer
	memset(&ui_list, 0x00, sizeof(ui_list));
	num_uis = 0;

	qboolean successful;

	Com_Printf("ZombonoUI is running UI creation scripts\n");
	successful = UI_AddUI("TeamUI", UI_CreateTeamUI);
	if (successful) successful = UI_AddUI("LeaderboardUI", UI_CreateLeaderboardUI);
	if (successful) successful = UI_AddUI("PostGameUI", UI_CreatePostGameUI);
	return successful;
}

qboolean UI_AddUI(char* name, qboolean(*on_create)())
{
	Com_DPrintf("Creating UI: %s\n", name);
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
		ui_t* current_ui = &ui_list[ui_num];

		if (!stricmp(current_ui->name, name))
		{
			return current_ui;
		}
	}

	return NULL;
}

ui_control_t* UI_GetControl(char* name)
{
	for (int ui_control_num = 0; ui_control_num < current_ui->num_controls; ui_control_num++)
	{
		ui_control_t* current_ui_control = &current_ui->controls[ui_control_num];

		if (!stricmp(current_ui_control->name, name))
		{
			return current_ui_control;
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

qboolean UI_AddText(char* name, char* text, int position_x, int position_y)
{
	ui_control_t* ui_control = &current_ui->controls[current_ui->num_controls];

	// not recommended to buffer overflow
	if (strlen(text) > MAX_UI_STR_LENGTH)
	{
		Com_Printf("Tried to set UI control text %s to %s - too long (max length %d)!", name, text, MAX_UI_STR_LENGTH);
		return false;
	}

	strcpy(ui_control->text, text);

	ui_control->type = ui_control_text;

	return UI_AddControl(name, position_x, position_y, 0, 0);
}

qboolean UI_AddImage(char* name, char* image_path, int position_x, int position_y, int size_x, int size_y)
{
	ui_control_t* ui_control = &current_ui->controls[current_ui->num_controls];

	// not recommended to buffer overflow
	if (strlen(image_path) > MAX_UI_STR_LENGTH)
	{
		Com_Printf("Tried to set UI control image path %s to %s - too long (max length %d)!", name, image_path, MAX_UI_STR_LENGTH);
		return false;
	}

	strcpy(ui_control->image_path, image_path);

	ui_control->type = ui_control_image;

	return UI_AddControl(name, position_x, position_y, size_x, size_y);
}

qboolean UI_AddButton(char* name, int position_x, int position_y, int size_x, int size_y)
{
	ui_control_t* ui_control = &current_ui->controls[current_ui->num_controls];
	ui_control->type = ui_control_button;

	return UI_AddControl(name, position_x, position_y, size_x, size_y);
}

qboolean UI_AddSlider(char* name, int position_x, int position_y, int size_x, int size_y, int value_min, int value_max)
{
	ui_control_t* ui_control = &current_ui->controls[current_ui->num_controls];
	ui_control->type = ui_control_slider;

	ui_control->value_min = value_min;
	ui_control->value_max = value_max;

	return UI_AddControl(name, position_x, position_y, size_x, size_y);
}

qboolean UI_AddCheckbox(char* name, int position_x, int position_y, int size_x, int size_y, qboolean checked)
{
	UI_AddControl(name, position_x, position_y, size_x, size_y);
	ui_control_t* ui_control = &current_ui->controls[current_ui->num_controls];

	ui_control->checked = checked;
	ui_control->type = ui_control_checkbox;

	return UI_AddControl(name, position_x, position_y, size_x, size_y);
}


qboolean UI_AddBox(char* name, int position_x, int position_y, int size_x, int size_y, int r, int g, int b, int a)
{
	UI_AddControl(name, position_x, position_y, size_x, size_y);
	ui_control_t* ui_control = &current_ui->controls[current_ui->num_controls];

	ui_control->color[0] = r;
	ui_control->color[1] = g;
	ui_control->color[2] = b;
	ui_control->color[3] = a;

	ui_control->type = ui_control_box;

	return UI_AddControl(name, position_x, position_y, size_x, size_y);
}

qboolean UI_SetEventOnClick(char* name, void (*func)(int btn, int x, int y))
{
	ui_control_t* ui_control = UI_GetControl(name);

	if (ui_control == NULL)
	{
		Com_Printf("ERROR: Tried to set unknown control on-click handler %s for UI %s!", name, current_ui->name);
		return false;
	}

	if (func == NULL)
	{
		Com_Printf("ERROR: Tried to set UI clicked event callback for control %s on UI %s to null!", ui_control->name, current_ui->name);
		return false;
	}

	ui_control->on_click = func;
	return true; 
}

void UI_HandleEventOnClick(int btn, int x, int y)
{
	// Current UI only
	for (int ui_control_num = 0; ui_control_num < current_ui->num_controls; ui_control_num++)
	{
		ui_control_t* ui_control = &current_ui->controls[ui_control_num];
		
		if (ui_control->on_click)
		{
			// todo: scale/??
			if (x >= ui_control->position_x
				&& y >= ui_control->position_y
				&& x <= ui_control->position_x + ui_control->size_x
				&& y <= ui_control->position_y + ui_control->size_y)
			{
				ui_control->on_click(btn, x, y);
			}
		}
	}
}

qboolean UI_SetEnabled(char* name, qboolean enabled)
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

qboolean UI_SetActive(char* name, qboolean active)
{
	ui_t* ui_ptr = UI_GetUI(name);

	if (ui_ptr != NULL)
	{
		ui_ptr->active = active;
		ui_active = active;

		// turn on the mouse cursor, bit hacky - we basically take over the mouse pointer when a UI is active
		IN_Activate(!active);

		current_ui = ui_ptr;
		return true; 
	}

	return false;
}

qboolean UI_SetText(char* name, char* text)
{
	ui_control_t* current_ui = UI_GetControl(name);

	if (current_ui == NULL)
	{
		Com_Printf("Tried to set NULL UI control text %s to %s!", name, text);
		return false;
	}

	if (strlen(text) > MAX_UI_STR_LENGTH)
	{
		Com_Printf("UI text for control %s, %s, was too long (max %d)", name, text, MAX_UI_STR_LENGTH);
		return false;
	}

	strcpy(current_ui->text, text);

	return true; 
}

qboolean UI_SetImage(char* name, char* image_path)
{
	ui_control_t* current_ui_control = UI_GetControl(name);

	if (current_ui_control == NULL)
	{
		Com_Printf("Tried to set NULL UI control image path %s to %s!", name, image_path);
		return false;
	}

	if (strlen(image_path) > MAX_UI_STR_LENGTH)
	{
		Com_Printf("UI image path for control %s, %s, was too long (max %d)", name, image_path, MAX_UI_STR_LENGTH);
		return false;
	}

	strcpy(current_ui_control->image_path, image_path);

	return true;
}

void UI_Clear(char* name)
{
	// clear every control but not the ui's info
	for (int ui_control_num = 0; ui_control_num < current_ui->num_controls; ui_control_num++)
	{
		ui_control_t* ui_control = &current_ui->controls[ui_control_num];
		memset(ui_control, 0x00, sizeof(ui_control_t));
	}

	current_ui->num_controls = 0;
	return true; 
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

	Draw_String(viddef.width - (8 * strlen(time)), 0, time);
	Draw_StringAlt(viddef.width - 144, 10, "Pre-release build!");

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
					case ui_control_box:
						UI_DrawBox(current_ui_control);
						break;
				}
			}
		}
	}
}

void UI_DrawText(ui_control_t text)
{
	Draw_String(text.position_x, text.position_y, text.text); 
}

void UI_DrawImage(ui_control_t image)
{
	re.DrawPic(image.position_x, image.position_y, image.image_path);
}

void UI_DrawButton(ui_control_t button)
{
	Com_Printf("UI: Buttons aren't implemented yet");
}

void UI_DrawSlider(ui_control_t slider)
{
	Com_Printf("UI: Sliders aren't implemented yet");
}

void UI_DrawCheckbox(ui_control_t checkbox)
{
	Com_Printf("UI: Checkboxes aren't implemented yet");
}

void UI_DrawBox(ui_control_t box)
{
	re.DrawFill(box.position_x, box.position_y, box.size_x, box.size_y, box.color[0], box.color[1], box.color[2], box.color[3]);
}