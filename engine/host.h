/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2009 John Fitzgibbons and others
Copyright (C) 2023      starfrost

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
//=============================================================================
// host.h - the host system specifies the base of the directory tree, the
// command line parms passed to the program, and the amount of memory
// available for the program to use
//=============================================================================

extern qboolean noclip_anglehack;

typedef struct quakeparms_s
{
	char* basedir;
	char* cachedir;		// for development over ISDN lines
	int		argc;
	char** argv;
	void* membase;
	int		memsize;
} quakeparms_t;

extern	quakeparms_t host_parms;

extern	cvar_t		sys_ticrate;
extern	cvar_t		sys_nostdout;
extern	cvar_t		developer;
extern	cvar_t		max_edicts; //johnfitz

extern	qboolean	host_initialized;		// true if into command execution
extern	double		host_frametime;
extern	byte* host_colormap;
extern	int			host_framecount;	// incremented every frame, never reset
extern	double		realtime;			// not bounded in any way, changed at
// start of every frame, never reset

void Host_ClearMemory(void);
void Host_ServerFrame(void);
void Host_InitCommands(void);
void Host_Init(quakeparms_t* parms);
void Host_Shutdown(void);
void Host_Error(char* error, ...);
void Host_EndGame(char* message, ...);
void Host_Frame(float time);
void Host_Version_f(void);
void Host_Quit_f(void);
void Host_ClientCommands(char* fmt, ...);
void Host_ShutdownServer(qboolean crash);

extern qboolean		msg_suppress_1;		// suppresses resolution and cache size console output
//  an fullscreen DIB focus gain/loss
extern int			current_skill;		// skill level for currently loaded level (in case
//  the user changes the cvar while the level is
//  running, this reflects the level actually in use)

extern qboolean		isDedicated;

extern int			minimum_memory;
