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
#include <client/client.h>
#include <client/include/menu_framework.h>

#define REF_OPENGL	0

#define NUM_REFS	1

extern cvar_t* vid_ref;
extern cvar_t* vid_borderless;
extern cvar_t* vid_fullscreen;
extern cvar_t* vid_gamma;
extern cvar_t* vid_hudscale;
extern cvar_t* scr_viewsize;

static cvar_t* gl_picmip;
static cvar_t* gl_vsync;
static cvar_t* gl_texturemode;

extern void M_ForceMenuOff();

/*
====================================================================

MENU INTERACTION

====================================================================
*/
#define OPENGL_MENU   0

#define NUM_REF_MENUS NUM_REFS

static menuframework_t	s_opengl_menu;
static menuframework_t* s_current_menu;
static int32_t 			s_current_menu_index;

static bool				is_dedicated_fullscreen;

static menulist_t		s_mode_list[NUM_REF_MENUS];
static menulist_t		s_ref_list[NUM_REF_MENUS];
static menuslider_t		s_tq_slider;
static menuslider_t		s_screensize_slider[NUM_REF_MENUS];
static menuslider_t		s_brightness_slider[NUM_REF_MENUS];
static menulist_t  		s_fullscreen_box[NUM_REF_MENUS];
static menulist_t		s_fullscreen_dedicated_box[NUM_REF_MENUS];
static menulist_t		s_texture_filtering_box[NUM_REF_MENUS];
static menulist_t		s_vsync_box;
static menuaction_t		s_apply_action[NUM_REF_MENUS];
static menuaction_t		s_cancel_action[NUM_REF_MENUS];
static menuaction_t		s_defaults_action[NUM_REF_MENUS];

// These are not real video modes, these are used so the user can select sane resolutions in the menu
typedef struct vidmode_s
{
	const char* description;
	int32_t     width, height;
	int32_t     mode;
} vidmode_t;

// These are strictly virtual modes and the driver only cares about r_width and r_height so we don't bother with them
vidmode_t vid_modes[] =
{
	{ "Mode 0: 320x240",   320, 240,   0 },
	{ "Mode 1: 400x300",   400, 300,   1 },
	{ "Mode 2: 512x384",   512, 384,   2 },
	{ "Mode 3: 640x480",   640, 480,   3 },
	{ "Mode 4: 800x600",   800, 600,   4 },
	{ "Mode 5: 960x720",   960, 720,   5 },
	{ "Mode 6: 1024x768",  1024, 768,  6 },
	{ "Mode 7: 1152x864",  1152, 864,  7 },
	{ "Mode 8: 1280x960",  1280, 960,  8 },
	{ "Mode 9: 1366x768",  1366, 768,  9 },
	{ "Mode 10: 1440x900", 1440, 900,  10 },
	{ "Mode 11: 1600x900", 1600, 900,  11 },
	{ "Mode 12: 1600x1200", 1600, 1200, 12 },
	{ "Mode 13: 1920x1080", 1920, 1080, 13 },
	{ "Mode 14: 1920x1200", 1920, 1200, 14 },
	{ "Mode 15: 2048x1536", 2048, 1536, 15 },
	{ "Mode 16: 2560x1440", 2560, 1440, 16 },
	{ "Mode 17: 3840x2160", 3840, 2160, 17 },
};

#define VID_NUM_MODES ( sizeof( vid_modes ) / sizeof( vid_modes[0] ) )

static void VidMenu_DriverCallback(void* unused)
{
	int32_t curr_value = s_ref_list[s_current_menu_index].curvalue;

	s_ref_list[VIDREF_GL].curvalue = curr_value;

	// this menu will be replaced anyway
	s_current_menu = &s_opengl_menu;
	s_current_menu_index = REF_OPENGL;
}

static void ScreenSizeCallback(void* s)
{
	menuslider_t* slider = (menuslider_t*)s;

	Cvar_SetValue("viewsize", slider->curvalue * 10);
}

static void BrightnessCallback(void* s)
{
	menuslider_t* slider = (menuslider_t*)s;
	int32_t i;

	for (i = 0; i < NUM_REFS; i++)
	{
		s_brightness_slider[i].curvalue = s_brightness_slider[s_current_menu_index].curvalue;
	}
}

static void DedicatedFullscreenCallback(void* s)
{
	is_dedicated_fullscreen = s_fullscreen_dedicated_box[s_current_menu_index].curvalue;
}

static void TextureFilteringCallback(void* unused)
{
	if (s_texture_filtering_box[s_current_menu_index].curvalue == 0)
	{
		gl_texturemode->string = "GL_NEAREST";
	}
	else if (s_texture_filtering_box[s_current_menu_index].curvalue == 1)
	{
		gl_texturemode->string = "GL_LINEAR";
	}
	else
	{
		gl_texturemode->string = "GL_LINEAR_MIPMAP_LINEAR";
	}

	gl_texturemode->modified = true;
}

