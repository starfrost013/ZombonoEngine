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
// cl_scrn.c -- master for refresh, status bar, console, chat, notify, etc
// This sucks ass get rid of it

/*

  full screen console
  put up loading plaque
  blanked background with loading plaque
  blanked background with menu
  full screen image for quit and victory

  end of unit intermissions

  */

#include "client.h"
#include <inttypes.h>

float		scr_con_current;	// aproaches scr_conlines at scr_conspeed
float		scr_conlines;		// 0.0 to 1.0 lines of console to display

bool	scr_initialized;		// ready to draw

int32_t 		scr_draw_loading;

vrect_t		scr_vrect;		// position of render window on screen

cvar_t		*scr_viewsize;
cvar_t		*scr_conspeed;
cvar_t		*scr_centertime;
cvar_t		*scr_showpause;

cvar_t		*scr_timegraph;
cvar_t		*scr_debuggraph;
cvar_t		*scr_graphheight;
cvar_t		*scr_graphscale;
cvar_t		*scr_graphshift;

cvar_t	*vid_hudscale;
extern cvar_t	*vid_ref;

typedef struct
{
	int32_t 	x1, y1, x2, y2;
} dirty_t;

dirty_t		scr_dirty, scr_old_dirty[2];

char		crosshair_pic[MAX_QPATH];
int32_t 		crosshair_width, crosshair_height;

void SCR_TimeRefresh_f (void);
void SCR_Loading_f (void);


/*
===============================================================================

BAR GRAPHS

===============================================================================
*/

/*
==============
CL_AddNetgraph

A new packet was just parsed
==============
*/
void CL_AddNetgraph (void)
{
	int32_t 	i;
	int32_t 	in;
	int32_t 	ping;

	// if using the debuggraph for something else, don't
	// add the net lines
	if (scr_timegraph->value)
		return;
	// see what the latency was on this packet
	in = cls.netchan.incoming_acknowledged & (CMD_BACKUP-1);
	ping = cls.realtime - cl.cmd_time[in];
	ping /= 30;
	if (ping > 30)
		ping = 30;
	SCR_DebugGraph (ping, 18, 15, 22, 255);
}


typedef struct
{
	float	value;
	vec_t   color[4];
} graphsamp_t;

static	int32_t 		current;
static	graphsamp_t	values[1024];

/*
==============
SCR_DebugGraph
==============
*/
void SCR_DebugGraph (float value, int32_t r, int32_t g, int32_t b, int32_t a)
{
	values[current & 1023].value = value;
	values[current & 1023].color[0] = r;
	values[current & 1023].color[1] = g;
	values[current & 1023].color[2] = b;
	values[current & 1023].color[3] = a;
	current++;
}

/*
==============
SCR_DrawDebugGraph
==============
*/
void SCR_DrawDebugGraph (void)
{
	int32_t 	a, x, y, w, i, h;
	float	v;
	vec_t	color[4];

	//
	// draw the graph
	//
	w = scr_vrect.width;

	x = scr_vrect.x;
	y = scr_vrect.y+scr_vrect.height;
	re.DrawFill (x, y-scr_graphheight->value,
		w, scr_graphheight->value, 123, 123, 123, 255);

	for (a=0 ; a<w ; a++)
	{
		i = (current-1-a+1024) & 1023;
		v = values[i].value;
		v = v*scr_graphscale->value + scr_graphshift->value;
		
		if (v < 0)
			v += scr_graphheight->value * (1+(int32_t)(-v/scr_graphheight->value));
		h = (int32_t)v % (int32_t)scr_graphheight->value;
		re.DrawFill (x+w-1-a, y - h, 1,	h, values[i].color[0], values[i].color[1], values[i].color[2], values[i].color[3]);
	}
}

/*
===============================================================================

CENTER PRINTING

===============================================================================
*/

char		scr_centerstring[1024];
float		scr_centertime_start;	// for slow victory printing
float		scr_centertime_off;
int32_t 		scr_center_lines;
int32_t 		scr_erase_center;

