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
	if (current_ui == NULL) return;
	if (strncmp(current_ui->name, "TeamUI", 6)) return;

	UI_SetActive("TeamUI", false);
	UI_SetEnabled("TeamUI", false);
	MSG_WriteByte(&cls.netchan.message, clc_stringcmd_noconsole);
	MSG_WriteString(&cls.netchan.message, "setteam 1");
}

void UI_TeamUISetPlayerTeam(int btn, int x, int y)
{
	if (current_ui == NULL) return;
	if (strncmp(current_ui->name, "TeamUI", 6)) return;

	UI_SetActive("TeamUI", false);
	UI_SetEnabled("TeamUI", false);
	MSG_WriteByte(&cls.netchan.message, clc_stringcmd_noconsole);
	MSG_WriteString(&cls.netchan.message, "setteam 2");
}

//
// LeaderboardUI is in cl_ui_leaderboard.c
//

//
// BamfuslicatorUI
//

qboolean UI_BamfuslicatorUICreate()
{
	UI_SetPassive("BamfuslicatorUI", true);
	UI_AddBox("BamfuslicatorUI", "BamfuslicatorUI_TextBackground", (viddef.width / 2) - (150 * vid_hudscale->value), (viddef.height / 1.25), 8 * 48, 8, 232, 96, 0, 255);
	UI_AddText("BamfuslicatorUI", "BamfuslicatorUI_Text", "Zombie Type: **** UNKNOWN ZOMBIE TYPE ****", (viddef.width / 2) - (150 * vid_hudscale->value), (viddef.height / 1.25));
	return true; 
}