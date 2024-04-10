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

// cl_ui_scripts.c: ZombonoUI UI creation scripts
// 12/14/2023: Created

#include "client.h"

// 
// TeamUI
// 

bool UI_TeamUICreate()
{
	UI_AddText("TeamUI", "TeamUI_TeamSelectText", "T E A M  S E L E C T", viddef.width / 2 - (48 * vid_hudscale->value), (viddef.height / 2) - (80 * vid_hudscale->value));
	UI_AddImage("TeamUI", "TeamUI_DirectorTeam", "pics/ui/teamui_btn_director", (viddef.width / 2) - (256 * vid_hudscale->value), (viddef.height / 2) - (64 * vid_hudscale->value), 256, 128);
	UI_AddImage("TeamUI", "TeamUI_PlayerTeam", "pics/ui/teamui_btn_player", viddef.width / 2, (viddef.height / 2) - (64 * vid_hudscale->value), 256, 128);

	UI_AddText("TeamUI", "TeamUI_DirectorText", "Possesses the ^2Bamfuslicator^7 to spawn various\n^1undead^7. Moves slower.", viddef.width / 2 - (256 * vid_hudscale->value), (viddef.height / 2) + (70 * vid_hudscale->value));
	UI_AddText("TeamUI", "TeamUI_PlayerText", "Can't spawn ^1anything^7, but has access to more\n^2weapons^7. Moves faster.", viddef.width / 2, (viddef.height / 2) + (70 * vid_hudscale->value));

	UI_SetEventOnClickDown("TeamUI", "TeamUI_DirectorTeam", UI_TeamUISetDirectorTeam);
	UI_SetEventOnClickDown("TeamUI", "TeamUI_PlayerTeam", UI_TeamUISetPlayerTeam);
	return true; 
}

void UI_TeamUISetDirectorTeam(int32_t btn, int32_t x, int32_t y)
{
	if (current_ui == NULL) return;
	if (strncmp(current_ui->name, "TeamUI", 6)) return;

	UI_SetActive("TeamUI", false);
	UI_SetEnabled("TeamUI", false);
	MSG_WriteByte(&cls.netchan.message, clc_stringcmd_noconsole);
	MSG_WriteString(&cls.netchan.message, "setteam 1");
}

void UI_TeamUISetPlayerTeam(int32_t btn, int32_t x, int32_t y)
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

bool UI_BamfuslicatorUICreate()
{
	int32_t size_x = 0, size_y = 0;
	UI_SetPassive("BamfuslicatorUI", true);
	// we only care about precise Y size here
	// as all the text actually being drawn is smaller than the temp text, this positions it in the middle
	const char* temp_text = "Zombie Type: **** UNKNOWN ZOMBIE TYPE ****"; 
	Text_GetSize(cl_system_font->string, &size_x, &size_y, temp_text);
	int32_t x = (viddef.width / 2) - (120 * vid_hudscale->value);
	UI_AddBox("BamfuslicatorUI", "BamfuslicatorUI_TextBackground", x, 
		(viddef.height / 1.25), 240 * vid_hudscale->value, size_y * vid_hudscale->value, 232, 96, 0, 127);
	// terrible manual positioning because we don't know what the text is (changed at runtime)
	UI_AddText("BamfuslicatorUI", "BamfuslicatorUI_Text", temp_text, (x + (70 * vid_hudscale->value)), (viddef.height / 1.25));
	UI_AddText("BamfuslicatorUI", "BamfuslicatorUI_TextHelp", "Right click to change type of monster to spawn",
		x + (5 * vid_hudscale->value), (viddef.height / 1.25) + (size_y * vid_hudscale->value) + (3 * vid_hudscale->value)); // lazy way of centering it lol
	return true; 
}

//
// TimeUI
//

bool UI_TimeUICreate()
{
	int32_t size_x = 0, size_y = 0;

	UI_SetPassive("TimeUI", true);

	const char* temp_text = "Time: x:xx";
	Text_GetSize(cl_system_font->string, &size_x, &size_y, temp_text);

	UI_AddBox("TimeUI", "TimeUI_TextBox", (viddef.width / 2) - (size_x / 2) - (36 * vid_hudscale->value), 10 * vid_hudscale->value,
		size_x * vid_hudscale->value + (72 * vid_hudscale->value), size_y * vid_hudscale->value, 255, 0, 0, 180); // add a buffer of 10 pixels for larger numbers

	// text is set by gamecode
	UI_AddText("TimeUI", "TimeUI_Text", "N/A", (viddef.width / 2) - (size_x / 2), 11 * vid_hudscale->value); //+1 for padding/advance reasons

	return true; 
}

//
// ScoreUI
//

bool UI_ScoreUICreate()
{
	int32_t size_x = 0, size_y = 0;

	UI_SetPassive("ScoreUI", true);

	const char* temp_text = "Directors 0 : Players 0";
	Text_GetSize(cl_system_font->string, &size_x, &size_y, temp_text);
	//87, 0, 127, 255
	UI_AddBox("ScoreUI", "ScoreUI_TextBoxDirector", (viddef.width / 2) - (size_x / 2) - (24 * vid_hudscale->value), 30 * vid_hudscale->value,
		(size_x * vid_hudscale->value + (24 * vid_hudscale->value) / 2), size_y * vid_hudscale->value, 87, 0, 127, 255); // add a buffer of 10 pixels for larger numbers 
	UI_AddBox("ScoreUI", "ScoreUI_TextBoxPlayer", (viddef.width / 2) - (size_x / 2) + (62 * vid_hudscale->value), 30 * vid_hudscale->value,
		(size_x * vid_hudscale->value - (70 * vid_hudscale->value) / 2), size_y * vid_hudscale->value, 255, 106, 0, 255); // add a buffer of 10 pixels for larger numbers 

	// text is set by gamecode
	UI_AddText("ScoreUI", "ScoreUI_Text", "N/A", (viddef.width / 2) - (size_x / 2), 
		(30 * vid_hudscale->value) + (1 * vid_hudscale->value));

	return true; 
}

//
// LoadoutUI
//

bool UI_LoadoutUICreate()
{

}
