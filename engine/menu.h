/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2009 John Fitzgibbons and others

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

//
// the net drivers should just set the apropriate bits in m_activenet,
// instead of having the menu code look through their internal tables
//
#define	MNET_TCP		1

extern	int	m_activenet;

//
// menus
//
void M_Init (void);
void M_Keydown (int key);
void M_Draw (void);
void M_ToggleMenu_f (void);

// Main menu

#define	MAIN_ITEMS	5

#define MENU_MAIN_SINGLEPLAYER			0
#define MENU_MAIN_MULTIPLAYER			1
#define MENU_MAIN_OPTIONS				2
#define MENU_MAIN_DISCONNECT			3
#define MENU_MAIN_QUIT					4

// Options menu (TODO: move everything else here)

#ifdef _WIN32
#define	OPTIONS_ITEMS	16
#else
#define	OPTIONS_ITEMS	15
#endif

#define MENU_OPTIONS_CUSTOMIZE_CONTROLS	0
#define MENU_OPTIONS_GO_TO_CONSOLE		1
#define MENU_OPTIONS_RESET_TO_DEFAULTS	2	
#define MENU_OPTIONS_SCREEN_SIZE		3
#define MENU_OPTIONS_BRIGHTNESS			4
#define MENU_OPTIONS_MOUSE_SPEED		5
#define MENU_OPTIONS_CD_VOLUME			6
#define MENU_OPTIONS_SOUND_VOLUME		7
#define MENU_OPTIONS_ALWAYS_RUN			8
#define MENU_OPTIONS_INVERT_MOUSE		9
#define MENU_OPTIONS_MOUSE_ACCELERATION	10
#define MENU_OPTIONS_MOUSE_FORCE_SPEED	11
#define MENU_OPTIONS_LOOKSPRING			12
#define MENU_OPTIONS_LOOKSTRAFE			13
#define MENU_OPTIONS_VIDEO_OPTIONS		14
#define MENU_OPTIONS_MULTIPLAYER_SETUP	15
#define MENU_OPTIONS_WINDOWED_MOUSE		16
