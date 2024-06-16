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
// In the future, all of these will be handled using scripts...

#include <client/client.h>

// Globals
// see client.h for explanations on what these are 
int32_t num_uis;																					
ui_t	ui_list[MAX_UIS];
bool	ui_active = false;																				// This is so we know to turn on the mouse cursor when a UI is being displayed.
bool	ui_initialised = false;

// Current UI. A non-null value is enforced.
// Editing functions apply to this UI, as well as functions that are run on UI elements.
// You can only access UI elements through the current UI.
ui_t*	current_ui;

bool UI_AddControl(ui_t* ui, char* name, float position_x, float position_y, int32_t size_x, int32_t size_y);// Shared function that adds controls.

// Draw methods
void UI_DrawText(ui_control_t* text);															// Draws a text control.
void UI_DrawImage(ui_control_t* image);															// Draws an image control.
void UI_DrawSlider(ui_control_t* slider);														// Draws a slider control.
void UI_DrawCheckbox(ui_control_t* checkbox);													// Draws a checkbox control.
void UI_DrawBox(ui_control_t* box);																// Draws a box control.
void UI_DrawSpinControl(ui_control_t* spin_control);											// Draws a spin control.
void UI_DrawEntry(ui_control_t* entry);															// Draws an entry control.

void UI_SliderDoSlide(ui_control_t* slider, int32_t dir);										// 'Slides' a slider control.
void UI_SpinControlDoEnter(ui_control_t* spin_control);											// Handles pressing ENTER on a spincontrol.
void UI_SpinControlDoSlide(ui_control_t* spin_control, int32_t dir);							// 'Slides' a spin control.
bool Entry_OnKeyDown(ui_control_t* entry, int32_t key);											// Handles a key being pressed on an entry control.


bool UI_Init()
{
	// init UI cvars
	ui_newmenu = Cvar_Get("ui_newmenu", "0", 0);

	// they are not statically initalised here because UI gets reinit'd on vidmode change so we need to wipe everything clean
	memset(&ui_list, 0x00, sizeof(ui_t) * num_uis); // only clear the uis that actually exist
	num_uis = 0;
	bool successful;

	Com_Printf("ZombonoUI is running UI creation scripts\n");
	successful = UI_AddUI("TeamUI", UI_TeamUICreate);
	if (successful) successful = UI_AddUI("LeaderboardUI", UI_LeaderboardUICreate);
	if (successful) successful = UI_AddUI("BamfuslicatorUI", UI_BamfuslicatorUICreate);
	if (successful) successful = UI_AddUI("TimeUI", UI_TimeUICreate);
	if (successful) successful = UI_AddUI("ScoreUI", UI_ScoreUICreate);
	if (successful) successful = UI_AddUI("LoadoutUI", UI_LoadoutUICreate);
	if (successful) successful = UI_AddUI("MainMenuUI", UI_MainMenuUICreate);
	if (successful) successful = UI_AddUI("MainMenuQuickstartUI", UI_MainMenuQuickstartUICreate);
	if (successful) successful = UI_AddUI("MainMenuBrowseServersUI", UI_MainMenuBrowseServersUICreate);
	if (successful) successful = UI_AddUI("MainMenuSettingsUI", UI_MainMenuSettingsUICreate);
	if (successful) successful = UI_AddUI("MainMenuZombieTVUI", UI_MainMenuZombieTVUICreate);
	if (successful) successful = UI_AddUI("MainMenuQuitUI", UI_MainMenuQuitUICreate);
	if (successful) successful = UI_AddUI("KillFeedUI", UI_KillFeedUICreate);
	ui_initialised = successful;
	return successful;
}

