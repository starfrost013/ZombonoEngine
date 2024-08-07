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
// sys_win.c: Windows specific code

#include <common/common.h>
#include <client/client.h>
#include "winquake.h"
#include "resource.h"
#include <errno.h>
#include <float.h>
#include <fcntl.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>
#include <conio.h>
#include <win32/conproc.h>

int32_t 		starttime;

static HANDLE	hinput, houtput;

uint32_t		sys_msg_time;

#define	MAX_NUM_ARGVS	128
int32_t 		argc;
char* argv[MAX_NUM_ARGVS];

int32_t Sys_MsgboxV(char* title, uint32_t buttons, char* text, va_list args);


/*
===============================================================================

SYSTEM IO

===============================================================================
*/

#ifdef _MSC_VER
// this gets rid of RetardCompiler warnings
__declspec(noreturn) void Sys_Error(char* error, ...);
#else
__attribute__((noreturn)) void Sys_Error(char* error, ...);
#endif
void Sys_Error(char* error, ...)
{
	CL_Shutdown();
	Common_Shutdown();

	va_list args;

	va_start(args, error);

	Sys_MsgboxV("Fatal Error", MB_OK, error, args);
	va_end(args);

	// shut down QHOST hooks if necessary 
	DeinitConProc();

	exit(1);
}

// returns a value indicating which buttons were pressed
int32_t Sys_Msgbox(char* title, uint32_t buttons, char* text, ...)
{
	va_list		args;

	char text_processed[1024] = { 0 };

	va_start(args, text_processed);

	vsnprintf(text_processed, 1024, text, args);
	va_end(args);

	char* title_processed_ptr = title;
	char* text_processed_ptr = text_processed;

	if (localisation_initialised)
	{
		title_processed_ptr = Localisation_ProcessString(title);
		text_processed_ptr = Localisation_ProcessString(text);
	}

	return MessageBox(NULL, text_processed_ptr, title_processed_ptr, buttons);
}

int32_t Sys_MsgboxV(char* title, uint32_t buttons, char* text, va_list args)
{
	char text_processed[1024] = { 0 };

	vsnprintf(text_processed, 1024, text, args);

	return MessageBox(NULL, &text_processed, title, buttons);
}

void Sys_Quit()
{
	timeEndPeriod(1);

	CL_Shutdown();
	Netservices_Shutdown();
	Common_Shutdown();
	if (dedicated && dedicated->value)
		FreeConsole();
	else if (debug_console->value)
		FreeConsole();

	// shut down QHOST hooks if necessary
	DeinitConProc();

	exit(0);
}
/*
================
Sys_SetDPIAwareness

================
*/
typedef enum { dpi_unaware = 0, dpi_system_aware = 1, dpi_monitor_aware = 2 } dpi_awareness;
typedef BOOL(WINAPI* SetProcessDPIAwareFunc)();
typedef HRESULT(WINAPI* SetProcessDPIAwarenessFunc)(dpi_awareness value);

void	Sys_SetDPIAwareness()
{
	HMODULE hShcore = LoadLibraryA("Shcore.dll");
	HMODULE hUser32 = LoadLibraryA("user32.dll");
	SetProcessDPIAwarenessFunc setProcDPIAwareness = (SetProcessDPIAwarenessFunc)(hShcore ? GetProcAddress(hShcore, "SetProcessDpiAwareness") : NULL);
	SetProcessDPIAwareFunc setProcDPIAware = (SetProcessDPIAwareFunc)(hUser32 ? GetProcAddress(hUser32, "SetProcessDPIAware") : NULL);

	if (setProcDPIAwareness) /* Windows 8.1+ */
		setProcDPIAwareness(dpi_monitor_aware);
	else if (setProcDPIAware) /* Windows Vista-8.0 */
		setProcDPIAware();

	if (hShcore)
		FreeLibrary(hShcore);
	if (hUser32)
		FreeLibrary(hUser32);
}

//================================================================