/*
==============
SCR_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void SCR_CenterPrint (char *str)
{
	char	*s;
	int32_t 	i, j, l;

	strncpy (scr_centerstring, str, sizeof(scr_centerstring));
	scr_centertime_off = scr_centertime->value;
	scr_centertime_start = cl.time;

	// count the number of lines for centering
	scr_center_lines = 1;
	s = str;
	while (*s)
	{
		if (*s == '\n')
			scr_center_lines++;
		s++;
	}

	// echo it to the console
	Com_Printf("\n\n");

	Com_Printf("%s", str);

	Com_Printf("\n\n");
	Con_ClearNotify ();
}


void SCR_DrawCenterString (void)
{
	char	*start;
	int32_t 	l;
	int32_t 	j;
	int32_t 	x, y;
	int32_t 	remaining;
	int32_t 	size_x = 0, size_y = 0;
	font_t* system_font_ptr = Font_GetByName(cl_system_font->string);
// the finale prints the characters one at a time
	remaining = 9999;

	scr_erase_center = 0;
	start = scr_centerstring;

	if (scr_center_lines <= 4)
		y = viddef.height*0.35;
	else
		y = 48;

	Text_GetSize(cl_system_font->string, &size_x, &size_y, start);
	x = (viddef.width - size_x * vid_hudscale->value) / 2;
	SCR_AddDirtyPoint32_t(x, y);
	Text_Draw(cl_system_font->string, x, y, start);
	SCR_AddDirtyPoint32_t(x, y + system_font_ptr->line_height * vid_hudscale->value);
}

void SCR_CheckDrawCenterString (void)
{
	scr_centertime_off -= cls.frametime;
	
	if (scr_centertime_off <= 0)
		return;

	SCR_DrawCenterString ();
}

//=============================================================================

/*
=================
SCR_CalcVrect

Sets scr_vrect, the coordinates of the rendered window
=================
*/
static void SCR_CalcVrect (void)
{
	int32_t 	size;

	// bound viewsize
	if (scr_viewsize->value < 40)
		Cvar_Set ("viewsize","40");
	if (scr_viewsize->value > 100)
		Cvar_Set ("viewsize","100");

	size = scr_viewsize->value;

	scr_vrect.width  = viddef.width*size/100;
	scr_vrect.height = viddef.height*size/100;

	scr_vrect.x = (viddef.width - scr_vrect.width)/2;
	scr_vrect.y = (viddef.height - scr_vrect.height)/2;
}


/*
=================
SCR_SizeUp_f

Keybinding command
=================
*/
void SCR_SizeUp_f (void)
{
	Cvar_SetValue ("viewsize",scr_viewsize->value+10);
}


/*
=================
SCR_SizeDown_f

Keybinding command
=================
*/
void SCR_SizeDown_f (void)
{
	Cvar_SetValue ("viewsize",scr_viewsize->value-10);
}

/*
=================
SCR_Sky_f

Set a specific sky and rotation speed
=================
*/
void SCR_Sky_f (void)
{
	float	rotate;
	vec3_t	axis;

	if (Cmd_Argc() < 2)
	{
		Com_Printf ("Usage: sky <basename> <rotate> <axis x y z>\n");
		return;
	}
	if (Cmd_Argc() > 2)
		rotate = atof(Cmd_Argv(2));
	else
		rotate = 0;
	if (Cmd_Argc() == 6)
	{
		axis[0] = atof(Cmd_Argv(3));
		axis[1] = atof(Cmd_Argv(4));
		axis[2] = atof(Cmd_Argv(5));
	}
	else
	{
		axis[0] = 0;
		axis[1] = 0;
		axis[2] = 1;
	}

	re.SetSky (Cmd_Argv(1), rotate, axis);
}

//============================================================================

/*
==================
SCR_Init
==================
*/
void SCR_Init (void)
{
	scr_viewsize = Cvar_Get ("viewsize", "100", CVAR_ARCHIVE);
	scr_conspeed = Cvar_Get ("scr_conspeed", "3", 0);
	scr_showpause = Cvar_Get ("scr_showpause", "1", 0);
	scr_centertime = Cvar_Get ("scr_centertime", "2.5", 0);
	scr_timegraph = Cvar_Get ("timegraph", "0", 0);
	scr_debuggraph = Cvar_Get ("debuggraph", "0", 0);
	scr_graphheight = Cvar_Get ("graphheight", "32", 0);
	scr_graphscale = Cvar_Get ("graphscale", "1", 0);
	scr_graphshift = Cvar_Get ("graphshift", "0", 0);

//
// register our commands
//
	Cmd_AddCommand ("timerefresh",SCR_TimeRefresh_f);
	Cmd_AddCommand ("loading",SCR_Loading_f);
	Cmd_AddCommand ("sizeup",SCR_SizeUp_f);
	Cmd_AddCommand ("sizedown",SCR_SizeDown_f);
	Cmd_AddCommand ("sky",SCR_Sky_f);

	scr_initialized = true;
}


