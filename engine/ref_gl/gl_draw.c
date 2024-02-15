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

// draw.c

#include "gl_local.h"

image_t		*draw_chars;

extern	qboolean	scrap_dirty;
void Scrap_Upload (void);


/*
===============
Draw_InitLocal
===============
*/
void Draw_InitLocal (void)
{
	// load console characters (don't bilerp characters)
	draw_chars = GL_FindImage ("pics/conchars.tga", it_pic);
	GL_Bind( draw_chars->texnum );
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}



/*
================
Draw_Char

Draws one 8*8 graphics character with 0 being transparent.
It can be clipped to the top of the screen to allow the console to be
smoothly scrolled off.
================
*/
void Draw_Char (int x, int y, int num)
{
	int				row, col;
	float			frow, fcol, size;

	num &= 255;
	
	if ( (num&127) == 32 )
		return;		// space

	if (y <= -8)
		return;			// totally off screen

	cvar_t *scale = ri.Cvar_Get("hudscale", "1", 0);

	row = num>>4;
	col = num&15;

	frow = row*0.0625;
	fcol = col*0.0625;
	size = 0.0625;

	GL_Bind (draw_chars->texnum);

	qglBegin (GL_QUADS);
	qglTexCoord2f (fcol, frow);
	qglVertex2f (x, y);
	qglTexCoord2f (fcol + size, frow);
	qglVertex2f (x+8*scale->value, y);
	qglTexCoord2f (fcol + size, frow + size);
	qglVertex2f (x+8*scale->value, y+8*scale->value);
	qglTexCoord2f (fcol, frow + size);
	qglVertex2f (x, y+8*scale->value);
	qglEnd ();
}

/*
=============
Draw_FindPic
=============
*/
image_t	*Draw_FindPic (char *name)
{
	image_t *gl;
	char	fullname[MAX_QPATH];

	if (name[0] != '/' && name[0] != '\\')
	{
		Com_sprintf (fullname, sizeof(fullname), "pics/%s.tga", name);
		gl = GL_FindImage (fullname, it_pic);
	}
	else
		gl = GL_FindImage (name+1, it_pic);

	return gl;
}

/*
=============
Draw_GetPicSize
=============
*/
void Draw_GetPicSize (int *w, int *h, char *pic)
{
	image_t *gl;

	gl = Draw_FindPic (pic);
	if (!gl)
	{
		*w = *h = -1;
		return;
	}

	cvar_t *scale = ri.Cvar_Get("hudscale", "1", 0);

	*w = gl->width * scale->value;
	*h = gl->height * scale->value;
}

