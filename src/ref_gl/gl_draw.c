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

/*
===============
Draw_InitLocal
===============
*/
void Draw_InitLocal ()
{
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
		Com_sprintf (fullname, sizeof(fullname), "%s.tga", name);
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
void Draw_GetPicSize (int32_t *w, int32_t *h, char *pic)
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
void Draw_PicStretch (int32_t x, int32_t y, int32_t w, int32_t h, char *pic)
{
	image_t *gl;

	gl = Draw_FindPic (pic);
	if (!gl)
	{
		ri.Con_Printf (PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}

	GL_Bind (gl->texnum);
	glBegin (GL_QUADS);
	glTexCoord2f (gl->sl, gl->tl);
	glVertex2f (x, y);
	glTexCoord2f (gl->sh, gl->tl);
	glVertex2f (x+w, y);
	glTexCoord2f (gl->sh, gl->th);
	glVertex2f (x+w, y+h);
	glTexCoord2f (gl->sl, gl->th);
	glVertex2f (x, y+h);
	glEnd ();
}


/*
=============
Draw_Pic
=============
*/
void Draw_Pic (int32_t x, int32_t y, char *pic)
{
	image_t *gl;
	cvar_t *scale = ri.Cvar_Get("hudscale", "1", 0);

	gl = Draw_FindPic (pic);
	if (!gl)
	{
		ri.Con_Printf (PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}

	GL_Bind (gl->texnum);
	glBegin (GL_QUADS);
	glTexCoord2f (gl->sl, gl->tl);
	glVertex2f (x, y);
	glTexCoord2f (gl->sh, gl->tl);
	glVertex2f (x+gl->width*scale->value, y);
	glTexCoord2f (gl->sh, gl->th);
	glVertex2f (x+gl->width*scale->value, y+gl->height*scale->value);
	glTexCoord2f (gl->sl, gl->th);
	glVertex2f (x, y+gl->height*scale->value);
	glEnd ();

}


/*
================
Draw_PicRegion
================
*/
void Draw_PicRegion(int32_t x, int32_t y, int32_t start_x, int32_t start_y, int32_t end_x, int32_t end_y, char *pic, vec4_t color)
{
	image_t* gl;
	cvar_t* scale = ri.Cvar_Get("hudscale", "1", 0);

	gl = Draw_FindPic(pic);

	if (!gl)
	{
		ri.Con_Printf(PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}

	// set up UV coordinates
	float coord_begin_x = (float)start_x * scale->value / (float)(gl->upload_width * scale->value);
	float coord_begin_y = (float)start_y * scale->value / (float)(gl->upload_height * scale->value);
	float coord_end_x = (float)end_x * scale->value / (float)(gl->upload_width * scale->value);
	float coord_end_y = (float)end_y * scale->value / (float)(gl->upload_height * scale->value);

	float size_x = end_x - start_x;
	float size_y = end_y - start_y;

	GL_Bind(gl->texnum);

	// draw it
	// set up the colour as well
	glEnable(GL_BLEND);
	GL_TexEnv(GL_MODULATE); // multiply the glColor4f by the texture -> create a coloured texture.
	glColor4f(color[0] / 255.0f, color[1] / 255.0f, color[2] / 255.0f, color[3] / 255.0f);
 	glBegin(GL_QUADS);
	glTexCoord2f(coord_begin_x, coord_begin_y);
	glVertex2f(x, y);
	glTexCoord2f(coord_end_x, coord_begin_y);
	glVertex2f(x + size_x * scale->value, y);
	glTexCoord2f(coord_end_x, coord_end_y);
	glVertex2f(x + size_x * scale->value, y + size_y * scale->value);
	glTexCoord2f(coord_begin_x, coord_end_y);
	glVertex2f(x, y + size_y * scale->value);
	glEnd();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_BLEND);
}

/*
=============
Load_Pic

Load an image but don't draw immediately.
=============
*/
void Load_Pic(char* pic)
{
	image_t* gl;
	cvar_t* scale = ri.Cvar_Get("hudscale", "1", 0);

	gl = Draw_FindPic(pic);

	if (!gl)
	{
		ri.Con_Printf(PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}
}

/*
=============
Draw_TileClear

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
void Draw_TileClear (int32_t x, int32_t y, int32_t w, int32_t h, char *pic)
{
	image_t	*image;

	image = Draw_FindPic (pic);
	if (!image)
	{
		ri.Con_Printf (PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}

	GL_Bind (image->texnum);
	glBegin (GL_QUADS);
	glTexCoord2f (x/64.0, y/64.0);
	glVertex2f (x, y);
	glTexCoord2f ( (x+w)/64.0, y/64.0);
	glVertex2f (x+w, y);
	glTexCoord2f ( (x+w)/64.0, (y+h)/64.0);
	glVertex2f (x+w, y+h);
	glTexCoord2f ( x/64.0, (y+h)/64.0 );
	glVertex2f (x, y+h);
	glEnd ();

}


/*
=============
Draw_Fill

Fills a box of pixels with a single color
=============
*/
void Draw_Fill (int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, int32_t g, int32_t b, int32_t a)
{
	union color_u
	{
		byte		v[4];
	} color;

	//glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);
	glEnable (GL_BLEND);
	glDisable (GL_TEXTURE_2D);

	// set up RGBA colors
	color.v[0] = r;
	color.v[1] = g;
	color.v[2] = b;
	color.v[3] = a;

	float alpha = (float)color.v[3] / 255.0f;

	glColor4f (color.v[0]/255.0,
		color.v[1]/255.0,
		color.v[2]/255.0,
		alpha);

	glBegin (GL_QUADS);

	glVertex2f (x,y);
	glVertex2f (x+w, y);
	glVertex2f (x+w, y+h);
	glVertex2f (x, y+h);

	glEnd ();
	glColor4f (1,1,1,1);
	glEnable (GL_TEXTURE_2D);
	glDisable (GL_BLEND);
}

//=============================================================================


/*
================
Draw_FadeScreen

================
*/
void Draw_FadeScreen ()
{
	glEnable (GL_BLEND);
	glDisable (GL_TEXTURE_2D);
	glColor4f (0, 0, 0, 0.8);
	glBegin (GL_QUADS);

	glVertex2f (0,0);
	glVertex2f (vid.width, 0);
	glVertex2f (vid.width, vid.height);
	glVertex2f (0, vid.height);

	glEnd ();
	glColor4f (1,1,1,1);
	glEnable (GL_TEXTURE_2D);
	glDisable (GL_BLEND);
}