/*
==============
SCR_DrawNet
==============
*/
void SCR_DrawNet (void)
{
	if (cls.netchan.outgoing_sequence - cls.netchan.incoming_acknowledged 
		< CMD_BACKUP-1)
		return;

	re.DrawPic (scr_vrect.x+64, scr_vrect.y, "pics/net");
}

/*
==============
SCR_DrawPause
==============
*/
void SCR_DrawPause (void)
{
	int32_t 	w, h;

	if (!scr_showpause->value)		// turn off for screenshots
		return;

	if (!cl_paused->value)
		return;

	re.DrawGetPicSize (&w, &h, "pics/pause");
	re.DrawPic ((viddef.width-w)/2, viddef.height/2 + 8*vid_hudscale->value, "pics/pause");
}

/*
==============
SCR_DrawLoading
==============
*/
void SCR_DrawLoading (void)
{
	int32_t 	w, h;
		
	if (!scr_draw_loading)
		return;

	scr_draw_loading = false;
	re.DrawGetPicSize (&w, &h, "pics/loading");
	re.DrawPic ((viddef.width-w)/2, (viddef.height-h)/2, "pics/loading");
}

//=============================================================================

/*
==================
SCR_RunConsole

Scroll it up or down
==================
*/
void SCR_RunConsole (void)
{
// decide on the height of the console
	if (cls.key_dest == key_console)
		scr_conlines = 0.5;		// half screen
	else
		scr_conlines = 0;				// none visible
	
	if (scr_conlines < scr_con_current)
	{
		scr_con_current -= scr_conspeed->value*cls.frametime;
		if (scr_conlines > scr_con_current)
			scr_con_current = scr_conlines;

	}
	else if (scr_conlines > scr_con_current)
	{
		scr_con_current += scr_conspeed->value*cls.frametime;
		if (scr_conlines < scr_con_current)
			scr_con_current = scr_conlines;
	}

}

/*
==================
SCR_DrawConsole
==================
*/
void SCR_DrawConsole (void)
{
	Con_CheckResize ();
	
	if (cls.state == ca_disconnected || cls.state == ca_connecting)
	{	// forced full screen console
		Con_DrawConsole (1.0);
		return;
	}

	if (cls.state != ca_active || !cl.refresh_prepped)
	{	// connected, but can't render
		Con_DrawConsole (0.5);
		re.DrawFill (0, viddef.height/2, viddef.width, viddef.height/2, 0, 0, 0, 255);
		return;
	}

	if (scr_con_current)
	{
		Con_DrawConsole (scr_con_current);
	}
	else
	{
		if (cls.key_dest == key_game || cls.key_dest == key_message)
			Con_DrawNotify ();	// only draw notify in game
	}
}

//=============================================================================

/*
================
SCR_BeginLoadingPlaque
================
*/
void SCR_BeginLoadingPlaque (void)
{
	S_StopAllSounds ();
	cl.sound_prepped = false;		// don't play ambients
	Miniaudio_Stop ();
	if (cls.disable_screen)
		return;
	if (developer->value)
		return;
	if (cls.state == ca_disconnected)
		return;	// if at console, don't bring up the plaque
	if (cls.key_dest == key_console)
		return;

	scr_draw_loading = 1;

	SCR_UpdateScreen ();
	cls.disable_screen = Sys_Milliseconds ();
	cls.disable_servercount = cl.servercount;
}

/*
================
SCR_EndLoadingPlaque
================
*/
void SCR_EndLoadingPlaque (void)
{
	cls.disable_screen = 0;
	Con_ClearNotify ();
}

/*
================
SCR_Loading_f
================
*/
void SCR_Loading_f (void)
{
	SCR_BeginLoadingPlaque ();
}

/*
================
SCR_TimeRefresh_f
================
*/
int32_t entitycmpfnc( const entity_t *a, const entity_t *b )
{
	/*
	** all other models are sorted by model then skin
	*/
	if ( a->model == b->model )
	{
		return ( ( intptr_t ) a->skin - ( intptr_t ) b->skin );
	}
	else
	{
		return ( (intptr_t) a->model - (intptr_t) b->model );
	}
}

