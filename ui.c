#pragma once

#include "quakedef.h"

ui_t*	ui[MAX_UI_COUNT];	// Global Ui defines
ui_t*	current_ui;			// Pointer to current_UI inside ui
int		ui_count = 0;		// UI count

ui_t* UI_GetUI(char* name);

void UI_Init(void)
{
	// Allocate all the memory at once (speed :D)
	*ui = Hunk_AllocName(sizeof(ui_t) * MAX_UI_COUNT + (sizeof(ui_element_t) * MAX_UI_COUNT * MAX_UI_ELEMENTS), "UI");

	if (!ui)
	{
		Sys_Error("Failed to allocate hunk space for UI!");
	}
}

ui_t* UI_GetUI(char* name)
{
	for (int ui_number = 0; ui_number < MAX_UI_COUNT; ui_number++)
	{
		// You can only ever add uis, not delete them (although you can make them not be visible)
		// so this is a safe assumption to make
		if (ui[ui_number] == NULL)
			break;

		if (!Q_strcmp(ui[ui_number]->name, name)) return ui[ui_number];
	}

	return NULL;
}

void UI_Start(char* name)
{
	// check if the UI already exists
	ui_t* cached_ui = UI_GetUI(name);

	if (cached_ui) // we already created this ui so don't bother
	{
		current_ui = cached_ui;
		return; 
	}
	else
	{
		if (ui_count >= MAX_UI_ELEMENTS)
		{
			Con_Warning("Attempted to add a new UI when there are >= MAX_UI_COUNT UIs!");
			return;
		}

		// create a new UI

		ui_t* new_ui = ui[ui_count];
		
		strcpy(new_ui->name, name);

		// put it in its proper place
		memcpy(&ui[ui_count], &new_ui, sizeof(ui_t));
		ui_count++;

		current_ui = &ui[ui_count];
	}
}

void UI_AddButton(const char* on_click, const char* texture, float size_x, float size_y, float position_x, float position_y)
{
	ui_element_t new_button;

	memset(&new_button, 0x00, sizeof(ui_element_t));

	// note: "none" can be used to not call a function on click
	// it should never be null, so that's a Sys_Error

	if (on_click == NULL) Sys_Error("UI_AddButton: on_click was NULL!");
	if (texture == NULL) Sys_Error("UI_AddButton: texture was NULL!");
	
	// suppress warning
	if (on_click != NULL) strcpy(new_button.on_click, on_click);
	if (texture != NULL) strcpy(new_button.texture, texture);

	new_button.size_x = size_x;
	new_button.size_y = size_y;
	new_button.position_x = position_x;
	new_button.position_y = position_y;

	if (current_ui->element_count >= MAX_UI_ELEMENTS)
	{
		Con_Warning("Attempted to add UI element to UI with >= MAX_UI_ELEMENTS elements!");
		return;
	}

	current_ui->elements[current_ui->element_count] = new_button;
	current_ui->element_count++;
}

void UI_End(char* name)
{
	if (!UI_GetUI(name))
	{
		Host_Error("Tried to end a UI that does not exist!");
		return;
	}

	// current_ui is NULLPTR when no ui is set
	current_ui = NULL;
}