/*
Copyright (C) 2023-2024 starfrost

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// ui_killfeed.c - Killfeed UI (August 6, 2024)
// In the future, all of these will be handled using scripts...

#include <client/client.h>

killfeed_entry_t killfeed_entries[MAX_KILLFEED_ENTRIES] = { 0 }; // initialise to zero
int32_t killfeed_entry_count = 0;

cvar_t* killfeed_entry_display_time;

bool UI_KillFeedUICreate()
{
	// this doesn't do anything other than set the update event
	UI_SetEventOnUpdate("KillFeedUI", UI_KillFeedUpdate);
	return true;
}

#define TEMP_NAME_BUF_SIZE	32		// Size of the name buffer for the temporary UI elements
#define KILLFEEDUI_NAME_BOX "KillFeedUI_KillFeedEntry%dBox"
#define KILLFEEDUI_NAME_VICTIM "KillFeedUI_KillFeedEntry%dText"
#define KILLFEEDUI_NAME_ICON "KillFeedUI_KillFeedEntry%dIcon"

void UI_KillFeedAdd()
{
	char* msg_string;
	char name_buf[TEMP_NAME_BUF_SIZE] = { 0 };

	// Overwrite the oldest entry if we have reached the maximum number of entries.
	// The draw code separate
	if (killfeed_entry_count >= MAX_KILLFEED_ENTRIES)
		killfeed_entry_count = 0;

	killfeed_entry_t* killfeed_entry = &(killfeed_entries[killfeed_entry_count]);

	// get the kill type	

	killfeed_entry->inuse = true;
	killfeed_entry->time = Sys_Milliseconds();

	msg_string = MSG_ReadString(&net_message);
	strncpy(killfeed_entry->death_string, msg_string, MAX_DEATHSTRING_LENGTH);
	strncpy(killfeed_entry->icon_path, msg_string, MAX_QPATH);

	// use a bitflag here for slightly better structured code

	// add the ui, we need to come up with a unique string identifier for the UI 
	// this is required to remove it later


	//when we make UI_AddBox be consistent with the *SetColor UI functions a color4_t
	//color4_t killfeed_colour = { 255, 172, 117, 180 };

	// the positions get moved around later so just put them offscreen. the UI elements will literally last less than a frame in their initial positions
	// (see UI_KillFeedUpdate below)
	snprintf(name_buf, MAX_KILLFEED_ENTRIES, KILLFEEDUI_NAME_BOX, killfeed_entry_count);

	UI_AddBox("KillFeedUI", name_buf, -0.1f, -0.1f, 128, 32, 255, 172, 117, 180);
	memset(name_buf, 0x00, sizeof(name_buf));

	snprintf(name_buf, MAX_KILLFEED_ENTRIES, "KillFeedUI_KillFeedEntry%dText", killfeed_entry_count);
	UI_AddText("KillFeedUI", name_buf, killfeed_entry->death_string, -0.1f, -0.1f);
	memset(name_buf, 0x00, sizeof(name_buf));
	// Icon not implemented yet

	killfeed_entry_count++;
}

void UI_KillFeedUpdate()
{

}

void UI_KillFeedDelete()
{

}