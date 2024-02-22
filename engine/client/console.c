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
// console.c

#include "client.h"

console_t	con;

cvar_t		*con_notifytime;
extern cvar_t	*vid_hudscale;


#define		MAXCMDLINE	256
extern	char	key_lines[128][MAXCMDLINE];
extern	int		edit_line;
extern	int		key_linepos;

void Draw_String (int x, int y, char *s)
{
	while (*s)
	{
		re.DrawChar (x, y, *s);
		x+=8*vid_hudscale->value;
		s++;
	}
}

void Draw_StringAlt (int x, int y, char *s)
{
	while (*s)
	{
		re.DrawChar (x, y, *s ^ 0x80);
		x+=8*vid_hudscale->value;
		s++;
	}
}


void Key_ClearTyping (void)
{
	key_lines[edit_line][1] = 0;	// clear any typing
	key_linepos = 1;
}

/*
================
Con_ToggleConsole_f
================
*/
void Con_ToggleConsole_f (void)
{
	SCR_EndLoadingPlaque ();	// get rid of loading plaque

	if (cl.attractloop)
	{
		Cbuf_AddText ("killserver\n");
		return;
	}

	if (cls.state == ca_disconnected)
	{	// start the demo loop again
		Cbuf_AddText ("d1\n");
		return;
	}

	Key_ClearTyping ();
	Con_ClearNotify ();

	if (cls.key_dest == key_console)
	{
		M_ForceMenuOff ();
		Cvar_Set ("paused", "0");
	}
	else
	{
		M_ForceMenuOff ();
		cls.key_dest = key_console;	

		if (Cvar_VariableValue ("maxclients") == 1 
			&& Com_ServerState ())
			Cvar_Set ("paused", "1");
	}
}

/*
================
Con_ToggleChat_f
================
*/
void Con_ToggleChat_f (void)
{
	Key_ClearTyping ();

	if (cls.key_dest == key_console)
	{
		if (cls.state == ca_active)
		{
			M_ForceMenuOff ();
			cls.key_dest = key_game;
		}
	}
	else
		cls.key_dest = key_console;
	
	Con_ClearNotify ();
}

/*
================
Con_Clear_f
================
*/
void Con_Clear_f (void)
{
	memset (con.text, ' ', CON_TEXTSIZE);
}

						
/*
================
Con_Dump_f

Save the console contents out to a file
================
*/
void Con_Dump_f (void)
{
	int		l, x;
	char	*line;
	FILE	*f;
	char	buffer[1024];
	char	name[MAX_OSPATH];

	if (Cmd_Argc() != 2)
	{
		Com_Printf ("usage: condump <filename>\n");
		return;
	}

	Com_sprintf (name, sizeof(name), "%s/%s.txt", FS_Gamedir(), Cmd_Argv(1));

	Com_Printf ("Dumped console text to %s.\n", name);
	FS_CreatePath (name);
	f = fopen (name, "w");
	if (!f)
	{
		Com_Printf ("ERROR: couldn't open.\n");
		return;
	}

	// skip empty lines
	for (l = con.current - con.totallines + 1 ; l <= con.current ; l++)
	{
		line = con.text + (l%con.totallines)*con.linewidth;
		for (x=0 ; x<con.linewidth ; x++)
			if (line[x] != ' ')
				break;
		if (x != con.linewidth)
			break;
	}

	// write the remaining lines
	buffer[con.linewidth] = 0;
	for ( ; l <= con.current ; l++)
	{
		line = con.text + (l%con.totallines)*con.linewidth;
		strncpy (buffer, line, con.linewidth);
		for (x=con.linewidth-1 ; x>=0 ; x--)
		{
			if (buffer[x] == ' ')
				buffer[x] = 0;
			else
				break;
		}
		for (x=0; buffer[x]; x++)
			buffer[x] &= 0x7f;

		fprintf (f, "%s\n", buffer);
	}

	fclose (f);
}

						
/*
================
Con_ClearNotify
================
*/
void Con_ClearNotify (void)
{
	int		i;
	
	for (i=0 ; i<NUM_CON_CHAT_LINES ; i++)
		con.times[i] = 0;
}

						
/*
================
Con_MessageMode_f
================
*/
void Con_MessageMode_f (void)
{
	chat_team = false;
	cls.key_dest = key_message;
}

/*
================
Con_MessageMode2_f
================
*/
void Con_MessageMode2_f (void)
{
	chat_team = true;
	cls.key_dest = key_message;
}

