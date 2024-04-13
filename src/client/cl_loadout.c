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

void Loadout_Parse()
{
	int32_t num_items = MSG_ReadByte(&net_message);

	if (num_items < 0
		|| num_items > LOADOUT_MAX_ITEMS)
	{
		Com_Error(ERR_DROP, "Invalid number of items provided in an svc_loadout message");
		return;
	}

	for (int32_t item_num = 0; item_num < num_items; item_num++)
	{
		strncpy(&cl.loadout.items[item_num].item_name, MSG_ReadString(&net_message), LOADOUT_MAX_STRLEN);
		cl.loadout.items[item_num].amount = MSG_ReadLong(&net_message);
	}
}

void Loadout_SetCurrent(int32_t index)
{
	cl.loadout.current_index = index;
}