void SCR_TimeRefresh_f (void)
{
	int32_t 	i;
	int32_t 	start, stop;
	float	time;

	if ( cls.state != ca_active )
		return;

	start = Sys_Milliseconds ();

	if (Cmd_Argc() == 2)
	{	// run without page flipping
		re.BeginFrame( 0 );
		for (i=0 ; i<128 ; i++)
		{
			cl.refdef.viewangles[1] = i/128.0*360.0;
			re.RenderFrame (&cl.refdef);
		}
		re.EndFrame();
	}
	else
	{
		for (i=0 ; i<128 ; i++)
		{
			cl.refdef.viewangles[1] = i/128.0*360.0;

			re.BeginFrame( 0 );
			re.RenderFrame (&cl.refdef);
			re.EndFrame();
		}
	}

	stop = Sys_Milliseconds ();
	time = (stop-start)/1000.0;
	Com_Printf ("%f seconds (%f fps)\n", time, 128/time);
}

/*
=================
SCR_AddDirtyPoint
=================
*/
void SCR_AddDirtyPoint32_t (int32_t x, int32_t y)
{
	if (x < scr_dirty.x1)
		scr_dirty.x1 = x;
	if (x > scr_dirty.x2)
		scr_dirty.x2 = x;
	if (y < scr_dirty.y1)
		scr_dirty.y1 = y;
	if (y > scr_dirty.y2)
		scr_dirty.y2 = y;
}

void SCR_DirtyScreen (void)
{
	SCR_AddDirtyPoint32_t (0, 0);
	SCR_AddDirtyPoint32_t (viddef.width-1, viddef.height-1);
}

/*
==============
SCR_TileClear

Clear any parts of the tiled background that were drawn on last frame
==============
*/
void SCR_TileClear (void)
{
	if (scr_con_current == 1.0)
		return;		// full screen console
	if (scr_viewsize->value == 100)
		return;		// full screen rendering

	if (scr_vrect.y > 0)
	{	// clear above view screen
		re.DrawTileClear (scr_vrect.x , 0,
			scr_vrect.width, scr_vrect.y, "backtile");

		// clear below view screen
		re.DrawTileClear(scr_vrect.x, scr_vrect.y + scr_vrect.height,
			scr_vrect.width, viddef.height - scr_vrect.height - scr_vrect.y, "backtile");
	}

	if (scr_vrect.x > 0)
	{	// clear left of view screen
		re.DrawTileClear (0, 0, scr_vrect.x, viddef.height, "backtile");

		// clear right of view screen
		re.DrawTileClear(scr_vrect.x + scr_vrect.width, 0, scr_vrect.width, viddef.height, "backtile");
	}
}


//===============================================================


#define STAT_MINUS		10	// num frame for '-' stats digit
char		*sb_nums[2][11] = 
{
	// 2 sets of big numbers
	{"pics/num_0", "pics/num_1", "pics/num_2", "pics/num_3", "pics/num_4", "pics/num_5", "pics/num_6", "pics/num_7", "pics/num_8", "pics/num_9", "pics/num_minus"},
	{"pics/anum_0", "pics/anum_1", "pics/anum_2", "pics/anum_3", "pics/anum_4", "pics/anum_5", "pics/anum_6", "pics/anum_7", "pics/anum_8", "pics/anum_9", "pics/anum_minus"}
};

#define	ICON_WIDTH	24
#define	ICON_HEIGHT	24
#define	CHAR_WIDTH	16 * vid_hudscale->value
#define	ICON_SPACE	8

/*
================
SizeHUDString

Allow embedded \n in the string
================
*/
void SizeHUDString (char *string, int32_t *w, int32_t *h)
{
	int32_t 	lines, width, current;

	lines = 1;
	width = 0;

	current = 0;
	while (*string)
	{
		if (*string == '\n')
		{
			lines++;
			current = 0;
		}
		else
		{
			current++;
			if (current > width)
				width = current;
		}
		string++;
	}

	*w = width * 8 * vid_hudscale->value;
	*h = lines * 8 * vid_hudscale->value;
}

