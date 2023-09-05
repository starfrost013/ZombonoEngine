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

		if (!stricmp(ui[ui_number]->name, name)) return ui[ui_number];
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

	// load and cache the pic
	Draw_CachePic(new_button.texture);

	current_ui->elements[current_ui->element_count] = new_button;
	current_ui->element_count++;
}

void UI_SetVisibility(char* name, qboolean visible)
{
	ui_t* acquired_ui = UI_GetUI(name);

	if (!acquired_ui)
	{
		Host_Error("Attempted to set visibility of invalid UI %s!", name);
		return;
	}

	acquired_ui->visible = visible;
}

void UI_Draw(void)
{
	GL_SetCanvas(CANVAS_DEFAULT);

	int		focused_uis = 0;

	for (int ui_num = 0; ui_num < MAX_UI_COUNT; ui_num++)
	{
		ui_t* current_ui = ui[ui_num];

		if (current_ui != NULL
			&& current_ui->visible)
		{
			for (int ui_element_num = 0; ui_element_num < current_ui->element_count; ui_element_num++)
			{
				ui_element_t current_ui_element = current_ui->elements[ui_element_num];

				switch (current_ui_element.type)
				{
					case ui_element_button:
						UI_DrawButton(current_ui_element);
						break;
				}
			}

			// update focus
			if (current_ui->focused)
			{
				// turn on the mouse so the user can click around and stuff
				focused_uis++;
				IN_DeactivateMouse();
				IN_ShowMouse();
			}
		}
	}

	if (focused_uis == 0)
	{
		// turn the mouse back on again
		IN_ActivateMouse();
		IN_HideMouse();
	}

	if (focused_uis > 1)
		Host_Error("Multiple UIs focused!");
}

void UI_DrawButton(ui_element_t button)
{
	// the button will be drawn here
	// while we already cached it, it will just return the precached pic upon calling it again so this is okay

	Draw_Pic(button.position_x, button.position_y, Draw_CachePic(button.texture));
}

void UI_SetFocus(char* name, qboolean focus)
{
	ui_t* acquired_ui = UI_GetUI(name);

	if (!acquired_ui)
	{
		Host_Error("Tried to set the focus of a UI %s that does not exist!", name);
		return;
	}

	acquired_ui->focused = focus;
}

void UI_OnClick(float x, float y)
{
	for (int uiNum = 0; uiNum < MAX_UI_COUNT; uiNum++)
	{
		ui_t* acquired_ui = ui[uiNum];

		if (acquired_ui != NULL
			&& acquired_ui->focused
			&& acquired_ui->visible)
		{
			for (int uiElementNum = 0; uiElementNum < acquired_ui->element_count; uiElementNum++)
			{
				ui_element_t acquired_ui_element = acquired_ui->elements[uiElementNum];

				// autoscale manages this
				float scale_factor = scr_menuscale.value;
				
				// could still be zero? make it not zero
				if (scale_factor == 0) scale_factor = 1;

				if (x >= acquired_ui_element.position_x
					&& x <= acquired_ui_element.position_x + (acquired_ui_element.size_x * scale_factor)
					&& y >= acquired_ui_element.position_y
					&& y <= acquired_ui_element.position_y + (acquired_ui_element.size_y * scale_factor))
				{
					// find QC OnClick
					dfunction_t* func = ED_FindFunction(acquired_ui_element.on_click);

					// execute it
					PR_ExecuteProgram(func - pr_functions);
				}
			}
		}
	}
}

void UI_End(void)
{
	// current_ui is NULLPTR when no ui is set
	current_ui = NULL;
}