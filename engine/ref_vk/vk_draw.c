/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2018-2019 Krzysztof Kondrak
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

// draw.c

#include "vk_local.h"

image_t		*draw_chars;

/*
===============
Draw_InitLocal
===============
*/
void Draw_InitLocal (void)
{
	// load console characters (don't bilerp characters)
	qvksampler_t samplerType = S_NEAREST;
	draw_chars = Vk_FindImage("pics/conchars.tga", it_pic, &samplerType);
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

	if (!vk_frameStarted)
		return;

	num &= 255;

	if ((num & 127) == 32)
		return;		// space

	cvar_t *scale = ri.Cvar_Get("hudscale", "1", 0);

	if (y <= -8 * scale->value)
		return;			// totally off screen

	row = num >> 4;
	col = num & 15;

	frow = row * 0.0625;
	fcol = col * 0.0625;
	size = 0.0625;

	float imgTransform[] = { (float)x / vid.width, (float)y / vid.height,
							 8.f * scale->value / vid.width, 8.f * scale->value / vid.height,
							 fcol, frow, size, size };
	QVk_DrawTexRect(imgTransform, sizeof(imgTransform), &draw_chars->vk_texture);
}

/*
=============
Draw_FindPic
=============
*/
image_t	*Draw_FindPic (char *name)
{
	image_t *vk;
	char	fullname[MAX_QPATH];

	if (name[0] != '/' && name[0] != '\\')
	{
		Com_sprintf(fullname, sizeof(fullname), "pics/%s.tga", name);
		vk = Vk_FindImage(fullname, it_pic, NULL);
	}
	else
		vk = Vk_FindImage(name + 1, it_pic, NULL);

	return vk;
}

/*
=============
Draw_GetPicSize
=============
*/
void Draw_GetPicSize (int *w, int *h, char *pic)
{
	image_t *vk;

	vk = Draw_FindPic(pic);
	if (!vk)
	{
		*w = *h = -1;
		return;
	}

	cvar_t *scale = ri.Cvar_Get("hudscale", "1", 0);

	*w = vk->width * scale->value;
	*h = vk->height * scale->value;
}

/*
=============
Draw_StretchPic
=============
*/
void Draw_StretchPic (int x, int y, int w, int h, char *pic)
{
	image_t *vk;

	if (!vk_frameStarted)
		return;

	vk = Draw_FindPic(pic);
	if (!vk)
	{
		ri.Con_Printf(PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}

	float imgTransform[] = { (float)x / vid.width, (float)y / vid.height,
							 (float)w / vid.width, (float)h / vid.height,
							  vk->sl,				vk->tl, 
							  vk->sh - vk->sl,		vk->th - vk->tl };
	QVk_DrawTexRect(imgTransform, sizeof(imgTransform), &vk->vk_texture);
}


/*
=============
Draw_Pic
=============
*/
void Draw_Pic (int x, int y, char *pic)
{
	image_t *vk;
	cvar_t *scale = ri.Cvar_Get("hudscale", "1", 0);

	vk = Draw_FindPic(pic);
	if (!vk)
	{
		ri.Con_Printf(PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}

	Draw_StretchPic(x, y, vk->width*scale->value, vk->height*scale->value, pic);
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

	image = Draw_FindPic(pic);
	if (!image)
	{
		ri.Con_Printf(PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}

	float imgTransform[] = { (float)x / vid.width,  (float)y / vid.height,
							 (float)w / vid.width,  (float)h / vid.height,
							 (float)x / 64.0,		(float)y / 64.0,
							 (float)w / 64.0,		(float)h / 64.0 };
	QVk_DrawTexRect(imgTransform, sizeof(imgTransform), &image->vk_texture);
}


/*
=============
Draw_Fill

Fills a box of pixels with a single color
=============
*/
void Draw_Fill (int x, int y, int w, int h, int r, int g, int b, int a)
{
	union
	{
		unsigned	c;
		byte		v[4];
	} color;

	if (!vk_frameStarted)
		return;

	// set up RGBA colors
	color.v[0] = r;
	color.v[1] = g;
	color.v[2] = b;
	color.v[3] = a;

	float imgTransform[] = { (float)x / vid.width, (float)y / vid.height,
							 (float)w / vid.width, (float)h / vid.height,
							 color.v[0] / 255.f, color.v[1] / 255.f, color.v[2] / 255.f, 1.f };
	QVk_DrawColorRect(imgTransform, sizeof(imgTransform), RP_UI);
}

//=============================================================================

/*
================
Draw_FadeScreen

================
*/
void Draw_FadeScreen (void)
{
	float imgTransform[] = { 0.f, 0.f, vid.width, vid.height, 0.f, 0.f, 0.f, .8f };

	if (!vk_frameStarted)
		return;

	QVk_DrawColorRect(imgTransform, sizeof(imgTransform), RP_UI);
}