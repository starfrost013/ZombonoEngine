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
// cl_ui.c -- ZombonoUI (December 9, 2023)

#include "client.h"

// Globals
// see client.h for explanations on what these are 
int			num_uis;																					
ui_t		ui_list[MAX_UIS];
qboolean	ui_active = false;																				// This is so we know to turn on the mouse cursor when a UI is being displayed.

// Current UI. A non-null value is enforced.
// Editing functions apply to this UI, as well as functions that are run on UI elements.
// You can only access UI elements through the current UI.
ui_t*	current_ui;

qboolean		UI_AddControl(ui_t* ui, char* name, int position_x, int position_y, int size_x, int size_y);// Shared function that adds controls.

// Draw methods
void			UI_DrawText(ui_control_t* text);															// Draws a text control.
void			UI_DrawImage(ui_control_t* image);															// Draws an image control.
void			UI_DrawSlider(ui_control_t* slider);														// Draws a slider control.
void			UI_DrawCheckbox(ui_control_t* checkbox);													// Draws a checkbox control.
void			UI_DrawBox(ui_control_t* box);

qboolean UI_Init()
{
	// they are not statically initalised here because UI gets reinit'd on vidmode change so we need to wipe everything clean
	memset(&ui_list, 0x00, sizeof(ui_t) * num_uis); // only clear the uis that actually exist
	num_uis = 0;
	qboolean successful;

	Com_Printf("ZombonoUI is running UI creation scripts\n");
	successful = UI_AddUI("TeamUI", UI_TeamUICreate);
	if (successful) successful = UI_AddUI("LeaderboardUI", UI_LeaderboardUICreate);
	if (successful) successful = UI_AddUI("BamfuslicatorUI", UI_BamfuslicatorUICreate);
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
		ui_t* ui_ptr = &ui_list[ui_num];

		if (!stricmp(ui_ptr->name, name))
		{
			return ui_ptr;
		}
	}

	Com_Printf("UI_GetUI: Couldn't find UI: %s\n", name);
	return NULL;
}

// get a UI control on the UI ui
ui_control_t* UI_GetControl(char* ui_name, char* name)
{
	ui_t* ui_ptr = UI_GetUI(ui_name);

	if (!ui_ptr)
	{
		// message already pRinted
		return NULL; 
	}

	for (int ui_control_num = 0; ui_control_num < ui_ptr->num_controls; ui_control_num++)
	{
		ui_control_t* ui_control_ptr = &ui_ptr->controls[ui_control_num];

		if (!stricmp(ui_control_ptr->name, name))
		{
			return ui_control_ptr;
		}
	}

	return NULL;
}

qboolean UI_AddControl(ui_t* ui_ptr, char* name, int position_x, int position_y, int size_x, int size_y)
{
	if (ui_ptr->num_controls >= CONTROLS_PER_UI)
	{
		Sys_Error("Tried to add too many controls to the UI %s, max %d", ui_ptr->name, ui_ptr->num_controls);
		return false;
	}

	ui_control_t* current_control = &ui_ptr->controls[ui_ptr->num_controls];
	ui_ptr->num_controls++;

	strcpy(current_control->name, name);

	current_control->position_x = position_x;
	current_control->position_y = position_y;
	current_control->size_x = size_x;
	current_control->size_y = size_y;
	 
	return true;
}

qboolean UI_AddText(char* ui_name, char* name, char* text, int position_x, int position_y)
{
	ui_t* ui_ptr = UI_GetUI(ui_name);

	if (!ui_ptr)
	{
		// message already printed
		return false;
	}

	ui_control_t* ui_control = &ui_ptr->controls[ui_ptr->num_controls];

	// not recommended to buffer overflow
	if (strlen(text) > MAX_UI_STR_LENGTH)
	{
		Com_Printf("Tried to set UI control text %s to %s - too long (max length %d)!\n", name, text, MAX_UI_STR_LENGTH);
		return false;
	}

	strcpy(ui_control->text, text);

	ui_control->type = ui_control_text;

	return UI_AddControl(ui_ptr, name, position_x, position_y, 0, 0);
}