/*
=============
Draw_StretchPic
=============
*/
void Draw_StretchPic (int x, int y, int w, int h, char *pic)
{
	image_t *gl;

	gl = Draw_FindPic (pic);
	if (!gl)
	{
		ri.Con_Printf (PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}

	if (scrap_dirty)
		Scrap_Upload ();

	GL_Bind (gl->texnum);
	qglBegin (GL_QUADS);
	qglTexCoord2f (gl->sl, gl->tl);
	qglVertex2f (x, y);
	qglTexCoord2f (gl->sh, gl->tl);
	qglVertex2f (x+w, y);
	qglTexCoord2f (gl->sh, gl->th);
	qglVertex2f (x+w, y+h);
	qglTexCoord2f (gl->sl, gl->th);
	qglVertex2f (x, y+h);
	qglEnd ();
}


/*
=============
Draw_Pic
=============
*/
void Draw_Pic (int x, int y, char *pic)
{
	image_t *gl;
	cvar_t *scale = ri.Cvar_Get("hudscale", "1", 0);

	gl = Draw_FindPic (pic);
	if (!gl)
	{
		ri.Con_Printf (PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}
	if (scrap_dirty)
		Scrap_Upload ();

	GL_Bind (gl->texnum);
	qglBegin (GL_QUADS);
	qglTexCoord2f (gl->sl, gl->tl);
	qglVertex2f (x, y);
	qglTexCoord2f (gl->sh, gl->tl);
	qglVertex2f (x+gl->width*scale->value, y);
	qglTexCoord2f (gl->sh, gl->th);
	qglVertex2f (x+gl->width*scale->value, y+gl->height*scale->value);
	qglTexCoord2f (gl->sl, gl->th);
	qglVertex2f (x, y+gl->height*scale->value);
	qglEnd ();
}


/*
================
Draw_PicRegion
================
*/
void Draw_PicRegion(int x, int y, int start_x, int start_y, int end_x, int end_y, char *pic)
{
	image_t* gl;
	cvar_t* scale = ri.Cvar_Get("hudscale", "1", 0);

	gl = Draw_FindPic(pic);

	if (scrap_dirty)
	{
		Scrap_Upload();
	}

	// set up UV coordinates
	int coord_begin_x = (float)start_x * scale->value / (float)(gl->width * scale->value);
	int coord_begin_y = (float)start_y * scale->value / (float)(gl->height * scale->value);
	int coord_end_x = (float)end_x * scale->value / (float)(gl->width * scale->value);
	int coord_end_y = (float)end_y * scale->value / (float)(gl->height * scale->value);
	GL_Bind(gl->texnum);

	// draw it
	qglBegin(GL_QUADS);
	qglTexCoord2f(coord_begin_x, coord_begin_y);
	qglVertex2f(x, y);
	qglTexCoord2f(coord_end_x, coord_begin_y);
	qglVertex2f(x + gl->width * scale->value, y);
	qglTexCoord2f(coord_end_x, coord_end_y);
	qglVertex2f(x + gl->width * scale->value, y + gl->height * scale->value);
	qglTexCoord2f(coord_begin_x, coord_end_y);
	qglVertex2f(x, y + gl->height * scale->value);
	qglEnd();
}

/*
=============
Load_Pic

Load an image but don't draw immediately.
=============
*/
void Load_Pic(int x, int y, char* pic)
{
	image_t* gl;
	cvar_t* scale = ri.Cvar_Get("hudscale", "1", 0);

	gl = Draw_FindPic(pic);

	if (!gl)
	{
		ri.Con_Printf(PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}

	if (scrap_dirty)
		Scrap_Upload();

}


/*
=============
Draw_TileClear

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
void Draw_TileClear (int x, int y, int w, int h, char *pic)
{
	image_t	*image;

	image = Draw_FindPic (pic);
	if (!image)
	{
		ri.Con_Printf (PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}

	GL_Bind (image->texnum);
	qglBegin (GL_QUADS);
	qglTexCoord2f (x/64.0, y/64.0);
	qglVertex2f (x, y);
	qglTexCoord2f ( (x+w)/64.0, y/64.0);
	qglVertex2f (x+w, y);
	qglTexCoord2f ( (x+w)/64.0, (y+h)/64.0);
	qglVertex2f (x+w, y+h);
	qglTexCoord2f ( x/64.0, (y+h)/64.0 );
	qglVertex2f (x, y+h);
	qglEnd ();

}


/*
=============
Draw_Fill

Fills a box of pixels with a single color
WHY DOES THE ALPHA NOT WORK???
=============
*/
void Draw_Fill (int x, int y, int w, int h, int r, int g, int b, int a)
{
	union color_u
	{
		byte		v[4];
	} color;

	//qglBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);
	qglEnable (GL_BLEND);
	qglDisable (GL_TEXTURE_2D);

	// set up RGBA colors
	color.v[0] = r;
	color.v[1] = g;
	color.v[2] = b;
	color.v[3] = a;

	float alpha = (float)color.v[3] / 255.0f;

	qglColor4f (color.v[0]/255.0,
		color.v[1]/255.0,
		color.v[2]/255.0,
		alpha);

	qglBegin (GL_QUADS);

	qglVertex2f (x,y);
	qglVertex2f (x+w, y);
	qglVertex2f (x+w, y+h);
	qglVertex2f (x, y+h);

	qglEnd ();
	qglColor4f (1,1,1,1);
	qglEnable (GL_TEXTURE_2D);
	qglDisable (GL_BLEND);
}

//=============================================================================


/*
================
Draw_FadeScreen

================
*/
void Draw_FadeScreen (void)
{
	qglEnable (GL_BLEND);
	qglDisable (GL_TEXTURE_2D);
	qglColor4f (0, 0, 0, 0.8);
	qglBegin (GL_QUADS);

	qglVertex2f (0,0);
	qglVertex2f (vid.width, 0);
	qglVertex2f (vid.width, vid.height);
	qglVertex2f (0, vid.height);

	qglEnd ();
	qglColor4f (1,1,1,1);
	qglEnable (GL_TEXTURE_2D);
	qglDisable (GL_BLEND);
}
