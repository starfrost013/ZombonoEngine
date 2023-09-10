#pragma once

/*
	Zombono
	Copyright © 2023 starfrost

	New UI System

	Allows QuakeC (running on the server) or the client to generate an immediate-mode UI. This is a more flexible solution
	than the existing hardcoded UI solution, and is secure and managed by the server (although the client can also use it for things like menus)
*/

#define MAX_UI_COUNT			48		// 48 total UIs
#define MAX_UI_ELEMENTS			64		// 64 elements per UI	

// UI element types.
typedef enum ui_element_type_e
{
	ui_element_button,
	ui_element_checkbox,
	ui_element_text,
	ui_element_slider,
} ui_element_type;

// UI element defines; more will be added to this class as more controls get added.
typedef struct ui_element_s
{
	ui_element_type		type;						// Type of UI element to draw.
	char				texture[64];				// Image to draw for the element.
	float				size_x;						// Size of the element on screen (X).
	float				size_y;						// Size of the element on screen (Y).
	float				position_x;					// Position of the element on the screen (X).
	float				position_y;					// Position of the element on the screen (X).
	char				on_click[32];				// QuakeC function to call on click.
	char				text[64];					// The text of this UI element.
	qboolean			checked;					// Determines if the UI element is checked
	float				value;						// Holds the value of the UI element.
} ui_element_t;

// UI define itself. Has a name for caching purposes.
typedef struct ui_s
{
	char				name[16];					// Name.
	ui_element_t		elements[MAX_UI_ELEMENTS];	// List of elements.
	int					element_count;				// Number of elements.
	qboolean			visible;					// Is the UI visible?
	qboolean			focused;					// Is the UI focused?
} ui_t;

// Global UI list.
extern ui_t*				ui[];

void UI_Init(void);										// Initialise UI subsystem
void UI_Start(char* name);								// Start a new UI
void UI_Draw(void);										// Draw all UIs
void UI_SetVisibility(char* name, qboolean visibility);	// Set UI visibility
void UI_SetFocus(char* name, qboolean focus);			// Set UI focus
void UI_OnClick(float x, float y);						// UI click event. Not for QC
void UI_End();								// End a UI

//
// Controls
//
// these end up being copied to a nonconst buf to get around the limitation of MSG_WriteString() using a globally shared buffer. (TODO: fix these warnings with MSG_WriteStrings)
void UI_AddButton(char* on_click, char* texture, float size_x, float size_y, float position_x, float position_y);					// Add a new button
void UI_AddCheckbox(char* on_click, char* text, qboolean checked, float size_x, float size_y, float position_x, float position_y);	// Add a new checkbox
void UI_AddText(char* on_click, char* text, float size_x, float size_y, float position_x, float position_y);						// Add a new text block
void UI_AddSlider(char* on_click, char* text, float value, float size_x, float size_y, float position_x, float position_y);			// Add a new slider