bool UI_AddUI(char* name, bool(*on_create)())
{
	Com_DPrintf("Creating UI: %s\n", name);
	current_ui = &ui_list[num_uis];

	if (num_uis > MAX_UIS)
	{
		Sys_Error("Tried to create a UI when there are more than %d UIs!", MAX_UIS);
		return false; 
	}

	num_uis++;

	if (strlen(name) > MAX_UI_STRLEN)
	{
		Sys_Error("Tried to create a UI with name more than %d characters!", MAX_UI_STRLEN);
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
	for (int32_t ui_num = 0; ui_num < num_uis; ui_num++)
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

	for (int32_t ui_control_num = 0; ui_control_num < ui_ptr->num_controls; ui_control_num++)
	{
		ui_control_t* ui_control_ptr = &ui_ptr->controls[ui_control_num];

		if (!stricmp(ui_control_ptr->name, name))
			return ui_control_ptr;

	}

	return NULL;
}

bool UI_AddControl(ui_t* ui_ptr, char* name, float position_x, float position_y, int32_t size_x, int32_t size_y)
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

bool UI_AddText(char* ui_name, char* name, char* text, float position_x, float position_y)
{
	ui_t* ui_ptr = UI_GetUI(ui_name);

	if (!ui_ptr)
		return false;

	ui_control_t* ui_control_ptr = &ui_ptr->controls[ui_ptr->num_controls];

	// not recommended to buffer overflow
	if (strlen(text) > MAX_UI_STRLEN)
	{
		Com_Printf("Tried to set UI control text %s to %s - too long (max length %d)!\n", name, text, MAX_UI_STRLEN);
		return false;
	}

	strcpy(ui_control_ptr->text, text);

	ui_control_ptr->type = ui_control_text;

	if (strlen(ui_control_ptr->font) == 0)
		Text_GetSize(cl_system_font->string, &ui_control_ptr->size_x, &ui_control_ptr->size_y, text);
	else
		Text_GetSize(ui_control_ptr->font, &ui_control_ptr->size_x, &ui_control_ptr->size_y, text);


	return UI_AddControl(ui_ptr, name, position_x, position_y, 0, 0);
}

bool UI_AddImage(char* ui_name, char* name, char* image_path, float position_x, float position_y, int32_t size_x, int32_t size_y)
{
	ui_t* ui_ptr = UI_GetUI(ui_name);

	if (!ui_ptr)
	{
		// message already printed
		return false;
	}

	ui_control_t* ui_control = &ui_ptr->controls[ui_ptr->num_controls];

	// not recommended to buffer overflow
	if (strlen(image_path) > MAX_UI_STRLEN)
	{
		Com_Printf("The UI control %s's image path %s is too long (max length %d)!\n", name, image_path, MAX_UI_STRLEN);
		return false;
	}

	strcpy(ui_control->image_path, image_path);

	ui_control->type = ui_control_image;

	return UI_AddControl(ui_ptr, name, position_x, position_y, size_x, size_y);
}

bool UI_AddSlider(char* ui_name, char* name, float position_x, float position_y, int32_t size_x, int32_t size_y, int32_t value_min, int32_t value_max)
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

bool UI_AddCheckbox(char* ui_name, char* name, float position_x, float position_y, int32_t size_x, int32_t size_y, bool checked)
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

bool UI_AddBox(char* ui_name, char* name, float position_x, float position_y, int32_t size_x, int32_t size_y, int32_t r, int32_t g, int32_t b, int32_t a)
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

bool UI_SetEnabled(char* ui_name, bool enabled)
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

bool UI_SetActivated(char* ui_name, bool activated)
{
	ui_t* ui_ptr = UI_GetUI(ui_name);

	if (ui_ptr != NULL)
	{
		ui_ptr->activated = activated;
		ui_active = activated;

		// if the UI requires the mouse....
		if (!ui_ptr->passive)
		{
			// turn on the mouse cursor, bit hacky - we basically take over the mouse pointer when a UI is active
			Input_Activate(!activated);
		}

		current_ui = ui_ptr;
		return true; 
	}

	return false;
}

// set UI ui_name passivity to passive
bool UI_SetPassive(char* ui_name, bool passive)
{
	ui_t* ui_ptr = UI_GetUI(ui_name);

	if (ui_ptr != NULL)
	{
		ui_ptr->passive = passive; 
		return true;
	}

	return false;
}


bool UI_SetText(char* ui_name, char* control_name, char* text)
{
	ui_control_t* ui_control_ptr = UI_GetControl(ui_name, control_name);

	if (ui_control_ptr == NULL)
	{
		Com_Printf("Couldn't find UI control %s to set text to %s!\n", control_name, text);
		return false;
	}

	if (strlen(text) > MAX_UI_STRLEN)
	{
		Com_Printf("UI text for control %s, %s, was too long (max %d)\n", control_name, text, MAX_UI_STRLEN);
		return false;
	}

	if (strlen(ui_control_ptr->font) == 0)
		Text_GetSize(cl_system_font->string, &ui_control_ptr->size_x, &ui_control_ptr->size_y, text);
	else
		Text_GetSize(ui_control_ptr->font, &ui_control_ptr->size_x, &ui_control_ptr->size_y, text);

	strcpy(ui_control_ptr->text, text);

	return true; 
}

bool UI_SetImage(char* ui_name, char* name, char* image_path)
{
	ui_control_t* ui_control_ptr = UI_GetControl(ui_name, name);

	if (ui_control_ptr == NULL)
	{
		Com_Printf("Tried to set NULL UI control image path %s to %s!\n", name, image_path);
		return false;
	}

	if (strlen(image_path) > MAX_UI_STRLEN)
	{
		Com_Printf("UI image path for control %s, %s, was too long (max %d)\n", name, image_path, MAX_UI_STRLEN);
		return false;
	}

	strcpy(ui_control_ptr->image_path, image_path);

	return true;
}

bool UI_SetInvisible(char* ui_name, char* control_name, bool invisible)
{
	ui_control_t* ui_control_ptr = UI_GetControl(ui_name, control_name);

	if (ui_control_ptr == NULL)
	{
		Com_Printf("Tried to set NULL UI control image on hover path %s to %b!\n", control_name, invisible);
		return false;
	}

	ui_control_ptr->invisible = invisible;

	return true;
}

bool UI_SetImageOnHover(char* ui_name, char* control_name, char* image_path)
{
	ui_control_t* ui_control_ptr = UI_GetControl(ui_name, control_name);

	if (ui_control_ptr == NULL)
	{
		Com_Printf("Tried to set NULL UI control image on hover path %s to %s!\n", control_name, image_path);
		return false;
	}

	if (strlen(image_path) > MAX_UI_STRLEN)
	{
		Com_Printf("UI image on hover path for control %s, %s, was too long (max %d)\n", control_name, image_path, MAX_UI_STRLEN);
		return false;
	}

	strcpy(ui_control_ptr->image_path_on_hover, image_path);

	return true;
}

bool UI_SetImageOnClick(char* ui_name, char* name, char* image_path)
{
	ui_control_t* ui_control_ptr = UI_GetControl(ui_name, name);

	if (ui_control_ptr == NULL)
	{
		Com_Printf("Tried to set NULL UI control image on click path %s to %s!\n", name, image_path);
		return false;
	}

	if (strlen(image_path) > MAX_UI_STRLEN)
	{
		Com_Printf("UI image on click path for control %s, %s, was too long (max %d)\n", name, image_path, MAX_UI_STRLEN);
		return false;
	}

	strcpy(ui_control_ptr->image_path_on_click, image_path);

	return true;
}

bool UI_SetImageIsStretched(char* ui_name, char* name, bool is_stretched)
{
	ui_control_t* ui_control_ptr = UI_GetControl(ui_name, name);

	if (ui_control_ptr == NULL)
	{
		Com_Printf("Tried to set NULL UI control image is stretched %s to %d!\n", name, is_stretched);
		return false;
	}
	
	ui_control_ptr->image_is_stretched = is_stretched;
	return true;
}

void UI_Clear(char* ui_name)
{
	ui_t* ui_ptr = UI_GetUI(ui_name);

	if (!ui_ptr) return; 		// message already printed

	// clear every control but not the ui's info
	// we clear every control, not just the ones that exist currently, in case more controls existed at some point (such as with Leaderboard UI)
	for (int32_t ui_control_num = 0; ui_control_num < ui_ptr->num_controls; ui_control_num++)
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
	for (int32_t ui_num = 0; ui_num < num_uis; ui_num++)
	{
		ui_t* ui_ptr = &ui_list[ui_num];

		UI_SetEnabled(ui_ptr->name, false);
		UI_SetActivated(ui_ptr->name, false);
	}
}

void UI_Draw()
{
// draw debug/playtest indicator
	
// this is NOT!! efficient don't do this (esp. getting the length of the string every frame and stuff) but not used in release 
#if defined(PLAYTEST) || !defined(NDEBUG)
	time_t		raw_time;
	struct tm*	local_time;

	time(&raw_time);
	local_time = localtime(&raw_time);
	char		time_str[128] = { 0 };
#ifdef PLAYTEST
	strftime(&time_str, 128, "Playtest Build v" ZOMBONO_VERSION " (%b %d %Y %H:%M:%S)", local_time);
#elif !defined(NDEBUG)
	strftime(&time_str, 128, "Debug Build v" ZOMBONO_VERSION " (%b %d %Y %H:%M:%S)", local_time);
#endif

	int32_t size_x = 0, size_y = 0;
	Text_GetSize(cl_system_font->string, &size_x, &size_y, time_str);
	// TODO: Text_GetSize
	Text_Draw(cl_system_font->string, gl_width->value - size_x, 0, time_str);
	const char* prerelease_text = "^3Pre-release build!";
	Text_GetSize(cl_system_font->string, &size_x, &size_y, prerelease_text);
	Text_Draw(cl_system_font->string, gl_width->value - size_x, 10 * vid_hudscale->value, prerelease_text);

#endif

	for (int32_t ui_num = 0; ui_num < num_uis; ui_num++)
	{
		// draw the current UI if enabled (*ACTIVE* means it's receiving input events0
		ui_t* current_ui = &ui_list[ui_num];

		if (current_ui->enabled)
		{
			for (int32_t ui_control_num = 0; ui_control_num < current_ui->num_controls; ui_control_num++)
			{
				ui_control_t* current_ui_control = &current_ui->controls[ui_control_num];

				if (!current_ui_control->invisible)
				{
					float final_pos_x = current_ui_control->position_x * gl_width->value;
					float final_pos_y = current_ui_control->position_y * gl_height->value;

					// toggle UI hover images if the mouse is within a UI
					current_ui_control->hovered =
						(last_mouse_pos_x >= final_pos_x
							&& last_mouse_pos_x <= (final_pos_x + (current_ui_control->size_x * vid_hudscale->value))
							&& last_mouse_pos_y >= final_pos_y
							&& last_mouse_pos_y <= (final_pos_y + (current_ui_control->size_y * vid_hudscale->value)));

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
					case ui_control_spin:
						UI_DrawSpinControl(current_ui_control);
						break;
					case ui_control_entry:
						UI_DrawEntry(current_ui_control);
						break;
					}
				}
			}
		}
	}
}