void DrawCenteredString (char *string, int32_t x, int32_t y, int32_t centerwidth)
{
	int32_t 	margin;
	char	line[1024];
	int32_t 	width;
	int32_t 	i;
	font_t* system_font_ptr = Font_GetByName(cl_system_font->string);

	margin = x;

	while (*string)
	{
		// scan out one line of text from the string
		width = 0;
		while (*string && *string != '\n')
			line[width++] = *string++;
		line[width] = 0;

		if (centerwidth)
			x = margin + ((centerwidth - width*8)*vid_hudscale->value)/2;
		else
			x = margin;

		// set string terminator for text engine
		char orig_end_of_line = line[width];
		line[width] = '\0';

		Text_Draw(cl_system_font->string, x, y, line);

		line[width] = orig_end_of_line;
		if (*string)
		{
			string++;	// skip the \n
			x = margin;
			y += system_font_ptr->line_height * vid_hudscale->value;
		}
	}
}


/*
==============
SCR_DrawField
==============
*/
void SCR_DrawField (int32_t x, int32_t y, int32_t color, int32_t width, int32_t value)
{
	char	num[16], *ptr;
	int32_t 	l;
	int32_t 	frame;

	if (width < 1)
		return;

	// draw number string
	if (width > 5)
		width = 5;

	SCR_AddDirtyPoint32_t (x, y);
	SCR_AddDirtyPoint32_t (x+width*CHAR_WIDTH+2, y+23);

	Com_sprintf (num, sizeof(num), "%i", value);
	l = (int32_t)strlen(num);
	if (l > width)
		l = width;
	x += 2 + CHAR_WIDTH*(width - l);

	ptr = num;
	while (*ptr && l)
	{
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr -'0';

		re.DrawPic (x,y,sb_nums[color][frame]);
		x += CHAR_WIDTH;
		ptr++;
		l--;
	}
}


/*
===============
SCR_TouchPics

Allows rendering code to cache all needed sbar graphics
===============
*/
void SCR_TouchPics (void)
{
	int32_t 	i, j;

	for (i=0 ; i<2 ; i++)
		for (j=0 ; j<11 ; j++)
			re.RegisterPic (sb_nums[i][j]);

	if (crosshair->value)
	{
		cvar_t *scale = Cvar_Get("hudscale", "1", 0);

		if (crosshair->value > 3 || crosshair->value < 0)
			crosshair->value = 3;

		Com_sprintf (crosshair_pic, sizeof(crosshair_pic), "pics/ch%i", (int32_t)(crosshair->value));
		re.DrawGetPicSize (&crosshair_width, &crosshair_height, crosshair_pic);

		// remove scaling - it will be applied during draw with proper screen centering
		if (scale->value > 1)
		{
			crosshair_width /= (int32_t)scale->value;
			crosshair_height /= (int32_t)scale->value;
		}

		if (!crosshair_width)
			crosshair_pic[0] = 0;
	}
}

