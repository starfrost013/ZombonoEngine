#pragma once

#include "quakedef.h"

ui_t*	ui[MAX_UI_COUNT];	// Global Ui defines
ui_t*	current_ui;			// Pointer to current_UI inside ui
int		ui_count = 0;		// UI count

ui_t* UI_GetUI(char* name);
void UI_AddUI(ui_element_t element);

void UI_DrawButton(ui_element_t button);
void UI_DrawCheckbox(ui_element_t button);
void UI_DrawText(ui_element_t button);
void UI_DrawSlider(ui_element_t button);

void UI_Init(void)
{
	// Allocate all the memory at once (speed :D)
	*ui = Hunk_AllocName(sizeof(ui_t) * MAX_UI_COUNT + (sizeof(ui_element_t) * MAX_UI_COUNT * MAX_UI_ELEMENTS), "UI");

	if (!ui)
	{
		Sys_Error("Failed to allocate hunk space for UI!");
	}
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

void UI_AddElement(ui_element_t element)
{
	if (current_ui->element_count >= MAX_UI_ELEMENTS)
	{
		Con_Warning("Attempted to add UI element to UI with >= MAX_UI_ELEMENTS elements!");
		return;
	}

	current_ui->elements[current_ui->element_count] = element;
	current_ui->element_count++;
}

void UI_AddButton(char* on_click, char* texture, float size_x, float size_y, float position_x, float position_y)
{
	ui_element_t new_button;

	memset(&new_button, 0x00, sizeof(ui_element_t));

	// note: "none" can be used to not call a function on click
	// it should never be null, so that's a Sys_Error

	if (texture == NULL
		&& strlen(on_click) > 0)
	{
		Host_Error("UI_AddButton: texture was NULL!");
		return;
	}
	
	// suppress warning
	if (on_click != NULL
		&& strlen(on_click) > 0) strcpy(new_button.on_click, on_click);

	strcpy(new_button.texture, texture);

	new_button.size_x = size_x;
	new_button.size_y = size_y;
	new_button.position_x = position_x;
	new_button.position_y = position_y;
	new_button.type = ui_element_button;

	// load and cache the UI element's texture, if it exists
	if (new_button.texture != NULL
		&& strlen(new_button.texture) > 0) Draw_CachePic(new_button.texture);

	UI_AddElement(new_button);
}

void UI_AddText(char* on_click, char* text, float position_x, float position_y)
{
	ui_element_t new_text;

	memset(&new_text, 0x00, sizeof(ui_element_t));

	if (text == NULL)
	{
		Host_Error("UI_AddText: text was NULL!");
		return;
	}

	strcpy(new_text.text, text);

	if (on_click != NULL
		&& strlen(on_click) > 0) strcpy(new_text.on_click, on_click);

	//size_x ignored for text until the new font system is in
	new_text.size_x = 0;
	new_text.size_y = 0;
	new_text.position_x = position_x;
	new_text.position_y = position_y;
	new_text.type = ui_element_text;

	UI_AddElement(new_text);
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

					case ui_element_text:
						UI_DrawText(current_ui_element);
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

	if (button.texture == NULL
		&& strlen(button.texture) == 0) Host_Error("Tried to draw a button with no texture!");

	Draw_Pic(button.position_x, button.position_y, Draw_CachePic(button.texture));
}

void UI_DrawText(ui_element_t text)
{
	unsigned int length = strlen(text.text);

	int font_size = 8;		// Temporary until the new font engine is in (that supports font sizes)
	int line_count = 0;		// Number of lines (/n chars...) 

	for (int char_num = 0; char_num < length; char_num++)
	{
		char current_char = text.text[char_num];

		if (current_char == '\n') line_count++; // account for new lines

		Draw_Character(text.position_x + (font_size * char_num), text.position_y + (font_size * line_count), current_char); 		// draw the character in the right place
	}
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

				// on_click optional
				if (acquired_ui_element.on_click == NULL) continue;

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