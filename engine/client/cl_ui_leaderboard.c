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

// cl_ui_leaderboard.c: The new leaderboard UI! (December 24-25, 2023)

#include "client.h"

#define TEXT_BUF_LENGTH			16

void UI_LeaderboardUIUpdate()
{
	int x, y;
	char text[TEXT_BUF_LENGTH];

	// stupid hack
	UI_Clear("LeaderboardUI");
	UI_LeaderboardUICreate();

	// byte to reduce net usage
	cl.leaderboard.num_clients = MSG_ReadByte(&net_message);

	y = (viddef.height / 2) - 124;

	// update all the data here so we don't need to clear it
	for (int client_num = 0; client_num < cl.leaderboard.num_clients; client_num++)
	{
		// reset x
		x = (viddef.width / 2) - 304;

		leaderboard_entry_t leaderboard_entry = cl.leaderboard.entries[client_num];
		strncpy(leaderboard_entry.name, MSG_ReadString(&net_message), 32);
		leaderboard_entry.ping = MSG_ReadShort(&net_message);
		leaderboard_entry.score = MSG_ReadShort(&net_message);
		leaderboard_entry.team = MSG_ReadShort(&net_message);
		leaderboard_entry.time = MSG_ReadShort(&net_message); //should this be an int?
		leaderboard_entry.is_spectator = MSG_ReadByte(&net_message);
		strncpy(leaderboard_entry.map_name, MSG_ReadString(&net_message), 32);
		leaderboard_entry.time_remaining = MSG_ReadShort(&net_message);

		// todo: boxes and headers (headers in cl_ui_scripts)
		
		// move by one line
		y += 12;

		// draw name
		UI_AddText("LeaderboardUI", "LeaderboardUIText_TempName", leaderboard_entry.name, x, y);

		x += 8 * 20;
		
		// ping
		snprintf(text, TEXT_BUF_LENGTH, "%d", leaderboard_entry.ping);
		UI_AddText("LeaderboardUI", "LeaderboardUIText_TempPing", text, x, y);

		x += 8 * 10;

		//team
		int box_size = 8 * 11; // a bit of padding

		if (leaderboard_entry.team == common_team_director)
		{
			UI_AddBox("LeaderboardUI", "LeaderboardUIText_TempTeamBox", x, y, box_size, 8, 87, 0, 127, 255);
			UI_AddText("LeaderboardUI", "LeaderboardUIText_TempTeam", "Director", x, y);
		}
		else if (leaderboard_entry.team == common_team_player)
		{
			UI_AddBox("LeaderboardUI", "LeaderboardUIText_TempTeamBox", x, y, box_size, 8, 219, 87, 0, 255);
			UI_AddText("LeaderboardUI", "LeaderboardUIText_TempTeam", "Player", x, y);
		}
		else
		{
			UI_AddBox("LeaderboardUI", "LeaderboardUIText_TempTeamBox", x, y, box_size, 8, 127, 127, 127, 255);
			UI_AddText("LeaderboardUI", "LeaderboardUIText_TempTeam", "Unassigned", x, y);
		}

		x += 8 * 12;
		
		// score
		snprintf(text, TEXT_BUF_LENGTH, "%d", leaderboard_entry.score);
		UI_AddText("LeaderboardUI", "LeaderboardUIText_TempScore", text, x, y);

		// time
		x += 8 * 10;
		snprintf(text, TEXT_BUF_LENGTH, "%d seconds", leaderboard_entry.time);
		UI_AddText("LeaderboardUI", "LeaderboardUIText_TempTime", text, x, y);

		x += 8 * 10;
		// are they spectating?
		if (leaderboard_entry.is_spectator)
		{
			UI_AddText("LeaderboardUI", "LeaderboardUIText_TempIsSpectating", "Yes", x, y);
		}
		else
		{
			UI_AddText("LeaderboardUI", "LeaderboardUIText_TempIsSpectating", "No", x, y);
		}

		// stupid fucking hack!!!

		if (client_num == 0)
		{
			x = (viddef.width / 2) - 320;
			y = (viddef.height / 2) + 176;

			char map_buf[36]; // 31 map name length + 5 for "Map: "
			char time_buf[38];  // 31 map name length + 7 for "Time: " and optional 0

			snprintf(map_buf, 36, "Map: %s", leaderboard_entry.map_name);

			UI_AddText("LeaderboardUI", "LeaderboardUIText_TempMapName", map_buf, x, y);

			y += 8;

			int seconds = leaderboard_entry.time_remaining % 60;

			if (seconds < 10)
			{
				snprintf(time_buf, 38, "Time: %d:0%d", leaderboard_entry.time_remaining / 60, seconds);
			}
			else
			{
				snprintf(time_buf, 38, "Time: %d:%d", leaderboard_entry.time_remaining / 60, seconds);
			}

			UI_AddText("LeaderboardUI", "LeaderboardUIText_TempTime", time_buf, x, y);
		}
	}
	
	// You need to toggle it here otherwise you can never turn it on because UIs not being disabled don't get events.
	// Writing UI code is like being shot into the sun.
	if (current_ui == NULL || strcmp(current_ui->name, "TeamUI")) UI_LeaderboardUIToggle(K_TAB);
}