/*
================
Sys_Init
================
*/
void Sys_Init()
{
	OSVERSIONINFO	vinfo;

	timeBeginPeriod(1);

	vinfo.dwOSVersionInfoSize = sizeof(vinfo);

	if (!GetVersionEx(&vinfo))
		Sys_Error("Couldn't get OS info");

	// VS2022 requires Windows 7
	// GetVersionEx fuckery didn't start until Windows 8.1 so it's safe
	if (vinfo.dwMajorVersion < 6
		|| (vinfo.dwMajorVersion == 6 && vinfo.dwMinorVersion < 1))
	{
		// TODO: LOCALISE THIS TEXT
		Sys_Error("%s requires Windows 7 or later!", gameinfo.name);
	}

	if (dedicated->value)
	{
		if (!AllocConsole())
			Sys_Error("Couldn't create dedicated server console");

		hinput = GetStdHandle(STD_INPUT_HANDLE);
		houtput = GetStdHandle(STD_OUTPUT_HANDLE);

		// let QHOST hook in
		InitConProc(argc, argv);
	}
	else if (debug_console->value)
	{
		AllocConsole();
		SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), 0x07);
		DeleteMenu(GetSystemMenu(GetConsoleWindow(), false), SC_CLOSE, MF_BYCOMMAND);
		freopen("CONOUT$", "w", stderr);
	}

	// enable DPI awareness
	Sys_SetDPIAwareness();
}


static char	console_text[256];
static int32_t console_textlen;

/*
================
Sys_ConsoleInput
================
*/
char* Sys_ConsoleInput()
{
	INPUT_RECORD	recs[1024];
	int32_t 	dummy;
	int32_t 	ch, numread, numevents;

	if (!dedicated || !dedicated->value)
		return NULL;


	for (;; )
	{
		if (!GetNumberOfConsoleInputEvents(hinput, &numevents))
			Sys_Error("Error getting # of console events");

		if (numevents <= 0)
			break;

		if (!ReadConsoleInput(hinput, recs, 1, &numread))
			Sys_Error("Error reading console input");

		if (numread != 1)
			Sys_Error("Couldn't read console input");

		if (recs[0].EventType == KEY_EVENT)
		{
			if (!recs[0].Event.KeyEvent.bKeyDown)
			{
				ch = recs[0].Event.KeyEvent.uChar.AsciiChar;

				switch (ch)
				{
				case '\r':
					WriteFile(houtput, "\r\n", 2, &dummy, NULL);

					if (console_textlen)
					{
						console_text[console_textlen] = 0;
						console_textlen = 0;
						return console_text;
					}
					break;

				case '\b':
					if (console_textlen)
					{
						console_textlen--;
						WriteFile(houtput, "\b \b", 3, &dummy, NULL);
					}
					break;

				default:
					if (ch >= ' ')
					{
						if (console_textlen < sizeof(console_text) - 2)
						{
							WriteFile(houtput, &ch, 1, &dummy, NULL);
							console_text[console_textlen] = ch;
							console_textlen++;
						}
					}

					break;

				}
			}
		}
	}

	return NULL;
}


/*
================
Sys_ConsoleOutput

Print text to the dedicated console
================
*/
void Sys_ConsoleOutput(char* string)
{
	int32_t 	dummy;
	char	text[256];

	if (!dedicated || !dedicated->value)
	{
		if (debug_console && debug_console->value)
		{
			fputs(string, stderr);
			OutputDebugString(string);
		}
		return;
	}

	if (console_textlen)
	{
		text[0] = '\r';
		memset(&text[1], ' ', console_textlen);
		text[console_textlen + 1] = '\r';
		text[console_textlen + 2] = 0;
		WriteFile(houtput, text, console_textlen + 2, &dummy, NULL);
	}

	WriteFile(houtput, string, (DWORD)strlen(string), &dummy, NULL);

	if (console_textlen)
		WriteFile(houtput, console_text, console_textlen, &dummy, NULL);
}

/*
================
Sys_GetClipboardData

================
*/
char* Sys_GetClipboardData(void)
{
	char* data = NULL;
	char* cliptext;

	if (OpenClipboard(NULL) != 0)
	{
		HANDLE hClipboardData;

		if ((hClipboardData = GetClipboardData(CF_TEXT)) != 0)
		{
			if ((cliptext = GlobalLock(hClipboardData)) != 0)
			{
				data = malloc(GlobalSize(hClipboardData) + 1);
				strcpy(data, cliptext);
				GlobalUnlock(hClipboardData);
			}
		}
		CloseClipboard();
	}
	return data;
}