qboolean UI_AddImage(char* ui_name, char* name, char* image_path, int position_x, int position_y, int size_x, int size_y)
{
	ui_t* ui_ptr = UI_GetUI(ui_name);

	if (!ui_ptr)
	{
		// message already printed
		return false;
	}

	ui_control_t* ui_control = &ui_ptr->controls[ui_ptr->num_controls];

	// not recommended to buffer overflow
	if (strlen(image_path) > MAX_UI_STR_LENGTH)
	{
		Com_Printf("The UI control %s's image path %s is too long (max length %d)!\n", name, image_path, MAX_UI_STR_LENGTH);
		return false;
	}

	strcpy(ui_control->image_path, image_path);

	ui_control->type = ui_control_image;

	return UI_AddControl(ui_ptr, name, position_x, position_y, size_x, size_y);
}

qboolean UI_AddSlider(char* ui_name, char* name, int position_x, int position_y, int size_x, int size_y, int value_min, int value_max)
{
	ui_t* ui_ptr = UI_GetUI(ui_name);

	if (!ui_ptr)
	{
		// message already printed
		return false;
	}

	ui_control_t* ui_control = &ui_ptr->controls[ui_ptr->num_controls];
	ui_control->type = ui_control_slider;

	ui_control->value_min = value_min;
	ui_control->value_max = value_max;

	return UI_AddControl(ui_ptr, name, position_x, position_y, size_x, size_y);
}

qboolean UI_AddCheckbox(char* ui_name, char* name, int position_x, int position_y, int size_x, int size_y, qboolean checked)
{
	ui_t* ui_ptr = UI_GetUI(ui_name);

	if (!ui_ptr)
	{
		// message already printed
		return false;
	}

	ui_control_t* ui_control = &ui_ptr->controls[ui_ptr->num_controls];

	ui_control->checked = checked;
	ui_control->type = ui_control_checkbox;

	return UI_AddControl(ui_ptr, name, position_x, position_y, size_x, size_y);
}

qboolean UI_AddBox(char* ui_name, char* name, int position_x, int position_y, int size_x, int size_y, int r, int g, int b, int a)
{
	ui_t* ui_ptr = UI_GetUI(ui_name);

	if (!ui_ptr)
	{
		// message already printed
		return false;
	}

	ui_control_t* ui_control = &ui_ptr->controls[ui_ptr->num_controls];

	ui_control->color[0] = r;
	ui_control->color[1] = g;
	ui_control->color[2] = b;
	ui_control->color[3] = a;

	ui_control->type = ui_control_box;

	return UI_AddControl(ui_ptr, name, position_x, position_y, size_x, size_y);
}

qboolean UI_SetEnabled(char* ui_name, qboolean enabled)
{
	ui_t* ui_ptr = UI_GetUI(ui_name);

	if (ui_ptr != NULL)
	{
		// otherwise enable the ui
		ui_ptr->enabled = enabled;

		current_ui = (ui_ptr->enabled) ? ui_ptr : NULL;
	}

	return false;
}

qboolean UI_SetActive(char* ui_name, qboolean active)
{
	ui_t* ui_ptr = UI_GetUI(ui_name);

	if (ui_ptr != NULL)
	{
		ui_ptr->active = active;
		ui_active = active;

		// if the UI requires the mouse....
		if (!ui_ptr->passive)
		{
			// turn on the mouse cursor, bit hacky - we basically take over the mouse pointer when a UI is active
			IN_Activate(!active);
		}

		current_ui = ui_ptr;
		return true; 
	}

	return false;
}

// set UI ui_name passivity to passive
qboolean UI_SetPassive(char* ui_name, qboolean passive)
{
	ui_t* ui_ptr = UI_GetUI(ui_name);

	if (ui_ptr != NULL)
	{
		ui_ptr->passive = passive; 
		return true;
	}

	return false;
}

qboolean UI_SetText(char* ui_name, char* name, char* text)
{
	ui_control_t* ui_control_ptr = UI_GetControl(ui_name, name);

	if (ui_control_ptr == NULL)
	{
		Com_Printf("Couldn't find UI control %s to set text to %s!\n", name, text);
		return false;
	}

	if (strlen(text) > MAX_UI_STR_LENGTH)
	{
		Com_Printf("UI text for control %s, %s, was too long (max %d)\n", name, text, MAX_UI_STR_LENGTH);
		return false;
	}

	strcpy(ui_control_ptr->text, text);

	return true; 
}

