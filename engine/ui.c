#pragma once

#include "quakedef.h"

ui_t*	ui[MAX_UI_COUNT];	// Contains all UIs
ui_t*	current_ui;			// Pointer to current_UI inside ui
int		ui_count = 0;		// UI count

ui_t* UI_GetUI(char* name);
void UI_AddElement(ui_element_t element);

void UI_DrawButton(ui_element_t* button);
void UI_DrawCheckbox(ui_element_t* button);
void UI_DrawText(ui_element_t* button);
void UI_DrawSlider(ui_element_t* button);

void UI_DefaultOnClickHandler(ui_element_t* button); // must point to global ui heap

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

	if (texture == NULL)
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

void UI_AddCheckbox(char* on_click, char* text, qboolean checked, float position_x, float position_y)
{
	ui_element_t new_checkbox;
	
	memset(&new_checkbox, 0x00, sizeof(ui_element_t));

	if (text == NULL)
	{
		Host_Error("UI_AddCheckbox: text was NULL!");
		return;
	}

	strcpy(new_checkbox.text, text);

	// temporary before font system
	int font_size = 8;

	new_checkbox.checked = checked;
	new_checkbox.size_x = font_size;
	new_checkbox.size_y = font_size;
	
	new_checkbox.position_x = position_x;
	new_checkbox.position_y = position_y;
	new_checkbox.type = ui_element_checkbox;

	UI_AddElement(new_checkbox);
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
	new_text.position_x = position_x;
	new_text.position_y = position_y;
	new_text.type = ui_element_text;

	UI_AddElement(new_text);
}