/*
================
Con_CheckResize

If the line width has changed, reformat the buffer.
================
*/
void Con_CheckResize (void)
{
	int		i, j, width, oldwidth, oldtotallines, numlines, numchars;
	char	tbuf[CON_TEXTSIZE];

	width = (viddef.width >> 3) - 2;

	if (width == con.linewidth)
		return;

	if (width < 1)			// video hasn't been initialized yet
	{
		width = 38;
		con.linewidth = width;
		con.totallines = CON_TEXTSIZE / con.linewidth;
		memset (con.text, ' ', CON_TEXTSIZE);
	}
	else
	{
		oldwidth = con.linewidth;
		con.linewidth = width;
		oldtotallines = con.totallines;
		con.totallines = CON_TEXTSIZE / con.linewidth;
		numlines = oldtotallines;

		if (con.totallines < numlines)
			numlines = con.totallines;

		numchars = oldwidth;
	
		if (con.linewidth < numchars)
			numchars = con.linewidth;

		memcpy (tbuf, con.text, CON_TEXTSIZE);
		memset (con.text, ' ', CON_TEXTSIZE);

		for (i=0 ; i<numlines ; i++)
		{
			for (j=0 ; j<numchars ; j++)
			{
				con.text[(con.totallines - 1 - i) * con.linewidth + j] =
						tbuf[((con.current - i + oldtotallines) %
							  oldtotallines) * oldwidth + j];
			}
		}

		Con_ClearNotify ();
	}

	con.current = con.totallines - 1;
	con.display = con.current;
}


/*
================
Con_Init
================
*/
void Con_Init (void)
{
	con.linewidth = -1;

	Con_CheckResize ();
	
	Com_Printf ("Console initialized.\n");

	Com_Printf("Zombono © 2023-2024 starfrost.\n");
	Com_Printf("ALPHA RELEASE\n");

//
// register our commands
//
	con_notifytime = Cvar_Get ("con_notifytime", "3", 0);

	Cmd_AddCommand ("toggleconsole", Con_ToggleConsole_f);
	Cmd_AddCommand ("togglechat", Con_ToggleChat_f);
	Cmd_AddCommand ("messagemode", Con_MessageMode_f);
	Cmd_AddCommand ("messagemode2", Con_MessageMode2_f);
	Cmd_AddCommand ("clear", Con_Clear_f);
	Cmd_AddCommand ("condump", Con_Dump_f);
	con.initialized = true;
}


/*
===============
Con_Linefeed
===============
*/
void Con_Linefeed (void)
{
	con.x = 0;
	if (con.display == con.current)
		con.display++;
	con.current++;
	memset (&con.text[(con.current%con.totallines)*con.linewidth]
	, ' ', con.linewidth);
}

/*
================
Con_Print

Handles cursor positioning, line wrapping, etc
All console printing must go through this in order to be logged to disk
If no console is visible, the text will appear at the top of the game window
================
*/
void Con_Print (char *txt)
{
	int		y;
	int		c, l;
	static int	cr;
	int		mask;

	if (!con.initialized)
		return;

	if (txt[0] == 1 || txt[0] == 2)
	{
		mask = 128;		// go to colored text
		txt++;
	}
	else
		mask = 0;


	while ( (c = *txt) )
	{
	// count word length
		for (l=0 ; l< con.linewidth ; l++)
			if ( txt[l] <= ' ')
				break;

	// word wrap
		if (l != con.linewidth && (con.x + l > con.linewidth) )
			con.x = 0;

		txt++;

		if (cr)
		{
			con.current--;
			cr = false;
		}

		
		if (!con.x)
		{
			Con_Linefeed ();
		// mark time for transparent overlay
			if (con.current >= 0)
				con.times[con.current % NUM_CON_CHAT_LINES] = cls.realtime;
		}

		switch (c)
		{
		case '\n':
			con.x = 0;
			break;

		case '\r':
			con.x = 0;
			cr = 1;
			break;

		default:	// display character and advance
			y = con.current % con.totallines;
			con.text[y*con.linewidth+con.x] = c | mask | con.ormask;
			con.x++;
			if (con.x >= con.linewidth)
				con.x = 0;
			break;
		}
		
	}
}


/*
==============
Con_CenteredPrint
==============
*/
void Con_CenteredPrint (char *text)
{
	int		l;
	char	buffer[1024];

	l = (int)strlen(text);
	l = (con.linewidth-l)/2;
	if (l < 0)
		l = 0;
	memset (buffer, ' ', l);
	strcpy (buffer+l, text);
	strcat (buffer, "\n");
	Con_Print (buffer);
}

