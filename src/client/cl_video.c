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

// Main windowed and fullscreen graphics interface module. This module
// is used for all of the engine's renderers, and is used in all platforms

#include <assert.h>
#include <float.h>

#include <client/client.h>

// Structure containing functions exported from refresh DLL
refexport_t	re;

// Console variables that we need to access from this module
cvar_t* vid_gamma;
cvar_t* vid_ref;			// Name of Refresh DLL loaded
cvar_t* vid_xpos;			// X coordinate of window position
cvar_t* vid_ypos;			// Y coordinate of window position
cvar_t* vid_borderless;
cvar_t* vid_fullscreen;
cvar_t* vid_refresh;
cvar_t* r_customwidth;
cvar_t* r_customheight;
cvar_t* viewsize;

// Global variables used internally by this module
viddef_t	viddef;				// global video state; used by other modules
HINSTANCE	reflib_library;		// Handle to refresh DLL 
bool		reflib_active = 0;

#define VID_NUM_MODES ( sizeof( vid_modes ) / sizeof( vid_modes[0] ) )

static bool s_alttab_disabled;

extern	uint32_t	sys_msg_time;

/*
==========================================================================

DLL GLUE

==========================================================================
*/

#define	MAXPRINTMSG	8192
void Vid_Printf(int32_t print_level, char* fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];
	static bool	inupdate;

	va_start(argptr, fmt);
	vsnprintf(msg, MAXPRINTMSG, fmt, argptr);
	va_end(argptr);

	if (print_level == PRINT_ALL)
	{
		Com_Printf("%s", msg);
	}
	else if (print_level == PRINT_DEVELOPER)
	{
		Com_DPrintf("%s", msg);
	}
	else if (print_level == PRINT_ALERT)
	{
		// ICONWARNING
		Sys_Msgbox("PRINT_ALERT", 30, msg);
	}
}

void Vid_Error(int32_t err_level, char* fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];
	static bool	inupdate;

	va_start(argptr, fmt);
	vsnprintf(msg, MAXPRINTMSG, fmt, argptr);
	va_end(argptr);

	Com_Error(err_level, "%s", msg);
}

/*
============
VID_Restart_f

Console command to re-start the video mode and refresh DLL. We do this
simply by setting the modified flag for the vid_ref variable, which will
cause the entire video mode and refresh DLL to be reset on the next frame.
============
*/
void Vid_Restart_f()
{
	vid_ref->modified = true;
}

/*
** VID_GetModeInfo
*/
typedef struct vidmode_s
{
	const char* description;
	int32_t         width, height;
	int32_t         mode;
} vidmode_t;

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
	{ "Mode 10: 1440x900", 1440, 900,  10},
	{ "Mode 11: 1600x900", 1600, 900,  11},
	{ "Mode 12: 1600x1200", 1600, 1200, 11 },
	{ "Mode 13: 1920x1080", 1920, 1080, 12 },
	{ "Mode 14: 1920x1200", 1920, 1200, 13 },
	{ "Mode 15: 2048x1536", 2048, 1536, 14 },
	{ "Mode 16: 2560x1440", 2560, 1440, 15 },
	{ "Mode 17: 3840x2160", 3840, 2160, 16 },
};

bool Vid_GetModeInfo(int32_t* width, int32_t* height, int32_t mode)
{
	if (mode == -1) // custom mode (using r_customwidth and r_customheight)
	{
		*width = r_customwidth->value;
		*height = r_customheight->value;
		return true;
	}

	if (mode < 0 || mode >= VID_NUM_MODES)
		return false;

	*width = vid_modes[mode].width;
	*height = vid_modes[mode].height;

	return true;
}

/*
** VID_ChangeResolution
*/
void Vid_ChangeResolution(int32_t width, int32_t height)
{
	int32_t old_width = viddef.width;
	int32_t old_height = viddef.height;

	viddef.width = width;
	viddef.height = height;

	cl.force_refdef = true;		// can't use a paused refdef

	char hudscale[5];
	memset(hudscale, 0, sizeof(hudscale));

	float wscale = (float)viddef.width / UI_SCALE_BASE_X;
	float hscale = (float)viddef.height / UI_SCALE_BASE_Y;

	if (wscale > hscale) wscale = hscale;
	if (wscale < 1) wscale = 1;

	snprintf(hudscale, 4, "%2f", wscale);
	vid_hudscale = Cvar_Set("hudscale", hudscale);
}

void VID_FreeReflib()
{
	if (!FreeLibrary(reflib_library))
		Com_Error(ERR_FATAL, "Reflib FreeLibrary failed");
	memset(&re, 0, sizeof(re));
	reflib_library = NULL;
	reflib_active = false;
}