void UI_DrawText(ui_control_t* text)
{
	int32_t final_pos_x = text->position_x * gl_width->value;
	int32_t final_pos_y = text->position_y * gl_height->value;

	// initialised to 0
	// if the font is not set use the system font
	if (strlen(text->font) == 0)
	{
		Text_Draw(cl_system_font->string, final_pos_x, final_pos_y, text->text);
	}
	else
	{
		Text_Draw(text->font, final_pos_x, final_pos_y, text->text);
	}
}

void UI_DrawImage(ui_control_t* image)
{
	int32_t final_pos_x = image->position_x * gl_width->value;
	int32_t final_pos_y = image->position_y * gl_height->value;

	int32_t final_size_x = image->size_x * vid_hudscale->value;
	int32_t final_size_y = image->size_y * vid_hudscale->value;

	char* image_path = image->image_path;

	if (image->focused
		&& image->image_path_on_click != NULL
		&& strlen(image->image_path_on_click) > 0)
	{
		image_path = image->image_path_on_click;
	}

	if (image->hovered
		&& image->image_path_on_hover != NULL
		&& strlen(image->image_path_on_hover) > 0)
	{
		image_path = image->image_path_on_hover;
	}

	if (image->image_is_stretched)
	{
		re.DrawPicStretch(final_pos_x, final_pos_y, final_size_x, final_size_y, image_path, NULL);
	}
	else
	{
		re.DrawPic(final_pos_x, final_pos_y, image_path, NULL);
	}

}

