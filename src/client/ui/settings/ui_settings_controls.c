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
// ui_settings_controls.c: Control Settings (August 17, 2024)

#include <client/client.h>

char* bindnames_new[][3] = // temp name
{
{"+attack1", 		"^5[STRING_BINDING_ATTACK1]"},
{"+attack2",		"^5[STRING_BINDING_ATTACK2]"},
{"weapnext", 		"^5[STRING_BINDING_WEAPNEXT]"},
{"weapprev", 		"^5[STRING_BINDING_WEAPPREV]"},
{"weaplast", 		"^5[STRING_BINDING_WEAPLAST]"},
{"+forward", 		"^5[STRING_BINDING_FORWARD]"},
{"+back", 			"^5[STRING_BINDING_BACK]"},
{"+left", 			"^5[STRING_BINDING_LEFT]"},
{"+right", 			"^5[STRING_BINDING_RIGHT]"},
{"+sprint", 			"^5[STRING_BINDING_SPEED]"},
{"+moveleft", 		"^5[STRING_BINDING_MOVELEFT]"},
{"+moveright", 		"^5[STRING_BINDING_MOVERIGHT]"},
{"+mlook", 			"^5[STRING_BINDING_MLOOK]"},
{"+jump",			"^5[STRING_BINDING_MOVEUP]"},
{"+crouch",		"^5[STRING_BINDING_MOVEDOWN]"},

{"invuse",			"^5[STRING_BINDING_INVUSE]"},
{"invdrop",			"^5[STRING_BINDING_INVDROP]"},
{"invprev",			"^5[STRING_BINDING_INVPREV]"},
{"invnext",			"^5[STRING_BINDING_INVNEXT]"},
{ 0, 0 }
};

void UI_SettingsControlsUIOnBackPressed(int32_t btn, int32_t x, int32_t y);

bool UI_SettingsControlsUICreate()
{
	if (!ui_newmenu->value)
		return true;

	UI_SetStackable("SettingsControlsUI", true);

	// background
	UI_AddImage("SettingsControlsUI", "UI_SettingsControlsUI_Background", "2d/ui/mainmenu/settingscontrolsui_background", 0, 0, r_width->value, r_height->value);
	UI_SetImageIsStretched("SettingsControlsUI", "UI_SettingsControlsUI_Background", true);
	UI_UseScaledAssets("SettingsControlsUI", "UI_SettingsControlsUI_Background", true);

	UI_AddImage("SettingsControlsUI", "SettingsControlsUI_Back", "2d/ui/global_btn_back", 0.75f, 0.87f, 256, 64);
	UI_SetImageOnHover("SettingsControlsUI", "SettingsControlsUI_Back", "2d/ui/global_btn_back_hover");
	UI_SetEventOnClickDown("SettingsControlsUI", "SettingsControlsUI_Back", UI_SettingsControlsUIOnBackPressed);

	UI_AddText("SettingsControlsUI", "SettingsControlsUI_Header", "^0Modify Controls", 0.25f, 0.25f);

	int32_t bind_num = 0;

	while (bind)

	return true;
}

// is this enough to justify adding global UI assets?
void UI_SettingsControlsUIOnBackPressed(int32_t btn, int32_t x, int32_t y)
{
	UI_Pop();
}