/*
================
SCR_ExecuteLayoutString 

================
*/
void SCR_ExecuteLayoutString (char *s)
{
	int32_t 	x, y;
	int32_t 	value;
	char	*token;
	int32_t 	width;
	int32_t 	index;
	clientinfo_t	*ci;

	if (cls.state != ca_active || !cl.refresh_prepped)
		return;

	if (!s[0])
		return;

	x = 0;
	y = 0;
	width = 3;

	while (s)
	{
		token = COM_Parse (&s);
		if (!strcmp(token, "xl"))
		{
			token = COM_Parse (&s);
			x = atoi(token) * vid_hudscale->value;
			continue;
		}
		if (!strcmp(token, "xr"))
		{
			token = COM_Parse (&s);
			x = viddef.width + atoi(token) * vid_hudscale->value;
			continue;
		}
		if (!strcmp(token, "xv"))
		{
			token = COM_Parse (&s);
			x = viddef.width/2 - 160 * vid_hudscale->value + atoi(token) * vid_hudscale->value;
			continue;
		}

		if (!strcmp(token, "yt"))
		{
			token = COM_Parse (&s);
			y = atoi(token) * vid_hudscale->value;
			continue;
		}
		if (!strcmp(token, "yb"))
		{
			token = COM_Parse (&s);
			y = viddef.height + atoi(token) * vid_hudscale->value;
			continue;
		}
		if (!strcmp(token, "yv"))
		{
			token = COM_Parse (&s);
			y = viddef.height/2 - 120 * vid_hudscale->value + atoi(token) * vid_hudscale->value;
			continue;
		}

		if (!strcmp(token, "pic"))
		{	// draw a pic from a stat number
			token = COM_Parse (&s);
			value = cl.frame.playerstate.stats[atoi(token)];
			if (value >= MAX_IMAGES)
				Com_Error (ERR_DROP, "Pic >= MAX_IMAGES");
			if (cl.configstrings[CS_IMAGES+value])
			{
				SCR_AddDirtyPoint32_t (x, y);
				SCR_AddDirtyPoint32_t (x+23, y+23);
				re.DrawPic (x, y, cl.configstrings[CS_IMAGES+value]);
			}
			continue;
		}

		if (!strcmp(token, "picn"))
		{	// draw a pic from a name
			token = COM_Parse (&s);
			SCR_AddDirtyPoint32_t (x, y);
			SCR_AddDirtyPoint32_t (x+23, y+23);
			re.DrawPic (x, y, token);
			continue;
		}

		if (!strcmp(token, "num"))
		{	// draw a number
			token = COM_Parse (&s);
			width = atoi(token);
			token = COM_Parse (&s);
			value = cl.frame.playerstate.stats[atoi(token)];
			SCR_DrawField (x, y, 0, width, value);
			continue;
		}

		if (!strcmp(token, "hnum"))
		{	// health number
			int32_t 	color;

			width = 3;
			value = cl.frame.playerstate.stats[STAT_HEALTH];
			if (value > 25)
				color = 0;	// green
			else if (value > 0)
				color = (cl.frame.serverframe>>2) & 1;		// flash
			else
				color = 1;

			if (cl.frame.playerstate.stats[STAT_FLASHES] & 1)
				re.DrawPic (x, y, "pics/field_3");

			SCR_DrawField (x, y, color, width, value);
			continue;
		}

		if (!strcmp(token, "anum"))
		{	// ammo number
			int32_t 	color;

			width = 3;
			value = cl.frame.playerstate.stats[STAT_AMMO];
			if (value > 5)
				color = 0;	// green
			else if (value >= 0)
				color = (cl.frame.serverframe>>2) & 1;		// flash
			else
				continue;	// negative number = don't show

			if (cl.frame.playerstate.stats[STAT_FLASHES] & 4)
				re.DrawPic (x, y, "pics/field_3");

			SCR_DrawField (x, y, color, width, value);
			continue;
		}

		if (!strcmp(token, "rnum"))
		{	// armor number
			int32_t 	color;

			width = 3;
			value = cl.frame.playerstate.stats[STAT_ARMOR];
			if (value < 1)
				continue;

			color = 0;	// green

			if (cl.frame.playerstate.stats[STAT_FLASHES] & 2)
				re.DrawPic (x, y, "pics/field_3");

			SCR_DrawField (x, y, color, width, value);
			continue;
		}


		if (!strcmp(token, "stat_string"))
		{
			token = COM_Parse (&s);
			index = atoi(token);
			if (index < 0 || index >= MAX_CONFIGSTRINGS)
				Com_Error (ERR_DROP, "Bad stat_string index");
			index = cl.frame.playerstate.stats[index];
			if (index < 0 || index >= MAX_CONFIGSTRINGS)
				Com_Error (ERR_DROP, "Bad stat_string index");
			Text_Draw (cl_system_font->string, x, y, cl.configstrings[index]);
			continue;
		}

		if (!strcmp(token, "cstring"))
		{
			token = COM_Parse (&s);
			DrawCenteredString (token, x, y, 320);
			continue;
		}

		if (!strcmp(token, "string"))
		{
			token = COM_Parse (&s);
			//todo: is this terminated properly?
			Text_Draw (cl_system_font->string, x, y, token);
			continue;
		}

		if (!strcmp(token, "if"))
		{	// draw a number
			token = COM_Parse (&s);
			value = cl.frame.playerstate.stats[atoi(token)];
			if (!value)
			{	// skip to endif
				while (s && strcmp(token, "endif") )
				{
					token = COM_Parse (&s);
				}
			}

			continue;
		}
	}
}

/*
================
SCR_DrawStats

The status bar is a small layout program that
is based on the stats array
================
*/
void SCR_DrawStats (void)
{
	SCR_ExecuteLayoutString (cl.configstrings[CS_STATUSBAR]);
}


/*
================
SCR_DrawLayout

================
*/
#define	STAT_LAYOUTS		13

