#include "defs.h"

/* 
=========== Z O M B O N O =========== 
      Copyright © 2023 starfrost
=====================================

modernui.qc : Modern UI qc defs
*/

void SUB_UIStart(char* name)
{
	// SELF must ALWAYS be current player.
	msg_entity = self; 

	// SELF must ALWAYS be current player before calling.
	if (msg_entity.classname != "player")
	{
		error("Tried to start a UI for a non-player entity!");
	}

	dprint("UIStart called\n");
	WriteByte(MSG_ONE, SVC_UI_START);
	WriteString(MSG_ONE, name);

	ui_last_entity = self;
}

void SUB_UIEnd()
{
	msg_entity = self;

	if (msg_entity.classname != "player")
	{
		error("Tried to end a UI for a non-player entity!");
	}

	dprint("UIEnd called\n");
	WriteByte(MSG_ONE, SVC_UI_END);

	ui_last_entity = self;
}

void SUB_UIAddButton(char* on_click, char* element_name, char* texture, float size_x, float size_y, float position_x, float position_y)
{
	// SELF must ALWAYS be current playerv
	msg_entity = ui_last_entity; 
	dprint("UIAddButton called\n");
	WriteByte(MSG_ONE, SVC_UI_ADD_BUTTON);
	WriteString(MSG_ONE, on_click);
	WriteString(MSG_ONE, element_name);
	WriteString(MSG_ONE, texture);
	WriteFloat(MSG_ONE, size_x);
	WriteFloat(MSG_ONE, size_y);
	WriteFloat(MSG_ONE, position_x);
	WriteFloat(MSG_ONE, position_y);
}


void SUB_UIAddCheckbox(char* on_click, char* element_name, char* text, float checked, float position_x, float position_y)
{
	// SELF must ALWAYS be current player before calling.
	msg_entity = ui_last_entity; 
	dprint("UIAddCheckbox called\n");
	WriteByte(MSG_ONE, SVC_UI_ADD_CHECKBOX);
	WriteString(MSG_ONE, on_click);
	WriteString(MSG_ONE, element_name);
	WriteString(MSG_ONE, text);
	WriteFloat(MSG_ONE, checked);
	WriteFloat(MSG_ONE, position_x);
	WriteFloat(MSG_ONE, position_y);
}

void SUB_UIAddSlider(char* on_click, char* element_name, char* text, float value_min, float value_max, float size_x, float size_y, float position_x, float position_y)
{
	// SELF must ALWAYS be current player before calling.
	msg_entity = ui_last_entity; 
	dprint("UIAddSlider called\n");
	WriteByte(MSG_ONE, SVC_UI_ADD_SLIDER);
	WriteString(MSG_ONE, on_click);
	WriteString(MSG_ONE, element_name);
	WriteString(MSG_ONE, text);
	WriteFloat(MSG_ONE, value_min);
	WriteFloat(MSG_ONE, value_max);
	WriteFloat(MSG_ONE, size_x);
	WriteFloat(MSG_ONE, size_y);
	WriteFloat(MSG_ONE, position_x);
	WriteFloat(MSG_ONE, position_y);
}

void SUB_UIAddText(char* on_click, char* element_name, char* text, float position_x, float position_y)
{
	// SELF must ALWAYS be current player before calling.
	msg_entity = ui_last_entity; 
	dprint("UIAddText called\n");
	WriteByte(MSG_ONE, SVC_UI_ADD_TEXT);
	WriteString(MSG_ONE, on_click);
	WriteString(MSG_ONE, element_name);
	WriteString(MSG_ONE, text);
	WriteFloat(MSG_ONE, position_x);
	WriteFloat(MSG_ONE, position_y);
}

void SUB_UISetVisibility(char* name, float visible)
{	
	// SELF must ALWAYS be current player before calling.
	msg_entity = ui_last_entity; 
	dprint("UISetVisibility called\n");
	WriteByte(MSG_ONE, SVC_UI_SET_VISIBILITY);
	WriteString(MSG_ONE, name);
	WriteFloat(MSG_ONE, visible);
}

void SUB_UISetFocus(char* name, float focused)
{
	// SELF must ALWAYS be current player before calling.
	msg_entity = ui_last_entity; 
	dprint("UISetFocus called\n");
	WriteByte(MSG_ONE, SVC_UI_SET_FOCUS);
	WriteString(MSG_ONE, name);
	WriteFloat(MSG_ONE, focused);
}

void SUB_UISetText(char* name, char* element_name, char* text)
{
    msg_entity = ui_last_entity;
	dprint(msg_entity.classname);
    dprint("UISetText called\n");
    WriteByte(MSG_ONE, SVC_UI_SET_TEXT);
    WriteString(MSG_ONE, name);
    WriteString(MSG_ONE, element_name);
    WriteString(MSG_ONE, text);
}