/*
==============================================================================

DRAWING

==============================================================================
*/


/*
================
Con_DrawInput

The input line scrolls horizontally if typing goes beyond the right edge
================
*/
void Con_DrawInput (void)
{
	int		y;
	int		i;
	char	*text;
	int		size_x = 0, size_y = 0;
	font_t*	system_font_ptr = Font_GetByName(cl_system_font->string); // checked by drawconsole

	if (cls.key_dest == key_menu)
		return;
	if (cls.key_dest != key_console && cls.state == ca_active)
		return;		// don't draw anything (always draw if not active)

	text = key_lines[edit_line];
	
// add the cursor frame
	text[key_linepos] = 10+((int)(cls.realtime>>8)&1);
	
// fill out remainder with spaces
	for (i=key_linepos+1 ; i< con.linewidth ; i++)
		text[i] = ' ';
		
//	prestep if horizontally scrolling
	if (key_linepos >= con.linewidth)
		text += 1 + key_linepos - con.linewidth;
		
// draw it
	y = con.vislines-16;

	// temp code
	Text_Draw(cl_system_font->string, 8*vid_hudscale->value, con.vislines - system_font_ptr->line_height * vid_hudscale->value, text);

// remove cursor
	key_lines[edit_line][key_linepos] = 0;
}


/*
================
Con_DrawNotify

Draws the last few lines of output transparently over the game top
================
*/
void Con_DrawNotify (void)
{
	int		x, v = 0;
	int		skip_size_x = 0, skip_size_y = 0;
	char	*text;
	int		i;
	int		time;
	char	*s;
	font_t* system_font_ptr = Font_GetByName(cl_system_font->string);

	for (i = con.current-NUM_CON_CHAT_LINES+1 ; i <= con.current ; i++)
	{
		if (i < 0)
			continue;
		time = con.times[i % NUM_CON_CHAT_LINES];
		if (time == 0)
			continue;
		time = cls.realtime - time;
		if (time > con_notifytime->value*1000)
			continue;
		text = con.text + (i % con.totallines)*con.linewidth;
		
		// temporary set a text terminator for the text system
		// TODO: does this overflow if we print all the 128kb.
		char temp = text[con.linewidth];
		text[con.linewidth] = '\0';
		Text_Draw(cl_system_font->string, 8 * vid_hudscale->value, v * vid_hudscale->value, text);
		text[con.linewidth] = temp;

		v += system_font_ptr->line_height;
	}

	if (cls.key_dest == key_message)
	{
		if (chat_team)
		{
			const char* say_text = "Chat (Team):";
			Text_GetSize(cl_system_font->string, &skip_size_x, &skip_size_y, say_text);
			Text_Draw (cl_system_font->string, 8, v*vid_hudscale->value, "Chat (Team):");
		}
		else
		{
			const char* say_text = "Chat:";
			Text_GetSize(cl_system_font->string, &skip_size_x, &skip_size_y, say_text);
			Text_Draw (cl_system_font->string, 8, v*vid_hudscale->value, "Chat:");
		}

		s = chat_buffer;
		if (chat_bufferlen > (viddef.width>>3)-(skip_size_x+1))
			s += chat_bufferlen - ((viddef.width>>3)-(skip_size_x+1));

		// terminator for text engine
		char original = s[chat_bufferlen];
		s[chat_bufferlen] = '\0';
		Text_Draw(cl_system_font->string, 8 + (skip_size_x) + system_font_ptr->size / 2, v, s);

		// wtf does this do? it draws a newline or vertical tab depending on if its realtime?
		Text_DrawChar (cl_system_font->string, 8 * skip_size_x * vid_hudscale->value, v*vid_hudscale->value, 10+((cls.realtime>>8)&1));
		v += system_font_ptr->line_height;
		s[chat_bufferlen] = original;
	}
	
	if (v)
	{
		SCR_AddDirtyPoint (0,0);
		SCR_AddDirtyPoint (viddef.width-1, v*vid_hudscale->value);
	}
}