void SCR_DrawLayout (void)
{
	if (!cl.frame.playerstate.stats[STAT_LAYOUTS])
		return;
	SCR_ExecuteLayoutString (cl.layout);
}

void SCR_DrawPos()
{
	font_t* console_font_ptr = Font_GetByName(cl_console_font->string);

	if (console_font_ptr == NULL)
	{
		Sys_Error("cl_console_font NULL in SCR_DrawPos!");
		return;
	}

	int y = (viddef.height - (60 * vid_hudscale->value));
	Text_Draw(cl_console_font->string, (30 * vid_hudscale->value), y, 
		"Position: %.2f  %.2f  %.2f", cl.frame.playerstate.pmove.origin[0], cl.frame.playerstate.pmove.origin[1], cl.frame.playerstate.pmove.origin[2]);
	y += console_font_ptr->line_height * vid_hudscale->value;
	Text_Draw(cl_console_font->string, (30 * vid_hudscale->value), y,
		"Velocity: %.2f  %.2f  %.2f", cl.frame.playerstate.pmove.velocity[0], cl.frame.playerstate.pmove.velocity[1], cl.frame.playerstate.pmove.velocity[2]);
}

//=======================================================

/*
==================
SCR_UpdateScreen

This is called every frame, and can also be called explicitly to flush
text to the screen.
==================
*/
void SCR_UpdateScreen (void)
{
	int32_t numframes;
	int32_t i;
	float separation[2] = { 0, 0 };

	// if the screen is disabled (loading plaque is up, or vid mode changing)
	// do nothing at all
	if (cls.disable_screen)
	{
		if (Sys_Milliseconds() - cls.disable_screen > 120000)
		{
			cls.disable_screen = 0;
			Com_Printf ("Loading plaque timed out.\n");
		}
		return;
	}

	if (!scr_initialized || !con.initialized)
		return;				// not initialized yet

	/*
	** range check cl_camera_separation so we don't inadvertently fry someone's
	** brain
	*/
	if ( cl_stereo_separation->value > 1.0 )
		Cvar_SetValue( "cl_stereo_separation", 1.0 );
	else if ( cl_stereo_separation->value < 0 )
		Cvar_SetValue( "cl_stereo_separation", 0.0 );

	if ( cl_stereo->value )
	{
		numframes = 2;
		separation[0] = -cl_stereo_separation->value / 2;
		separation[1] =  cl_stereo_separation->value / 2;
	}		
	else
	{
		separation[0] = 0;
		separation[1] = 0;
		numframes = 1;
	}

	for ( i = 0; i < numframes; i++ )
	{
		re.BeginFrame( separation[i] );
		// end frame and force video restart if swapchain is out of date
		if (vid_ref->modified)
		{
			re.EndWorldRenderpass();
			re.EndFrame();
			return;
		}

		if (scr_draw_loading == 2)
		{	//  loading plaque over black screen
			int32_t 	w, h;

			re.EndWorldRenderpass();
			scr_draw_loading = false;
			re.DrawGetPicSize (&w, &h, "pics/loading");
			re.DrawPic ((viddef.width-w)/2, (viddef.height-h)/2, "pics/loading");
//			re.EndFrame();
//			return;
		} 
		else 
		{
		
			// do 3D refresh drawing, and then update the screen
			SCR_CalcVrect ();

			V_RenderView ( separation[i] );

			// clear any dirty part of the background
			SCR_TileClear ();

			// still draw console if cl_drawhud is 0
			if (cl_drawhud->value)
			{
				SCR_DrawStats();
				if (cl.frame.playerstate.stats[STAT_LAYOUTS] & 1)
					SCR_DrawLayout();
				if (cl.frame.playerstate.stats[STAT_LAYOUTS] & 2)
					CL_DrawInventory();

				SCR_DrawNet();
				SCR_CheckDrawCenterString();

				if (scr_timegraph->value)
					SCR_DebugGraph(cls.frametime * 300, 0, 0, 0, 255);

				if (scr_debuggraph->value || scr_timegraph->value)
					SCR_DrawDebugGraph();

				SCR_DrawPause();

				UI_Draw();
			}

			if (cl_showpos->value)
			{
				SCR_DrawPos();
			}
			
			SCR_DrawConsole ();

			M_Draw ();

			SCR_DrawLoading ();
			
		}
	}
	re.EndFrame();
}