#define SLIDER_RANGE 10
#define RCOLUMN_OFFSET  16 * vid_hudscale->value
#define LCOLUMN_OFFSET -16 * vid_hudscale->value

void UI_SliderDoSlide(ui_control_t* slider, int32_t dir)
{
	slider->value_current += dir;

	if (slider->value_current > slider->value_max)
		slider->value_current = slider->value_max;
	else if (slider->value_current < slider->value_min)
		slider->value_current = slider->value_min;

}

void UI_DrawSlider(ui_control_t* slider)
{
	int32_t i;
	int32_t size_x = 0, size_y = 0; // emulate original code R2L drawing

	Text_GetSize(cl_system_font->string, &size_x, &size_y, slider->text);
	Text_Draw(cl_system_font->string, slider->position_x * gl_width->value  + LCOLUMN_OFFSET - size_x,
		slider->position_y * gl_height->value,
		slider->text);

	float range = (slider->value_current - slider->value_min) / (float)(slider->value_max - slider->value_min);

	if (range < 0)
		range = 0;
	if (range > 1)
		range = 1;

	re.DrawPic(slider->position_x * gl_width->value  + RCOLUMN_OFFSET, slider->position_y * gl_height->value, "2d/slider_01", NULL);
	
	for (i = 0; i < SLIDER_RANGE; i++)
		re.DrawPic(RCOLUMN_OFFSET + slider->position_x * gl_width->value + i * 8 * vid_hudscale->value + 8 * vid_hudscale->value, slider->position_y * gl_height->value, "2d/slider_02", NULL);
	
	re.DrawPic(RCOLUMN_OFFSET + slider->position_x * gl_width->value + i * 8 * vid_hudscale->value + 8 * vid_hudscale->value, slider->position_y * gl_height->value, "2d/slider_03", NULL);
	
	re.DrawPic((int32_t)(8 * vid_hudscale->value + RCOLUMN_OFFSET  + slider->position_x * gl_width->value + (SLIDER_RANGE - 1) * 8 * vid_hudscale->value * range), slider->position_y * gl_height->value, "2d/slider_value", NULL);
}

