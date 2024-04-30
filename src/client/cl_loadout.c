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
			MSG_ReadInt(&net_message); 
			return;
		}
	}

	strncpy(&cl.loadout.items[cl.loadout.num_items].item_name, str, LOADOUT_MAX_STRLEN);
	str = MSG_ReadString(&net_message);
	strncpy(&cl.loadout.items[cl.loadout.num_items].icon, str, LOADOUT_MAX_STRLEN);

	cl.loadout.items[cl.loadout.num_items].amount = MSG_ReadInt(&net_message);

	// basically a stack
	cl.loadout.num_items++;
}

void Loadout_Update()
{
	int32_t index = MSG_ReadByte(&net_message);

	// we set everything to an empty string when the player joins...
	// this means its not a valid item
	if (strlen(cl.loadout.items[index].item_name) == 0)
	{
		Com_Printf("ERROR: Tried to set nonexistent loadout index %d", index);
		return;
	}

	strncpy(&cl.loadout.items[cl.loadout.num_items].item_name, MSG_ReadString(&net_message), LOADOUT_MAX_STRLEN);
	cl.loadout.items[cl.loadout.num_items].amount = MSG_ReadInt(&net_message);
}

// Draws the loadout UI
void Loadout_Draw()
{
	for (int32_t item_num = 0; item_num < cl.loadout.num_items; item_num++)
	{
		loadout_entry_t* loadout_entry_ptr = &cl.loadout.items[item_num];


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
}

void Loadout_SetCurrent(int32_t index)
{
	cl.loadout.num_items = index;
}