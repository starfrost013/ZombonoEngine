/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2018-2019 Krzysztof Kondrak
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
#include "client.h"

// cl_loadout.c: Clientside loadout system (April 12, 2024)

void Loadout_UpdateUI();

void Loadout_Add()
{
	if (cl.loadout.num_items >= LOADOUT_MAX_ITEMS)
	{
		Com_Printf("ERROR: Tried to create loadout item %d, max %d...", LOADOUT_MAX_ITEMS);
		return;
	}

	// we can guarantee msg_readstring won't be called again while this function runs
	char* str = MSG_ReadString(&net_message);

	// see if we already have it
	for (int32_t item_num = 0; item_num < cl.loadout.num_items; item_num++)
	{
		loadout_entry_t current_loadout_entry = cl.loadout.items[item_num];

		// if we already have it...don't bother adding it 
		if (current_loadout_entry.item_name != NULL
			&& !strncmp(str, current_loadout_entry.item_name, LOADOUT_MAX_STRLEN))
		{
			//autoswitching is governed by server
			// we have to read this otherwise the protocol gets out of sync with the message and the game dies
			MSG_ReadString(&net_message);
			MSG_ReadShort(&net_message); 
			return;
		}
	}

	strncpy(&cl.loadout.items[cl.loadout.num_items].item_name, str, LOADOUT_MAX_STRLEN);
	str = MSG_ReadString(&net_message);
	strncpy(&cl.loadout.items[cl.loadout.num_items].icon, str, LOADOUT_MAX_STRLEN);

	cl.loadout.items[cl.loadout.num_items].amount = MSG_ReadShort(&net_message);

	cl.loadout.num_items++;
	Loadout_UpdateUI();
}

void Loadout_UpdateUI()
{
	// this is so items arent seen to "skip" if we lose one of the items in the middle
	int32_t item_num_visual = 0; 

	for (int32_t item_num = 0; item_num < cl.loadout.num_items; item_num++)
	{
		loadout_entry_t* loadout_entry_ptr = &cl.loadout.items[item_num];

		// turn on the parts of the UI...
		char loadout_ui_buffer[MAX_UI_STRLEN] = { 0 };

		if (item_num <= 9
			&& loadout_entry_ptr->item_name[0]) // TEMP - what do we do for item 11+?
		{
			snprintf(loadout_ui_buffer, MAX_UI_STRLEN, "LoadoutUI_Option%d", item_num_visual);

			// let it be visible and set the offer
			UI_SetInvisible("LoadoutUI", loadout_ui_buffer, false);
			UI_SetImage("LoadoutUI", loadout_ui_buffer, loadout_entry_ptr->icon);

			item_num_visual++;
		}
	}


}

// Removes the item with the name item_name.
void Loadout_Remove(char* item_name)
{
	// see if we already have it
	for (int32_t item_num = 0; item_num < cl.loadout.num_items; item_num++)
	{
		loadout_entry_t* current_loadout_item_ptr = &cl.loadout.items[item_num];

		// if we already have it...don't bother adding it 
		if (current_loadout_item_ptr->item_name != NULL
			&& !strncmp(item_name, current_loadout_item_ptr->item_name, LOADOUT_MAX_STRLEN))
		{
			// delete the item

			// if its the last item also decrement num_items (we just ignore null ones otherwise)
			if (item_num == (cl.loadout.num_items))
				cl.loadout.num_items--;

			memset(&cl.loadout.items[item_num], 0x00, sizeof(loadout_entry_t));
			return;
		}
	}


	cl.loadout.num_items--;
	Loadout_UpdateUI();
}

void Loadout_SetCurrent(int32_t index)
{
	cl.loadout.num_items = index;
}