void UI_DrawCheckbox(ui_control_t* checkbox)
{
	Com_Printf("UI: Checkboxes aren't implemented yet!\n");
}

void UI_DrawBox(ui_control_t* box)
{
	int32_t final_pos_x = box->position_x * gl_width->value;
	int32_t final_pos_y = box->position_y * gl_height->value;

	int32_t final_size_x = box->size_x * vid_hudscale->value;
	int32_t final_size_y = box->size_y * vid_hudscale->value;

	re.DrawFill(final_pos_x, final_pos_y, final_size_x, final_size_y, box->color);
}

void UI_SpinControlDoEnter(ui_control_t* spin_control)
{
	spin_control->value_current++;
	if (spin_control->item_names[(int32_t)spin_control->value_current] == 0)
		spin_control->value_current = 0;
}

void UI_SpinControlDoSlide(ui_control_t* spin_control, int32_t dir)
{
	spin_control->value_current += dir;

	if (spin_control->value_current < 0)
		spin_control->value_current = 0;
	else if (spin_control->item_names[(int32_t)spin_control->value_current] == 0)
		spin_control->value_current--;
}

void UI_DrawSpinControl(ui_control_t* spin_control)
{
	int32_t size_x = 0, size_y = 0;

	if (spin_control->text)
	{
		Text_GetSize(cl_system_font->string, &size_x, &size_y, spin_control->text);
		Text_Draw(cl_system_font->string, spin_control->position_x * gl_width->value + LCOLUMN_OFFSET - size_x,
			spin_control->position_y,
			spin_control->text);
	}

	Text_Draw(cl_system_font->string, RCOLUMN_OFFSET + spin_control->position_x * gl_width->value, spin_control->position_y, spin_control->item_names[(int32_t)spin_control->value_current]);
}