qboolean UI_SetImage(char* ui_name, char* name, char* image_path)
{
	ui_control_t* ui_control_ptr = UI_GetControl(ui_name, name);

	if (ui_control_ptr == NULL)
	{
		Com_Printf("Tried to set NULL UI control image path %s to %s!\n", name, image_path);
		return false;
	}

	if (strlen(image_path) > MAX_UI_STR_LENGTH)
	{
		Com_Printf("UI image path for control %s, %s, was too long (max %d)\n", name, image_path, MAX_UI_STR_LENGTH);
		return false;
	}

	strcpy(ui_control_ptr->image_path, image_path);

	return true;
}

void UI_Clear(char* ui_name)
{
	ui_t* ui_ptr = UI_GetUI(ui_name);

	if (!ui_ptr) return; 		// message already printed

	// clear every control but not the ui's info
	// we clear every control, not just the ones that exist currently, in case more controls existed at some point (such as with Leaderboard UI)
	for (int ui_control_num = 0; ui_control_num < ui_ptr->num_controls; ui_control_num++)
	{
		ui_control_t* ui_control = &ui_ptr->controls[ui_control_num];
		memset(ui_control, 0x00, sizeof(ui_control_t));
	}

	// tell everyone there are no controls
	ui_ptr->num_controls = 0;
}

// Resets all UIs
void UI_Reset()
{
	for (int ui_num = 0; ui_num < num_uis; ui_num++)
	{
		ui_t* ui_ptr = &ui_list[ui_num];

		UI_SetEnabled(ui_ptr->name, false);
		UI_SetActive(ui_ptr->name, false);
	}
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
	char		time_str[128] = { 0 };
#ifdef PLAYTEST
	strftime(&time, 128, "Playtest Build v" ZOMBONO_VERSION " (%b %d %Y %H:%M:%S)", local_time);
#elif _DEBUG
	strftime(&time_str, 128, "Debug Build v" ZOMBONO_VERSION " (%b %d %Y %H:%M:%S)", local_time);
#endif

	int size_x = 0, size_y = 0;
	Text_GetSize(cl_system_font->string, &size_x, &size_y, time_str);
	// TODO: Text_GetSize
	Text_Draw(cl_system_font->string, viddef.width - size_x, 0, time_str);
	const char* prerelease_text = "^3Pre-release build!";
	Text_GetSize(cl_system_font->string, &size_x, &size_y, prerelease_text);
	Text_Draw(cl_system_font->string, viddef.width - size_x, 10, prerelease_text);

#endif

	for (int ui_num = 0; ui_num < num_uis; ui_num++)
	{
		// draw the current UI if enabled (*ACTIVE* means it's receiving input events0
		ui_t* current_ui = &ui_list[ui_num];

		if (current_ui->enabled)
		{
			for (int ui_control_num = 0; ui_control_num < current_ui->num_controls; ui_control_num++)
			{
				ui_control_t* current_ui_control = &current_ui->controls[ui_control_num];
				
				switch (current_ui_control->type)
				{
					case ui_control_text:
						UI_DrawText(current_ui_control);
						break;
					case ui_control_image:
						UI_DrawImage(current_ui_control);
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

void UI_DrawText(ui_control_t* text)
{
	// initialised to 0
	// if the font is not set use the system font
	if (strlen(text->font) == 0)
	{
		Text_Draw(cl_system_font->string, text->position_x, text->position_y, text->text);
	}
	else
	{
		Text_Draw(text->font, text->position_x, text->position_y, text->text);
	}
}

void UI_DrawImage(ui_control_t* image)
{
	re.DrawPic(image->position_x, image->position_y, image->image_path);
}

void UI_DrawSlider(ui_control_t* slider)
{
	Com_Printf("UI: Sliders aren't implemented yet!\n");
}

void UI_DrawCheckbox(ui_control_t* checkbox)
{
	Com_Printf("UI: Checkboxes aren't implemented yet!\n");
}

void UI_DrawBox(ui_control_t* box)
{
	re.DrawFill(box->position_x, box->position_y, box->size_x, box->size_y, box->color[0], box->color[1], box->color[2], box->color[3]);
}