static void ResetDefaults(void* unused)
{
	Vid_MenuInit();
}

static void ApplyChanges(void* unused)
{
	float gamma;
	int32_t i;

	/*
	** make values consistent
	*/
	for (i = 0; i < NUM_REFS; i++)
	{
		s_fullscreen_box[i].curvalue = s_fullscreen_box[s_current_menu_index].curvalue;
		s_brightness_slider[i].curvalue = s_brightness_slider[s_current_menu_index].curvalue;
		s_ref_list[i].curvalue = s_ref_list[s_current_menu_index].curvalue;
	}

	/*
	** invert sense so greater = brighter, and scale to a range of 0.5 to 1.3
	*/
	gamma = (0.8f - (s_brightness_slider[s_current_menu_index].curvalue / 10.0f - 0.5f)) + 0.5f;

	Cvar_SetValue("vid_gamma", gamma);
	Cvar_SetValue("gl_picmip", 3 - s_tq_slider.curvalue);

	// if the user wants dedi, use it...
	if (is_dedicated_fullscreen)
	{
		Cvar_SetValue("vid_fullscreen", s_fullscreen_box[s_current_menu_index].curvalue);
		Cvar_SetValue("vid_borderless", 0);
	}
	else
	{
		Cvar_SetValue("vid_borderless", s_fullscreen_box[s_current_menu_index].curvalue);
		Cvar_SetValue("vid_fullscreen", 0);
	}

	Cvar_SetValue("gl_vsync", s_vsync_box.curvalue);

	// "custom" menu option doesn't resize
	if (s_mode_list[OPENGL_MENU].curvalue > 0)
	{
		// -1 for the "custom" option
		Cvar_SetValue("r_width", vid_modes[s_mode_list[OPENGL_MENU].curvalue - 1].width);
		Cvar_SetValue("r_height", vid_modes[s_mode_list[OPENGL_MENU].curvalue - 1].height);
	}

	switch (s_ref_list[s_current_menu_index].curvalue)
	{
	case REF_OPENGL:
		Cvar_Set("vid_ref", "gl");
		break;
	}

	/*
	** update appropriate stuff if we're running OpenGL and gamma
	** has been modified
	*/
	if (stricmp(vid_ref->string, "gl") == 0)
	{
		if (vid_gamma->modified)
			vid_ref->modified = true;
	}

	M_ForceMenuOff();
}

static void CancelChanges(void* unused)
{
	extern void M_PopMenu();

	M_PopMenu();
}

