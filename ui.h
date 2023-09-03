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

} ui_element_type;

// UI element defines; more will be added to this class as more controls get added.
typedef struct ui_element_s
{
	ui_element_type		type;
	char				image[64];
	int					size;
	int					position;
} ui_element_t;

// UI define itself. Has a name for caching purposes.
typedef struct ui_s
{
	char				name[16];
	ui_element_t		elements[MAX_UI_ELEMENTS];

} ui_t;

// Global UI list.
extern ui_t*				ui[];

void UI_Init(void);			// Initialise UI subsystem
void UI_Start(char* name);	// Start a new UI
void UI_Draw(void);			// Draw all UIs
void UI_End(char* names);	// End a UI
void UI_AddButton(void);	// Add a new  button