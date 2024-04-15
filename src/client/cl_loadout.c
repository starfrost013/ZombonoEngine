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

// cl_loadout.c: Loadout system (April 12, 2024)

void Loadout_Add()
{
	if (cl.loadout.current_index >= LOADOUT_MAX_ITEMS)
	{
		Com_Printf("ERROR: Tried to create loadout item %d, max %d...", LOADOUT_MAX_ITEMS);
		return;
	}

	strncpy(&cl.loadout.items[cl.loadout.current_index].item_name, MSG_ReadString(&net_message), LOADOUT_MAX_STRLEN);
	cl.loadout.items[cl.loadout.current_index].amount = MSG_ReadInt(&net_message);
	
	// basically a stack
	cl.loadout.current_index++;
}

void Loadout_Update()
{
	int32_t index = MSG_ReadByte(&net_message);

	// we set everything to an empty string when the player joins...
	// this means its not a valid item
	if (strlen(cl.loadout.items[index].item_name) == 0)
	{
		Com_Printf("ERROR: Tried to set nonexistent loadout index %d", index);
	}

	strncpy(&cl.loadout.items[cl.loadout.current_index].item_name, MSG_ReadString(&net_message), LOADOUT_MAX_STRLEN);
	cl.loadout.items[cl.loadout.current_index].amount = MSG_ReadInt(&net_message);
}

// Removes the most recently added item.
void Loadout_Remove()
{
	memset(&cl.loadout.items[cl.loadout.current_index], 0x00, sizeof(loadout_entry_t));
	cl.loadout.current_index--;
}

void Loadout_SetCurrent(int32_t index)
{
	cl.loadout.current_index = index;
}