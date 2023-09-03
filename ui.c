#pragma once

#include "quakedef.h"

ui_t*	ui[MAX_UI_COUNT];	// Global Ui defines
ui_t*	current_ui;			// Pointer to current_UI 
int		ui_count = 0;		// UI count

ui_t* UI_GetUI(char* name);

void UI_Init(void)
{
	// Allocate all the memory at once (speed :D)
	*ui = Hunk_AllocName(sizeof(ui_t) * MAX_UI_COUNT, "UI");

	if (!ui)
	{
		Sys_Error("Failed to allocate hunk space for UI!");
	}

}

ui_t* UI_GetUI(char* name)
{
	for (int ui_number = 0; ui_number < MAX_UI_COUNT; ui_number++)
	{
		if (!strcmp(ui[ui_number]->name, name)) return ui[ui_number];
	}

	return NULL;
}

void UI_Start(char* name)
{
	ui_t* new_ui = UI_GetUI(name);

	if (new_ui) // we already created this ui so don't bother
	{
		current_ui = new_ui;
		return; 
	}
	else
	{
		memset(&new_ui, 0x00, sizeof(ui_t));
		*new_ui->name = name;

		*ui[ui_count] = *new_ui;
		ui_count++;
	}
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