/*
** VID_MenuInit
*/
void Vid_MenuInit()
{
	// THIS LIST **MUST** CORRESPOND EXACTLY WITH THE LIST IN VID_DLL.C!!! (except custom)
	static const char* resolutions[] =
	{
		"[custom   ]",
		"[320 240  ]",
		"[400 300  ]",
		"[512 384  ]",
		"[640 480  ]",
		"[800 600  ]",
		"[960 720  ]",
		"[1024 768 ]",
		"[1152 864 ]",
		"[1280 960 ]",
		"[1366 768 ]",
		"[1440 900 ]",
		"[1600 900 ]",
		"[1600 1200]",
		"[1920 1080]",
		"[1920 1200]",
		"[2048 1536]",
		"[2560 1440]",
		"[3840 2160]",
		0
	};
	static const char* refs[] =
	{
		"[default OpenGL]",
		0
	};
	static const char* yes_no_names[] =
	{
		"^1No",
		"^2Yes",
		0
	};

	static const char* filter_modes[] =
	{
		"None",
		"Bilinear",
		"Trilinear",
		0
	};

	int32_t i;

	if (!gl_picmip)
		gl_picmip = Cvar_Get("gl_picmip", "0", 0);
	if (!r_width)
		r_width = Cvar_Get("r_width", "1366", CVAR_ARCHIVE);
	if (!r_height)
		r_height = Cvar_Get("r_height", "768", CVAR_ARCHIVE);
	if (!gl_vsync)
		gl_vsync = Cvar_Get("gl_vsync", "0", CVAR_ARCHIVE);
	if (!gl_texturemode)
		gl_texturemode = Cvar_Get("gl_texturemode", "GL_LINEAR_MIPMAP_LINEAR", CVAR_ARCHIVE);

	// default to the first item
	s_mode_list[OPENGL_MENU].curvalue = 0; 	// by default it will display "custom"

	// since modes don't actually exist now and the game can run at an arbitrary resolution we have to search for the correct item
	for (int32_t mode_num = 0; mode_num < VID_NUM_MODES; mode_num++)
	{
		if (vid_modes[mode_num].width == r_width->value)
		{
			s_mode_list[OPENGL_MENU].curvalue = mode_num + 1; // +1 for "custom" option
			break;
		}

	}

	if (!scr_viewsize)
		scr_viewsize = Cvar_Get("viewsize", "100", CVAR_ARCHIVE);

	s_screensize_slider[OPENGL_MENU].curvalue = scr_viewsize->value / 10;

	if (strcmp(vid_ref->string, "gl") == 0)
	{
		s_current_menu_index = OPENGL_MENU;
		s_ref_list[s_current_menu_index].curvalue = REF_OPENGL;
	}

	s_opengl_menu.x = r_width->value * 0.50;
	s_opengl_menu.nitems = 0;

	for (i = 0; i < NUM_REF_MENUS; i++)
	{
		s_ref_list[i].generic.type = MTYPE_SPINCONTROL;
		s_ref_list[i].generic.name = "^5Driver";
		s_ref_list[i].generic.x = 0;
		s_ref_list[i].generic.y = 0;
		s_ref_list[i].generic.callback = VidMenu_DriverCallback;
		s_ref_list[i].itemnames = refs;

		s_mode_list[i].generic.type = MTYPE_SPINCONTROL;
		s_mode_list[i].generic.name = "^5Video Mode";
		s_mode_list[i].generic.x = 0;
		s_mode_list[i].generic.y = 10 * vid_hudscale->value;
		s_mode_list[i].itemnames = resolutions;

		s_screensize_slider[i].generic.type = MTYPE_SLIDER;
		s_screensize_slider[i].generic.x = 0;
		s_screensize_slider[i].generic.y = 20 * vid_hudscale->value;
		s_screensize_slider[i].generic.name = "^5Screen Size";
		s_screensize_slider[i].minvalue = 4;
		s_screensize_slider[i].maxvalue = 10;
		s_screensize_slider[i].generic.callback = ScreenSizeCallback;

		s_brightness_slider[i].generic.type = MTYPE_SLIDER;
		s_brightness_slider[i].generic.x = 0;
		s_brightness_slider[i].generic.y = 30 * vid_hudscale->value;
		s_brightness_slider[i].generic.name = "^5Brightness";
		s_brightness_slider[i].generic.callback = BrightnessCallback;
		s_brightness_slider[i].minvalue = 5;
		s_brightness_slider[i].maxvalue = 13;
		s_brightness_slider[i].curvalue = (1.3f - vid_gamma->value + 0.5f) * 10;

		s_fullscreen_box[i].generic.type = MTYPE_SPINCONTROL;
		s_fullscreen_box[i].generic.x = 0;
		s_fullscreen_box[i].generic.y = 40 * vid_hudscale->value;
		s_fullscreen_box[i].generic.name = "^5Fullscreen";
		s_fullscreen_box[i].itemnames = yes_no_names;
		s_fullscreen_box[i].curvalue = ((is_dedicated_fullscreen) ? vid_fullscreen->value : vid_borderless->value);

		s_fullscreen_dedicated_box[i].generic.type = MTYPE_SPINCONTROL;
		s_fullscreen_dedicated_box[i].generic.x = 0;
		s_fullscreen_dedicated_box[i].generic.y = 50 * vid_hudscale->value;
		s_fullscreen_dedicated_box[i].generic.name = "^5Use dedicated fullscreen";
		s_fullscreen_dedicated_box[i].generic.callback = DedicatedFullscreenCallback;
		s_fullscreen_dedicated_box[i].itemnames = yes_no_names;
		s_fullscreen_dedicated_box[i].curvalue = (is_dedicated_fullscreen);

		s_fullscreen_dedicated_box[i].curvalue = (is_dedicated_fullscreen);

		s_texture_filtering_box[i].generic.type = MTYPE_SPINCONTROL;
		s_texture_filtering_box[i].generic.x = 0;
		s_texture_filtering_box[i].generic.y = 90 * vid_hudscale->value;
		s_texture_filtering_box[i].generic.name = "^5Texture filtering";
		s_texture_filtering_box[i].generic.callback = TextureFilteringCallback;
		s_texture_filtering_box[i].itemnames = filter_modes;

		if (!strcmp(gl_texturemode->string, "GL_NEAREST"))
		{
			s_texture_filtering_box->curvalue = 0;
		}
		// for some reason this is fucking on on shitpiler (MSVC)
		// and causing crashes if i directly compare with the bilinear strings...WHAT
		else if (strcmp(gl_texturemode->string, "GL_LINEAR_MIPMAP_LINEAR"))
		{
			s_texture_filtering_box->curvalue = 1;
		}
		else
		{
			s_texture_filtering_box->curvalue = 2;
		}

		s_apply_action[i].generic.type = MTYPE_ACTION;
		s_apply_action[i].generic.name = "^5Apply Changes";
		s_apply_action[i].generic.x = 0;
		s_apply_action[i].generic.y = 170 * vid_hudscale->value;
		s_apply_action[i].generic.callback = ApplyChanges;

		s_defaults_action[i].generic.type = MTYPE_ACTION;
		s_defaults_action[i].generic.name = "^5Reset to Defaults";
		s_defaults_action[i].generic.x = 0;
		s_defaults_action[i].generic.y = 180 * vid_hudscale->value;
		s_defaults_action[i].generic.callback = ResetDefaults;

		s_cancel_action[i].generic.type = MTYPE_ACTION;
		s_cancel_action[i].generic.name = "^5Cancel";
		s_cancel_action[i].generic.x = 0;
		s_cancel_action[i].generic.y = 190 * vid_hudscale->value;
		s_cancel_action[i].generic.callback = CancelChanges;
	}

	s_tq_slider.generic.type = MTYPE_SLIDER;
	s_tq_slider.generic.x = 0;
	s_tq_slider.generic.y = 60 * vid_hudscale->value;
	s_tq_slider.generic.name = "^5Texture Quality";
	s_tq_slider.minvalue = 0;
	s_tq_slider.maxvalue = 3;
	s_tq_slider.curvalue = 3 - gl_picmip->value;

	s_vsync_box.generic.type = MTYPE_SPINCONTROL;
	s_vsync_box.generic.x = 0;
	s_vsync_box.generic.y = 80 * vid_hudscale->value;
	s_vsync_box.generic.name = "^5VSync";
	s_vsync_box.curvalue = gl_vsync->value;
	s_vsync_box.itemnames = yes_no_names;

	Menu_AddItem(&s_opengl_menu, (void*)&s_ref_list[OPENGL_MENU]);
	Menu_AddItem(&s_opengl_menu, (void*)&s_mode_list[OPENGL_MENU]);
	Menu_AddItem(&s_opengl_menu, (void*)&s_screensize_slider[OPENGL_MENU]);
	Menu_AddItem(&s_opengl_menu, (void*)&s_brightness_slider[OPENGL_MENU]);
	Menu_AddItem(&s_opengl_menu, (void*)&s_fullscreen_box[OPENGL_MENU]);
	Menu_AddItem(&s_opengl_menu, (void*)&s_fullscreen_dedicated_box[OPENGL_MENU]);
	Menu_AddItem(&s_opengl_menu, (void*)&s_tq_slider);
	Menu_AddItem(&s_opengl_menu, (void*)&s_vsync_box);
	Menu_AddItem(&s_opengl_menu, (void*)&s_texture_filtering_box[OPENGL_MENU]);

	Menu_AddItem(&s_opengl_menu, (void*)&s_apply_action[OPENGL_MENU]);
	Menu_AddItem(&s_opengl_menu, (void*)&s_defaults_action[OPENGL_MENU]);
	Menu_AddItem(&s_opengl_menu, (void*)&s_cancel_action[OPENGL_MENU]);

	Menu_Center(&s_opengl_menu);

	s_opengl_menu.x -= 8 * vid_hudscale->value;
	s_opengl_menu.y += 30 * vid_hudscale->value;
}