/*
================
Con_DrawConsole

Draws the console with the solid background
================
*/
void Con_DrawConsole (float frac)
{
	int				i, j, x, y, n;
	int				rows;
	char			*text;
	int				row;
	int				lines;
	char			version[64];
	char			dlbar[1024];
	int				size_x = 0, size_y = 0;
	font_t*			system_font_ptr;

	system_font_ptr = Font_GetByName(cl_system_font->string);

	// we already check it can't be null and exit earlier, but the user can change the cvar
	if (!system_font_ptr)
	{
		Com_Printf("You changed the system font to an invalid value (%s). Trying the default...", cl_system_font->string);
		Cvar_Set("cl_system_font", "bahnschrift_bold_8");

		system_font_ptr = Font_GetByName(cl_system_font->string);

		// we tried again with the default but it failed
		if (!system_font_ptr)
		{
			// just go down
			Sys_Error("You changed the system font to an invalid value, and then deleted the default system font. You deserve this :(");
			return;
		}
	}

	lines = viddef.height * frac;
	if (lines <= 0)
		return;

	if (lines > viddef.height)
		lines = viddef.height;

// draw the background (draw widescreen 457*240 background if 16:9 res)
	if ((float)viddef.width / (float)viddef.height > 1.4f)
	{
		re.DrawStretchPic(0, lines - viddef.height, viddef.width, viddef.height, "pics/conback_16x9");
	}
	else
	{
		re.DrawStretchPic(0, lines - viddef.height, viddef.width, viddef.height, "pics/conback");
	}

	SCR_AddDirtyPoint (0,0);
	SCR_AddDirtyPoint (viddef.width-1,lines-1);

	Com_sprintf (version, sizeof(version), "^2Zombono v%s", ZOMBONO_VERSION);
	Text_GetSize(cl_system_font->string, &size_x, &size_y, version);
	Text_Draw(cl_system_font->string, viddef.width - size_x, lines - 12 * vid_hudscale->value, version);

// draw the text
	con.vislines = lines;

	rows = (lines-22)>>3;		// rows of text to draw
	y = (lines - 30 * vid_hudscale->value)/vid_hudscale->value;

// draw from the bottom up
	if (con.display != con.current)
	{
	// draw arrows to show the buffer is backscrolled
		for (x = 0; x < con.linewidth; x += 4)

			Text_DrawChar(cl_system_font->string, ((x+1)<<3)*vid_hudscale->value, y*vid_hudscale->value, '^');
	
		y -= system_font_ptr->line_height;
		rows--;
	}
	
	row = con.display;
	for (i=0 ; i<rows ; i++, y-= system_font_ptr->line_height, row--)
	{
		if (row < 0)
			break;
		if (con.current - row >= con.totallines)
			break;		// past scrollback wrap point
			
		text = con.text + (row % con.totallines)*con.linewidth;
		
		char original_character = text[con.linewidth];

		// send to the text drawing system
		// this is a stupid hack: we temporarily set the end of the line to a null byte to draw the line, then restore it so the entire console doesn't get fucked.
		text[con.linewidth] = '\0';
		Text_Draw(cl_system_font->string, 8 * vid_hudscale->value, y * vid_hudscale->value, text);
		text[con.linewidth] = original_character;
	}

//ZOID
	// draw the download bar
	// figure out width
	if (cls.download) {
		if ((text = strrchr(cls.downloadname, '/')) != NULL)
			text++;
		else
			text = cls.downloadname;

		x = con.linewidth - ((con.linewidth * 7) / 40);
		y = (x - (int)strlen(text) - 24 * vid_hudscale->value) / vid_hudscale->value;
		i = con.linewidth/6;
		if (strlen(text) > i) {
			y = x - i - 20;
			strncpy(dlbar, text, i);
			dlbar[i] = 0;
			strcat(dlbar, "...");
		} else
			strcpy(dlbar, text);
		strcat(dlbar, ": ");
		i = (int)strlen(dlbar);
		dlbar[i++] = '\x80';
		// where's the dot go?
		if (cls.downloadpercent == 0)
			n = 0;
		else
			n = y * cls.downloadpercent / 100;
			
		for (j = 0; j < y; j++)
			if (j == n)
				dlbar[i++] = '\x83';
			else
				dlbar[i++] = '\x81';
		dlbar[i++] = '\x82';
		dlbar[i] = 0;

		sprintf(dlbar + strlen(dlbar), " %02d%%", cls.downloadpercent);

		// draw it
		y = con.vislines-12*vid_hudscale->value;
		for (i = 0; i < strlen(dlbar); i++)
			Text_DrawChar(cl_system_font, ((i+1)<<3)*vid_hudscale->value, y, dlbar[i]);
	}
//ZOID

// draw the input prompt, user text, and cursor if desired
	Con_DrawInput ();
}


