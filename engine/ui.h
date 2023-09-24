#pragma once

/*
	Zombono
	Copyright © 2023 starfrost

	New UI System

	Allows QuakeC (running on the server) or the client to generate an immediate-mode UI. This is a more flexible solution
	than the existing hardcoded UI solution, and is secure and managed by the server (although the client can also use it for things like menus)
*/

#define MAX_UI_COUNT			48								// 48 total UIs
#define MAX_UI_ELEMENTS			64								// 64 elements per UI	

// UI element types.
typedef enum ui_element_type_e
{
	ui_element_button,
	ui_element_checkbox,
	ui_element_slider,
	ui_element_text,
} ui_element_type;

// Enumerates UI event types.
typedef enum ui_event_type_e
{
	ui_event_click_down,
	ui_event_click_up,
	ui_event_mouse_move,
} ui_event_type;

// Defines a UI event.
typedef struct ui_event_s
{
	ui_event_type		type;
	void				(*c_handler)(void);						// C function to call on click. Overrides QC function if present.
	char				qc_handler[32];							// QC function to call on click.
	float				x;										// X coordinate of event.
	float				y;										// Y coordinate of event.
} ui_event_t;

// UI element defines; more will be added to this class as more controls get added.
typedef struct ui_element_s
{
	ui_element_type		type;									// Type of UI element to draw.
	char				name[32];								// The name of this UI element.
	char				texture[MAX_QPATH];						// Image to draw for the element.
	float				size_x;									// Size of the element on screen (X).
	float				size_y;									// Size of the element on screen (Y).
	float				position_x;								// Position of the element on the screen (X).
	float				position_y;								// Position of the element on the screen (X).
	ui_event_t			on_click_down;							// Event for on click down
	ui_event_t			on_click_up;							// Event for on click up
	ui_event_t			on_mouse_move;							// Event for mouse move.
	char				text[64];								// The text of this UI element.
	qboolean			checked;								// Determines if the UI element is checked
	float				value;									// Holds the value of the UI element. Goes from 0 to 1 - convert by using (min_value) + (max_value - min_value) * value
	float				min_value;								// The minimum value of this UI element.
	float				max_value;								// The maximum value of this UI element.
	qboolean			mouse_down;								// Determines if the mouse is currently held down on this element. Set to true when the element receives a ClickDown event,
																// false when it receives a ClickUp event.
} ui_element_t;

// UI define itself. Has a name for caching purposes.
typedef struct ui_s
{
	char				name[16];								// Name.
	ui_element_t		elements[MAX_UI_ELEMENTS];				// List of elements.
	int					element_count;							// Number of elements.
	qboolean			visible;								// Is the UI visible?
	qboolean			focused;								// Is the UI focused?
} ui_t;

extern ui_t*				ui;									// Global UI list.

void UI_Init(void);												// Initialise UI subsystem
void UI_Start(char* name);										// Start a new UI
void UI_Draw(void);												// Draw all UIs
void UI_SetVisibility(char* name, qboolean visibility);			// Set UI visibility
void UI_SetFocus(char* name, qboolean focus);					// Set UI focus
void UI_SetText(char* ui_name, char* element_name, char* text);	// Set UI text
void UI_OnClickDown(float x, float y);							// UI click down event. Not for QC
void UI_OnClickUp(float x, float y);							// UI click up event.
void UI_OnMouseMove(float x, float y);							// UI mouse move event.
void UI_End(void);												// End the current UI
void UI_Clear(void);											// Wipe UI on disconnect

//
// Controls
//
// these end up being copied to a nonconst buf to get around the limitation of MSG_WriteString() using a globally shared buffer.
// also the reason these are parameters is to simplify code when passing from QC
void UI_AddButton(char* on_click, char* element_name, char* texture, float size_x, float size_y, float position_x, float position_y);								// Add a new button
void UI_AddCheckbox(char* on_click, char* element_name, char* text, qboolean checked, float position_x, float position_y);											// Add a new checkbox
void UI_AddSlider(char* on_click, char* element_name, char* text, float value_min, float value_max, float size_x, float size_y, float position_x, float position_y);// Add a new slider
void UI_AddText(char* on_click, char* element_name, char* text, float position_x, float position_y);																// Add a new text block