/*
================
VID_MenuDraw
================
*/
void Vid_MenuDraw()
{
	int32_t w, h;

	s_current_menu = &s_opengl_menu;

	/*
	** draw the banner
	*/
	re.DrawGetPicSize(&w, &h, "2d/m_banner_video");
	re.DrawPic(r_width->value / 2 - w / 2, r_height->value / 2 - 110 * vid_hudscale->value, "2d/m_banner_video", NULL, false);

	/*
	** move cursor to a reasonable starting position
	*/
	Menu_AdjustCursor(s_current_menu, 1);

	/*
	** draw the menu
	*/
	Menu_Draw(s_current_menu);
}

/*
================
Vid_MenuKey
================
*/
const char* Vid_MenuKey(int32_t key)
{
	menuframework_t* m = s_current_menu;
	static const char* sound = "misc/menu1.wav";

	switch (key)
	{
	case K_ESCAPE:
		CancelChanges(NULL);
		return NULL;
	case K_UPARROW:
		m->cursor--;
		Menu_AdjustCursor(m, -1);
		sound = "misc/menu2.wav";
		break;
	case K_DOWNARROW:
		m->cursor++;
		Menu_AdjustCursor(m, 1);
		sound = "misc/menu2.wav";
		break;
	case K_LEFTARROW:
		Menu_SlideItem(m, -1);
		sound = "misc/menu2.wav";
		break;
	case K_RIGHTARROW:
		Menu_SlideItem(m, 1);
		sound = "misc/menu2.wav";
		break;
	case K_KP_ENTER:
	case K_ENTER:
		if (!Menu_SelectItem(m))
			ApplyChanges(NULL);
		break;
	}

	return sound;
}