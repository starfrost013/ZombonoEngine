#pragma once

#include "quakedef.h"

ui_t*	ui[MAX_UI_COUNT];	// Global Ui defines
ui_t*	current_ui;			// Pointer to current_UI inside ui
int		ui_count = 0;		// UI count

ui_t* UI_GetUI(char* name);

void UI_DrawButton(ui_element_t button);

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

		if (!strncmp(ui[ui_number]->name, name, 16)) return ui[ui_number];
	}

	//no error here because in some cases we want prog errors, othercases Sys_Error
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

		ui_count++;

		current_ui = new_ui;
	}
}

void UI_AddButton(char* on_click, char* texture, float size_x, float size_y, float position_x, float position_y)
{
	ui_element_t new_button;

	memset(&new_button, 0x00, sizeof(ui_element_t));

	// note: "none" can be used to not call a function on click
	// it should never be null, so that's a Sys_Error

	if (on_click == NULL) Host_Error("UI_AddButton: on_click was NULL!");
	if (texture == NULL) Host_Error("UI_AddButton: texture was NULL!");
	
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

void UI_SetVisibility(char* name, qboolean visible)
{
	ui_t* current_ui = UI_GetUI(name);

	if (!current_ui)
	{
		Host_Error("Attempted to set visibility of invalid UI %s!", name);
		return;
	}

	current_ui->visible = visible;
}

void UI_Draw(void)
{
	GL_SetCanvas(CANVAS_DEFAULT);

	for (int ui_num = 0; ui_num < MAX_UI_COUNT; ui_num++)
	{
		ui_t* current_ui = ui[ui_num];

		if (current_ui != NULL
			&& current_ui->visible)
		{
			for (int ui_element_num = 0; ui_element_num < MAX_UI_COUNT; ui_element_num++)
			{
				ui_element_t current_ui_element = current_ui->elements[ui_element_num];

				switch (current_ui_element.type)
				{
					case ui_element_button:
						UI_DrawButton(current_ui_element);
						break;
				}
			}
		}
	}
}

void UI_DrawButton(ui_element_t button)
{
	// the button will be drawn here
}

void UI_End(char* name)
{
	if (!UI_GetUI(name))
	{
		Host_Error("Tried to end a UI %s that does not exist!", name);
		return;
	}

	// current_ui is NULLPTR when no ui is set
	current_ui = NULL;
}