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
// cl_ui_mainmenu.c: New! main menu (April 20, 2024)

#include "client.h"

bool UI_MainMenuUICreate()
{
	// don't bother creating it if ui_newmenu is not set to 1
	if (!ui_newmenu->value)
		return true; 

	if (viddef.width >= 1920)
	{
		UI_AddImage("MainMenuUI", "UI_MainMenuUI_Background", "pics/ui/mainmenuui_background", 0, 0, viddef.width, viddef.height);
	}
	else if (viddef.width >= 960)
	{
		UI_AddImage("MainMenuUI", "UI_MainMenuUI_Background", "pics/ui/mainmenuui_background@0.5x", 0, 0, viddef.width, viddef.height);
	}
	else
	{
		UI_AddImage("MainMenuUI", "UI_MainMenuUI_Background", "pics/ui/mainmenuui_background@0.25x", 0, 0, viddef.width, viddef.height);
	}
	
	UI_SetImageIsStretched("MainMenuUI", "UI_MainMenuUI_Background", true);

	UI_AddImage("MainMenuUI", "UI_MainMenuUI_ZombonoLogo", "pics/zombono_logo_lores_red", 0, 0, (500 * vid_hudscale->value), (108 * vid_hudscale->value));
	int32_t x = (10 * vid_hudscale->value);
	int32_t y = (viddef.width / 1.85f);
	//todo: scale these?
	UI_AddImage("MainMenuUI", "UI_MainMenuUI_BtnCoop", "pics/ui/mainmenuui_btn_coop", x, y, 256, 40); 
	y += 40 * vid_hudscale->value;
	UI_AddImage("MainMenuUI", "UI_MainMenuUI_BtnBrowseServers", "pics/ui/mainmenuui_btn_browseservers", x, y, 256, 40);
	y += 40 * vid_hudscale->value;
	UI_AddImage("MainMenuUI", "UI_MainMenuUI_BtnZombieTelevision", "pics/ui/mainmenuui_btn_zombietelevision", x, y, 256, 40);
	y += 40 * vid_hudscale->value;
	UI_AddImage("MainMenuUI", "UI_MainMenuUI_BtnSettings", "pics/ui/mainmenuui_btn_settings", x, y, 256, 40);
	y += 40 * vid_hudscale->value;
	UI_AddImage("MainMenuUI", "UI_MainMenuUI_BtnQuit", "pics/ui/mainmenuui_btn_quit", x, y, 256, 40);
	y += 40 * vid_hudscale->value;

	// Main Menu buttons onhover images
	UI_SetImageOnHover("MainMenuUI", "UI_MainMenuUI_BtnCoop", "pics/ui/mainmenuui_btn_coop_hover");
	UI_SetImageOnHover("MainMenuUI", "UI_MainMenuUI_BtnBrowseServers", "pics/ui/mainmenuui_btn_browseservers_hover");
	UI_SetImageOnHover("MainMenuUI", "UI_MainMenuUI_BtnZombieTelevision", "pics/ui/mainmenuui_btn_zombietelevision_hover");
	UI_SetImageOnHover("MainMenuUI", "UI_MainMenuUI_BtnSettings", "pics/ui/mainmenuui_btn_settings_hover");
	UI_SetImageOnHover("MainMenuUI", "UI_MainMenuUI_BtnQuit", "pics/ui/mainmenuui_btn_quit_hover");

	return true;
}