/*
========================================================================

GAME DLL

========================================================================
*/

static HINSTANCE	game_library;

/*
=================
Sys_UnloadGame
=================
*/
void Sys_UnloadGame()
{
	if (!FreeLibrary(game_library))
		Com_Error(ERR_FATAL, "FreeLibrary failed for game library");
	game_library = NULL;
}

/*
=================
Sys_GetGameAPI

Loads the game dll
=================
*/
void* Sys_GetGameAPI(void* parms)
{
	void* (*GetGameAPI) (void*);
	char	name[MAX_OSPATH];
	char* path;
	char	cwd[MAX_OSPATH];
#if defined _M_IX86
	const char* gamename = "gamex86.dll";
#ifdef NDEBUG
	const char* debugdir = "release";
#else
	const char* debugdir = "debug";
#endif

#elif defined _M_X64
	const char* gamename = "gamex64.dll";

#ifdef NDEBUG
	const char* debugdir = "releasex64";
#else
	const char* debugdir = "debugx64";
#endif

#endif

	if (game_library)
		Com_Error(ERR_FATAL, "Sys_GetGameAPI without Sys_UnloadingGame");

	// check the current debug directory first for development purposes
	_getcwd(cwd, sizeof(cwd));
	Com_sprintf(name, sizeof(name), "%s/%s/%s", cwd, debugdir, gamename);
	game_library = LoadLibrary(name);
	if (game_library)
	{
		Com_DPrintf("LoadLibrary (%s)\n", name);
	}
	else
	{
#ifdef DEBUG
		// check the current directory for other development purposes
		Com_sprintf(name, sizeof(name), "%s/%s", cwd, gamename);
		game_library = LoadLibrary(name);
		if (game_library)
		{
			Com_DPrintf("LoadLibrary (%s)\n", name);
		}
		else
#endif
		{
			// now run through the search paths
			path = NULL;
			while (1)
			{
				path = FS_NextPath(path);
				if (!path)
					return NULL;		// couldn't find one anywhere
				Com_sprintf(name, sizeof(name), "%s/%s", path, gamename);
				game_library = LoadLibrary(name);
				if (game_library)
				{
					Com_DPrintf("LoadLibrary (%s)\n", name);
					break;
				}
			}
		}
	}

	GetGameAPI = (void*)GetProcAddress(game_library, "GetGameAPI");
	if (!GetGameAPI)
	{
		Sys_UnloadGame();
		return NULL;
	}

	return GetGameAPI(parms);
}

//=======================================================================


/*
==================
ParseCommandLine

==================
*/
void ParseCommandLine(LPSTR lpCmdLine)
{
	argc = 1;
	argv[0] = "exe";

	while (*lpCmdLine && (argc < MAX_NUM_ARGVS))
	{
		while (*lpCmdLine && ((*lpCmdLine <= 32) || (*lpCmdLine > 126)))
			lpCmdLine++;

		if (*lpCmdLine)
		{
			argv[argc] = lpCmdLine;
			argc++;

			while (*lpCmdLine && ((*lpCmdLine > 32) && (*lpCmdLine <= 126)))
				lpCmdLine++;

			if (*lpCmdLine)
			{
				*lpCmdLine = 0;
				lpCmdLine++;
			}

		}
	}

}

/*
==================
WinMain

The entry point on Windows platform
==================
*/

int32_t WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int32_t nCmdShow)
{
	int32_t time, oldtime, newtime;

	ParseCommandLine(lpCmdLine);

	Common_Init(argc, argv);
	oldtime = Sys_Milliseconds();

	/* main window message loop */
	while (1)
	{
		// if at a full screen console, don't update unless needed
		if ((dedicated && dedicated->value))
		{
			Sleep(1);
		}

		do
		{
			newtime = Sys_Milliseconds();
			time = newtime - oldtime;
		} 
		while (time < 1);

		// always run on dedicated, only run if the app is active otherwise
#ifndef DEDICATED_ONLY
		if (app_active
			|| dedicated->value

			|| (!dedicated->value && (cls.state == ca_connected
				|| cls.state == ca_active)))
#else
		if (app_active
			|| (dedicated && dedicated->value))
#endif
		{
			Common_Frame(time);
		}


		oldtime = newtime;
	}

	// never gets here
	return TRUE;
}
