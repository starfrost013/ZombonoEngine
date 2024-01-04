/*
Copyright (C) 1997-2001 Id Software, Inc.
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

// cl_ui_scripts.c: ZombonoUI UI creation scripts (December 13 - 14, 2023)

#include "client.h"

// 
// TeamUI
// 

qboolean UI_TeamUICreate()
{
	UI_AddText("TeamUI", "TeamUI_TeamSelectText", "T E A M  S E L E C T", viddef.width / 2 - 64, (viddef.height / 2) - 96);
	UI_AddImage("TeamUI", "TeamUI_DirectorTeam", "ui/teamui_btn_director", (viddef.width / 2) - 256, (viddef.height / 2) - 64, 256, 128);
	UI_AddImage("TeamUI", "TeamUI_PlayerTeam", "ui/teamui_btn_player", viddef.width / 2, (viddef.height / 2) - 64, 256, 128);

	UI_SetEventOnClick("TeamUI", "TeamUI_DirectorTeam", UI_TeamUISetDirectorTeam);
	UI_SetEventOnClick("TeamUI", "TeamUI_PlayerTeam", UI_TeamUISetPlayerTeam);
	return true; 
}

void UI_TeamUISetDirectorTeam(int btn, int x, int y)
{
	UI_SetActive("TeamUI", false);
	UI_SetEnabled("TeamUI", false);
	MSG_WriteByte(&cls.netchan.message, clc_stringcmd_noconsole);
	MSG_WriteString(&cls.netchan.message, "setteam 1");
}

void UI_TeamUISetPlayerTeam(int btn, int x, int y)
{
	UI_SetActive("TeamUI", false);
	UI_SetEnabled("TeamUI", false);
	MSG_WriteByte(&cls.netchan.message, clc_stringcmd_noconsole);
	MSG_WriteString(&cls.netchan.message, "setteam 2");
}

//
// LeaderboardUI
//

qboolean UI_LeaderboardUICreate()
{
	UI_AddBox("LeaderboardUI", "LeaderboardUI_Box", (viddef.width / 2) - 320, (viddef.height / 2) - 192, 640, 384, 0, 0, 0, 255); // why does alpha not work. wtf
	UI_SetEventOnKeyDown("LeaderboardUI", "LeaderboardUI_Box", UI_LeaderboardUIToggle);
	UI_AddImage("LeaderboardUI", "LeaderboardUI_Header", "ui/leaderboardui_header", (viddef.width / 2) - 160, (viddef.height / 2) - 192, 320, 64);
	UI_AddText("LeaderboardUI", "LeaderboardUI_Subheader_Name", "Name", (viddef.width / 2) - 304, (viddef.height / 2) - 124);
	UI_AddText("LeaderboardUI", "LeaderboardUI_Subheader_Ping", "Ping", (viddef.width / 2) - 144, (viddef.height / 2) - 124);
	UI_AddText("LeaderboardUI", "LeaderboardUI_Subheader_Team", "Team", (viddef.width / 2) - 64, (viddef.height / 2) - 124);
	UI_AddText("LeaderboardUI", "LeaderboardUI_Subheader_Score", "Score", (viddef.width / 2) + 32, (viddef.height / 2) - 124);
	UI_AddText("LeaderboardUI", "LeaderboardUI_Subheader_Time", "Time", (viddef.width / 2) + 112, (viddef.height / 2) - 124);
	UI_AddText("LeaderboardUI", "LeaderboardUI_Subheader_Spectating", "Spectating?", (viddef.width / 2) + 192, (viddef.height / 2) - 124);
	return true;
}

void UI_LeaderboardUIToggle(int btn)
{
	if (btn == K_TAB)
	{
		// fucking hack
		ui_t* leaderboard_ui_ptr = UI_GetUI("LeaderboardUI");

		UI_SetEnabled("LeaderboardUI", !leaderboard_ui_ptr->enabled);
		UI_SetActive("LeaderboardUI", !leaderboard_ui_ptr->active);
	}
}