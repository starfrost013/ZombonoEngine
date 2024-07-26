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
// cl_ui_mainmenu_quickstart.c: Quickstart menu (June 16, 2024)

#include <client/client.h>

void UI_MainMenuQuickstartUIOnBackPressed(uint32_t btn, int32_t x, int32_t y);

bool UI_MainMenuQuickstartUICreate()
{
	if (!ui_newmenu->value)
		return true;

	UI_SetStackable("MainMenuQuickstartUI", true);
	UI_AddImage("MainMenuQuickstartUI", "UI_MainMenuQuickstartUI_Background", "2d/ui/mainmenuquickstartui_background", 0, 0, r_width->value, r_height->value);
	UI_SetImageIsStretched("MainMenuQuickstartUI", "UI_MainMenuQuickstartUI_Background", true);
	UI_UseScaledAssets("MainMenuQuickstartUI", "UI_MainMenuQuickstartUI_Background", true);

	UI_AddImage("MainMenuQuickstartUI", "MainMenuQuickstartUI_Back", "2d/ui/global_btn_back", 0.75f, 0.87f, 256, 64);
	UI_SetImageOnHover("MainMenuQuickstartUI", "MainMenuQuickstartUI_Back", "2d/ui/global_btn_back_hover");
	UI_SetEventOnClickDown("MainMenuQuickstartUI", "MainMenuQuickstartUI_Back", UI_MainMenuQuickstartUIOnBackPressed);

	uint32_t size_x = 0, size_y = 0;


	Text_GetSize("bahnschrift_bold_18", &size_x, &size_y, "[STRING_QUICKSTARTUI_STARTGAMECOOP]");

	// first level menu options
	// aligned with the back sign
	UI_AddText("MainMenuQuickstartUI", "MainMenuQuickstartUI_Coop", "[STRING_QUICKSTARTUI_STARTGAMECOOP]", 
		1.00f - ((float)size_x/UI_SCALE_BASE_X) + 0.045f, 0.75f);
	Text_GetSize("bahnschrift_bold_18", &size_x, &size_y, "[STRING_QUICKSTARTUI_STARTGAMECOMPETITIVE]");
	UI_AddText("MainMenuQuickstartUI", "MainMenuQuickstartUI_Competitive", "[STRING_QUICKSTARTUI_STARTGAMECOMPETITIVE]", 
		1.00f - ((float)size_x / UI_SCALE_BASE_X) + 0.062f, 0.81f);

	// set the font to big bahnschrift
	UI_SetFont("MainMenuQuickstartUI", "MainMenuQuickstartUI_Coop", "bahnschrift_bold_18");
	UI_SetFont("MainMenuQuickstartUI", "MainMenuQuickstartUI_Competitive", "bahnschrift_bold_18");

	// create uI
	return true;
}

// is this enough to justify adding global UI assets?
void UI_MainMenuQuickstartUIOnBackPressed(int32_t btn, int32_t x, int32_t y)
{
	UI_Pop();
}