/*
==============
VID_LoadRefresh
==============
*/
bool Vid_LoadRefresh(char* name)
{
	refimport_t	ri = { 0 };
	GetRefAPI_t	GetRefAPI;

	if (reflib_active)
	{
		re.Shutdown();
		VID_FreeReflib();
	}

	Com_Printf("------- Loading %s -------\n", name);

	if ((reflib_library = LoadLibrary(name)) == 0)
	{
		Com_Printf("LoadLibrary(\"%s\") failed\n", name);

		return false;
	}

	ri.Cmd_AddCommand = Cmd_AddCommand;
	ri.Cmd_RemoveCommand = Cmd_RemoveCommand;
	ri.Cmd_Argc = Cmd_Argc;
	ri.Cmd_Argv = Cmd_Argv;
	ri.Cmd_ExecuteText = Cbuf_ExecuteText;
	ri.Con_Printf = Vid_Printf;
	ri.Sys_Error = Vid_Error;
	ri.FS_LoadFile = FS_LoadFile;
	ri.FS_FreeFile = FS_FreeFile;
	ri.FS_Gamedir = FS_Gamedir;
	ri.Cvar_Get = Cvar_Get;
	ri.Cvar_Set = Cvar_Set;
	ri.Cvar_SetValue = Cvar_SetValue;
	ri.Vid_GetModeInfo = Vid_GetModeInfo;
	ri.Vid_MenuInit = Vid_MenuInit;
	ri.Vid_ChangeResolution = Vid_ChangeResolution;

	if ((GetRefAPI = (void*)GetProcAddress(reflib_library, "GetRefAPI")) == 0)
		Com_Error(ERR_FATAL, "GetProcAddress failed on %s", name);

	re = GetRefAPI(ri);

	if (re.api_version != API_VERSION)
	{
		VID_FreeReflib();
		Com_Error(ERR_FATAL, "%s has incompatible api_version", name);
	}

	if (re.Init() == false)
	{
		re.Shutdown();
		VID_FreeReflib();
		return false;
	}

	Com_Printf("------------------------------------\n");
	reflib_active = true;

	vidref_val = VIDREF_OTHER;
	if (vid_ref)
	{
		if (!strcmp(vid_ref->string, "gl"))
			vidref_val = VIDREF_GL;
	}

	// set up the event procs
	re.SetKeyPressedProc(Key_Event);
	re.SetMousePressedProc(MouseClick_Event);
	re.SetMouseMovedProc(MouseMove_Event);
	re.SetMouseScrollProc(MouseScroll_Event);
	re.SetWindowFocusProc(WindowFocus_Event);
	re.SetWindowIconifyProc(WindowIconify_Event);
	return true;
}

/*
============
VID_CheckChanges

This function gets called once just before drawing each frame, and it's sole purpose in life
is to check to see if any of the video mode parameters have changed, and if they have to
update the rendering DLL and/or video mode to match.
============
*/
void Vid_CheckChanges()
{
	char name[100];

	if (vid_ref->modified)
	{
		cl.force_refdef = true;		// can't use a paused refdef
		S_StopAllSounds();
	}

	while (vid_ref->modified)
	{
		/*
		** refresh has changed
		*/
		vid_ref->modified = false;
		vid_borderless->modified = true;
		vid_fullscreen->modified = true;
		cl.refresh_prepped = false;
		cls.disable_screen = true;

		Com_sprintf(name, sizeof(name), "ref_%s.dll", vid_ref->string);
		if (!Vid_LoadRefresh(name))
		{
			Com_Error(ERR_FATAL, "Failed to initialise renderer. Renderer name: %s", vid_ref->string);

			/*
			** drop the console if we fail to load a refresh
			*/
			if (cls.key_dest != key_console)
			{
				Con_ToggleConsole_f();
			}
		}

		// turn off the cursor if we are switching into fullscreen mode
		if (vid_fullscreen->value
			|| vid_borderless->value)
		{
			re.EnableCursor(false);
		}

		cls.disable_screen = false;
	}

	/*
	** update our window position
	*/
	if (vid_xpos->modified || vid_ypos->modified)
	{
		if (!vid_borderless->value
			&& !vid_fullscreen->value)
		{
			re.SetWindowPosition(vid_xpos->value, vid_ypos->value);
		}

		vid_xpos->modified = false;
		vid_ypos->modified = false;
	}

	if (vid_refresh->modified)
	{
		vid_refresh->modified = false;
		cl.refresh_prepped = false;
	}
}

/*
============
VID_Init
============
*/
void Vid_Init()
{
	/* Create the video variables so we know how to start the graphics drivers */
	vid_ref = Cvar_Get("vid_ref", "gl", CVAR_ARCHIVE);
	vid_xpos = Cvar_Get("vid_xpos", "3", CVAR_ARCHIVE);
	vid_ypos = Cvar_Get("vid_ypos", "22", CVAR_ARCHIVE);
	vid_borderless = Cvar_Get("vid_borderless", "0", CVAR_ARCHIVE);
	vid_fullscreen = Cvar_Get("vid_fullscreen", "0", CVAR_ARCHIVE);
	vid_refresh = Cvar_Get("vid_refresh", "0", CVAR_NOSET);
	vid_gamma = Cvar_Get("vid_gamma", "1", CVAR_ARCHIVE);
	r_customwidth = Cvar_Get("r_customwidth", "1024", CVAR_ARCHIVE);
	r_customheight = Cvar_Get("r_customheight", "768", CVAR_ARCHIVE);
	viewsize = Cvar_Get("viewsize", "100", CVAR_ARCHIVE);

	/* Add some console commands that we want to handle */
	Cmd_AddCommand("vid_restart", Vid_Restart_f);

	/* Start the graphics mode and load refresh DLL */
	Vid_CheckChanges();
}

/*
============
VID_Shutdown
============
*/
void Vid_Shutdown()
{
	if (reflib_active)
	{
		re.Shutdown();
		VID_FreeReflib();
	}
}