bool Entry_OnKeyDown(ui_control_t* entry, int32_t key)
{
	//TODO: THIS CODE SUCKS
	//WHY DO WE DUPLICATE EVERYTHING ALREADY HANDLED IN INPUT SYSTEM????

	extern int32_t keydown[];

	// support pasting from the clipboard
	if ((keydown[K_V] && keydown[K_CTRL]) ||
		(((key == K_INSERT)) && keydown[K_SHIFT]))
	{
		char* cbd;

		if ((cbd = Sys_GetClipboardData()) != 0)
		{
			strncpy(entry->entry_text_buffer, cbd, strlen(entry->entry_text_buffer) - 1);

			entry->cursor_position = (int32_t)strlen(entry->entry_text_buffer);
			entry->cursor_last_visible = entry->cursor_position - entry->cursor_last_visible;
			if (entry->cursor_last_visible < 0)
				entry->cursor_last_visible = 0;

			free(cbd);
		}
		return true;
	}

	switch (key)
	{
	case K_LEFTARROW:
	case K_BACKSPACE:
		if (entry->cursor_position > 0)
		{
			memmove(&entry->entry_text_buffer[entry->cursor_position - 1], &entry->entry_text_buffer[entry->cursor_position], strlen(&entry->entry_text_buffer[entry->cursor_position]) + 1);
			entry->cursor_position--;

			if (entry->cursor_last_visible)
			{
				entry->cursor_last_visible--;
			}
		}
		break;

	case K_DELETE:
		memmove(&entry->entry_text_buffer[entry->cursor_position], &entry->entry_text_buffer[entry->cursor_position + 1], strlen(&entry->entry_text_buffer[entry->cursor_position + 1]) + 1);
		break;

	case K_KP_ENTER:
	case K_ENTER:
	case K_ESCAPE:
	case K_TAB:
		return false;

	case K_SPACE:
	default:
		char processed_key = Key_VirtualToPhysical(key, (keydown[K_SHIFT])
			|| (keydown[K_CAPS_LOCK])
			&& key >= K_A
			&& key <= K_Z)[0];

		//if (!isdigit(key) && (f->generic.flags & QMF_NUMBERSONLY))
			//return false;

		if (entry->cursor_position < strlen(entry->entry_text_buffer))
		{
			entry->entry_text_buffer[entry->cursor_position++] = processed_key;
			entry->entry_text_buffer[entry->cursor_position] = 0;

			if (entry->cursor_position > entry->cursor_last_visible)
			{
				entry->cursor_last_visible++;
			}
		}
	}

	return true;
}

//TODO: Draw cursor
void UI_DrawEntry(ui_control_t* entry)
{
	int32_t i;
	int32_t name_size_x = 0, name_size_y = 0;
	int32_t text_size_x = 0, text_size_y = 0;
	char tempbuffer[128] = "";

	if (entry->text)
	{
		Text_GetSize(cl_system_font->string, &name_size_x, &name_size_y, entry->text);
		Text_Draw(cl_system_font->string, entry->position_x * gl_width->value + LCOLUMN_OFFSET - name_size_x, entry->position_y * gl_height->value, entry->text);
	}

	strncpy(tempbuffer, entry->entry_text_buffer + entry->cursor_last_visible, entry->cursor_last_visible);

	re.DrawPic(entry->position_x * gl_width->value + 16 * vid_hudscale->value, entry->position_y * gl_height->value - 4 * vid_hudscale->value, "2d/field_top_01", NULL);
	re.DrawPic(entry->position_x * gl_width->value + 16 * vid_hudscale->value, entry->position_y * gl_height->value + 4 * vid_hudscale->value, "2d/field_bottom_01", NULL);

	re.DrawPic(entry->position_x * gl_width->value + 24 * vid_hudscale->value + entry->cursor_last_visible * 8 * vid_hudscale->value, entry->position_y * gl_height->value - 4 * vid_hudscale->value, "2d/field_top_03", NULL);
	re.DrawPic(entry->position_x * gl_width->value + 24 * vid_hudscale->value + entry->cursor_last_visible * 8 * vid_hudscale->value, entry->position_y * gl_height->value + 4 * vid_hudscale->value, "2d/field_bottom_03", NULL);

	for (i = 0; i < entry->cursor_last_visible; i++)
	{
		re.DrawPic(entry->position_x * gl_width->value + 24 * vid_hudscale->value + i * 8 * vid_hudscale->value, entry->position_y * gl_height->value - 4 * vid_hudscale->value, "2d/field_top_02", NULL);
		re.DrawPic(entry->position_x * gl_width->value + 24 * vid_hudscale->value + i * 8 * vid_hudscale->value, entry->position_y * gl_height->value + 4 * vid_hudscale->value, "2d/field_bottom_02", NULL);
	}

	Text_GetSize(cl_system_font->string, &text_size_x, &text_size_y, tempbuffer);
	Text_Draw(cl_system_font->string, entry->position_x * gl_width->value + 24 * vid_hudscale->value, (entry->position_y * gl_height->value) - 1, tempbuffer); // -1 for padding

	int32_t offset;

	if (entry->cursor_last_visible)
		offset = entry->cursor_last_visible;
	else
		offset = entry->cursor_position;

	if (((int32_t)(Sys_Milliseconds() / 250)) & 1)
	{
		// 8x8 is cursor size
		re.DrawPic(entry->position_x * gl_width->value + (offset + 2) + (text_size_x + 8) * vid_hudscale->value,
			entry->position_y * gl_height->value,
			"2d/field_cursor_on", NULL);
	}
}