void UI_AddSlider(char* on_click, char* text, float min_value, float max_value, float size_x, float size_y, float position_x, float position_y)
{
	ui_element_t new_slider;
	
	memset(&new_slider, 0x00, sizeof(ui_element_t));

	if (text == NULL)
	{
		Host_Error("UI_AddSlider: text was NULL!");
		return;
	}

	if (on_click != NULL
		&& strlen(on_click) > 0) strcpy(new_slider.on_click, on_click);

	if (text != NULL
		&& strlen(text) > 0) strcpy(new_slider.text, text);

	new_slider.size_x = size_x;
	new_slider.size_y = size_y;
	new_slider.min_value = min_value;
	new_slider.max_value = max_value;

	if (new_slider.min_value < 0
		|| new_slider.min_value > new_slider.max_value
		|| new_slider.max_value < 0)
	{
		Host_Error("UI_AddSlider: min_value > max_value or min_value or max_value < 0");
		return;
	}

	new_slider.position_x = position_x;
	new_slider.position_y = position_y;
	new_slider.type = ui_element_slider;

	UI_AddElement(new_slider);
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
				ui_element_t* current_ui_element = &current_ui->elements[ui_element_num];

				switch (current_ui_element->type)
				{
					case ui_element_button:
						UI_DrawButton(current_ui_element);
						break;

					case ui_element_checkbox:
						UI_DrawCheckbox(current_ui_element);
						break;

					case ui_element_text:
						UI_DrawText(current_ui_element);
						break;

					case ui_element_slider:
						UI_DrawSlider(current_ui_element);
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

void UI_DrawButton(ui_element_t* button)
{
	// the button will be drawn here
	// while we already cached it, it will just return the precached pic upon calling it again so this is okay

	if (button->texture == NULL
		&& strlen(button->texture) == 0) Host_Error("Tried to draw a button with no texture!");

	Draw_Pic(button->position_x, button->position_y, Draw_CachePic(button->texture));
}

void UI_DrawCheckbox(ui_element_t* checkbox)
{
	// draw the text set with the checkbox
	UI_DrawText(checkbox);

	// count the size so that we can put the checkbox to the right of and in the middle of the text
	int num_lines = 0, num_chars = 0, highest_num_chars = 0;
	int font_size = 8;

	for (int char_num = 0; char_num < strlen(checkbox->text); char_num++)
	{
		num_chars++;

		// reset
		if (checkbox->text[char_num] == '\n')
		{
			num_lines++;
			if (num_chars > highest_num_chars) highest_num_chars = num_chars;
			num_chars = 0; 
		}
	}

	// final position
	 // draw 1 char after the text horizontally, in the middle of the text vertically
	int final_x = checkbox->position_x + (highest_num_chars * (font_size + 1));
	int final_y = checkbox->position_y + ((font_size * num_lines) / 2) - (font_size / 2);

	// draw the checkbox
	if (checkbox->checked)
	{
		Draw_Character(final_x, final_y, 139);
	}
	else
	{
		Draw_Character(final_x, final_y, 11);
	}

}

void UI_DrawText(ui_element_t* text)
{
	unsigned int length = strlen(text->text);

	int font_size = 8;		// Temporary until the new font engine is in (that supports font sizes)
	int line_count = 0;		// Number of lines (/n chars...) 
	int char_count = 0;		// Number of characters on the current line (for multiline)

	for (int char_num = 0; char_num < length; char_num++)
	{
		char current_char = text->text[char_num];
		char_count++;

		if (current_char == '\n')
		{
			line_count++; // account for new lines
			char_count = 0;// reset
		}

		Draw_Character(text->position_x + (font_size * char_count), text->position_y + (font_size * line_count), current_char); 		// draw the character in the right place
	}
}

void UI_DrawSlider(ui_element_t* slider)
{
	// draw left edge
	Draw_Character(slider->position_x, slider->position_y, 128);

	int font_size = 8;

	// draw main part
	for (int slider_position = slider->position_x; slider_position < slider->position_x + slider->size_x; slider_position += font_size)
	{
		Draw_Character(slider_position, slider->position_y, 129);
	}

	// draw right edge
	Draw_Character(slider->position_x + slider->size_x, slider->position_y, 130);

	// draw current position character
	Draw_Character(slider->position_x + ((float)slider->size_x * slider->value), slider->position_y, 131);
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
	for (int uiNum = 0; uiNum < ui_count; uiNum++)
	{
		ui_t* acquired_ui = ui[uiNum];

		if (acquired_ui != NULL
			&& acquired_ui->focused
			&& acquired_ui->visible)
		{
			for (int uiElementNum = 0; uiElementNum < acquired_ui->element_count; uiElementNum++)
			{
				// Because it changes values we need to operate on the actual UI element in the hunk
				ui_element_t* acquired_ui_element = &acquired_ui->elements[uiElementNum];

				/*
				// autoscale manages this
				float scale_factor = scr_menuscale.value;
				
				// could still be zero? make it not zero
				
				if (scale_factor == 0) scale_factor = 1;
				*/

				if (x >= acquired_ui_element->position_x
					&& x <= acquired_ui_element->position_x + acquired_ui_element->size_x
					&& y >= acquired_ui_element->position_y
					&& y <= acquired_ui_element->position_y + acquired_ui_element->size_y)
				{
					// on_click optional
					if (acquired_ui_element->on_click == NULL
						|| strlen(acquired_ui_element->on_click) == 0)
					{
						// if it's NULL or an empty string, call the default onclick handler code
						UI_DefaultOnClickHandler(acquired_ui_element);
						return;
					}

					// find QC OnClick
					dfunction_t* func = ED_FindFunction(acquired_ui_element->on_click);

					if (func == NULL)
					{
						Host_Error("UI_OnClick: Tried to call invalid QC event handler %s", acquired_ui_element->on_click);
						return;
					}

					// execute it
					PR_ExecuteProgram(func - pr_functions);
				}
			}
		}
	}
}

// Implements default onclick handlers for element types.
// Called in the case element.on_click is NULL or zero-length.
void UI_DefaultOnClickHandler(ui_element_t* element)
{
	switch (element->type)
	{
		case ui_element_button:
		case ui_element_text:
		case ui_element_slider:
			return;
		case ui_element_checkbox:
			element->checked = !element->checked;
			break;
	}
}

void UI_End(void)
{
	// current_ui is NULLPTR when no ui is set
	current_ui = NULL;
}