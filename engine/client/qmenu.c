/*
Copyright (C) 1997-2001 Id Software, Inc.

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
#include <string.h>
#include <ctype.h>

#include "client.h"
#include "qmenu.h"

static void	 Action_DoEnter( menuaction_t *a );
static void	 Action_Draw( menuaction_t *a );
static void  Menu_DrawStatusBar( const char *string );
static void	 Menulist_DoEnter( menulist_t *l );
static void	 MenuList_Draw( menulist_t *l );
static void	 Separator_Draw( menuseparator_t *s );
static void	 Slider_DoSlide( menuslider_t *s, int dir );
static void	 Slider_Draw( menuslider_t *s );
static void	 SpinControl_DoEnter( menulist_t *s );
static void	 SpinControl_Draw( menulist_t *s );
static void	 SpinControl_DoSlide( menulist_t *s, int dir );

#define RCOLUMN_OFFSET  16 * vid_hudscale->value
#define LCOLUMN_OFFSET -16 * vid_hudscale->value

extern refexport_t re;
extern viddef_t viddef;
extern cvar_t *vid_hudscale;

#define VID_WIDTH viddef.width
#define VID_HEIGHT viddef.height

#define Draw_Char re.DrawChar
#define Draw_Fill re.DrawFill

void Action_DoEnter( menuaction_t *a )
{
	if ( a->generic.callback )
		a->generic.callback( a );
}

void Action_Draw( menuaction_t *a )
{
	// emulate r2l 
	int size_x = 0, size_y = 0;

	if ( a->generic.flags & QMF_LEFT_JUSTIFY )
	{
		// todo: non-system fonts in menu UI?
		Text_Draw(cl_system_font->string, a->generic.x + a->generic.parent->x + LCOLUMN_OFFSET, a->generic.y + a->generic.parent->y, a->generic.name);
	}
	else
	{
		Text_GetSize(cl_system_font->string, &size_x, &size_y, a->generic.name);
		Text_Draw(cl_system_font->string, a->generic.x + a->generic.parent->x + LCOLUMN_OFFSET - size_x, a->generic.y + a->generic.parent->y, a->generic.name);
	}
	if ( a->generic.ownerdraw )
		a->generic.ownerdraw( a );
}

qboolean Field_DoEnter( menufield_t *f )
{
	if ( f->generic.callback )
	{
		f->generic.callback( f );
		return true;
	}
	return false;
}

void Field_Draw( menufield_t *f )
{
	int i;
	int size_x = 0, size_y = 0;
	char tempbuffer[128]="";

	if (f->generic.name)
	{
		Text_GetSize(cl_system_font->string, &size_x, &size_y, f->generic.name);
		Text_Draw(cl_system_font->string, f->generic.x + f->generic.parent->x + LCOLUMN_OFFSET - size_x, f->generic.y + f->generic.parent->y, f->generic.name);
	}
	
	strncpy( tempbuffer, f->buffer + f->visible_offset, f->visible_length );

	Draw_Char( f->generic.x + f->generic.parent->x + 16 * vid_hudscale->value, f->generic.y + f->generic.parent->y - 4 * vid_hudscale->value, 18 );
	Draw_Char( f->generic.x + f->generic.parent->x + 16 * vid_hudscale->value, f->generic.y + f->generic.parent->y + 4 * vid_hudscale->value, 24 );

	Draw_Char( f->generic.x + f->generic.parent->x + 24 * vid_hudscale->value + f->visible_length * 8 * vid_hudscale->value, f->generic.y + f->generic.parent->y - 4 * vid_hudscale->value, 20 );
	Draw_Char( f->generic.x + f->generic.parent->x + 24 * vid_hudscale->value + f->visible_length * 8 * vid_hudscale->value, f->generic.y + f->generic.parent->y + 4 * vid_hudscale->value, 26 );

	for ( i = 0; i < f->visible_length; i++ )
	{
		Draw_Char( f->generic.x + f->generic.parent->x + 24 * vid_hudscale->value + i * 8 * vid_hudscale->value, f->generic.y + f->generic.parent->y - 4 * vid_hudscale->value, 19 );
		Draw_Char( f->generic.x + f->generic.parent->x + 24 * vid_hudscale->value + i * 8 * vid_hudscale->value, f->generic.y + f->generic.parent->y + 4 * vid_hudscale->value, 25 );
	}

	Text_Draw(cl_system_font->string, f->generic.x + f->generic.parent->x + 24 * vid_hudscale->value, f->generic.y + f->generic.parent->y, tempbuffer );

	if ( Menu_ItemAtCursor( f->generic.parent ) == f )
	{
		int offset;

		if ( f->visible_offset )
			offset = f->visible_length;
		else
			offset = f->cursor;

		if ( ( ( int ) ( Sys_Milliseconds() / 250 ) ) & 1 )
		{
			Draw_Char( f->generic.x + f->generic.parent->x + ( offset + 2 ) * 8 * vid_hudscale->value + 8 * vid_hudscale->value,
					   f->generic.y + f->generic.parent->y,
					   11 );
		}
		else
		{
			Draw_Char( f->generic.x + f->generic.parent->x + ( offset + 2 ) * 8 * vid_hudscale->value + 8 * vid_hudscale->value,
					   f->generic.y + f->generic.parent->y,
					   ' ' );
		}
	}
}

qboolean Field_Key( menufield_t *f, int key )
{
	extern int keydown[];

	switch ( key )
	{
	case K_KP_SLASH:
		key = '/';
		break;
	case K_KP_MINUS:
		key = '-';
		break;
	case K_KP_PLUS:
		key = '+';
		break;
	case K_KP_HOME:
		key = '7';
		break;
	case K_KP_UPARROW:
		key = '8';
		break;
	case K_KP_PGUP:
		key = '9';
		break;
	case K_KP_LEFTARROW:
		key = '4';
		break;
	case K_KP_5:
		key = '5';
		break;
	case K_KP_RIGHTARROW:
		key = '6';
		break;
	case K_KP_END:
		key = '1';
		break;
	case K_KP_DOWNARROW:
		key = '2';
		break;
	case K_KP_PGDN:
		key = '3';
		break;
	case K_KP_INS:
		key = '0';
		break;
	case K_KP_DEL:
		key = '.';
		break;
	}

	if ( key > 127 )
	{
		switch ( key )
		{
		case K_DEL:
		default:
			return false;
		}
	}

	/*
	** support pasting from the clipboard
	*/
	if ( ( toupper( key ) == 'V' && keydown[K_CTRL] ) ||
		 ( ( ( key == K_INS ) || ( key == K_KP_INS ) ) && keydown[K_SHIFT] ) )
	{
		char *cbd;
		
		if ( ( cbd = Sys_GetClipboardData() ) != 0 )
		{
			strtok( cbd, "\n\r\b" );

			strncpy( f->buffer, cbd, f->length - 1 );
			f->cursor = (int)strlen( f->buffer );
			f->visible_offset = f->cursor - f->visible_length;
			if ( f->visible_offset < 0 )
				f->visible_offset = 0;

			free( cbd );
		}
		return true;
	}

	switch ( key )
	{
	case K_KP_LEFTARROW:
	case K_LEFTARROW:
	case K_BACKSPACE:
		if ( f->cursor > 0 )
		{
			memmove( &f->buffer[f->cursor-1], &f->buffer[f->cursor], strlen( &f->buffer[f->cursor] ) + 1 );
			f->cursor--;

			if ( f->visible_offset )
			{
				f->visible_offset--;
			}
		}
		break;

	case K_KP_DEL:
	case K_DEL:
		memmove( &f->buffer[f->cursor], &f->buffer[f->cursor+1], strlen( &f->buffer[f->cursor+1] ) + 1 );
		break;

	case K_KP_ENTER:
	case K_ENTER:
	case K_ESCAPE:
	case K_TAB:
		return false;

	case K_SPACE:
	default:
		if ( !isdigit( key ) && ( f->generic.flags & QMF_NUMBERSONLY ) )
			return false;

		if ( f->cursor < f->length )
		{
			f->buffer[f->cursor++] = key;
			f->buffer[f->cursor] = 0;

			if ( f->cursor > f->visible_length )
			{
				f->visible_offset++;
			}
		}
	}

	return true;
}

void Menu_AddItem( menuframework_t *menu, void *item )
{
	if ( menu->nitems == 0 )
		menu->nslots = 0;

	if ( menu->nitems < MAXMENUITEMS )
	{
		menu->items[menu->nitems] = item;
		( ( menucommon_t * ) menu->items[menu->nitems] )->parent = menu;
		menu->nitems++;
	}

	menu->nslots = Menu_TallySlots( menu );
}

/*
** Menu_AdjustCursor
**
** This function takes the given menu, the direction, and attempts
** to adjust the menu's cursor so that it's at the next available
** slot.
*/
void Menu_AdjustCursor( menuframework_t *m, int dir )
{
	menucommon_t *citem;

	/*
	** see if it's in a valid spot
	*/
	if ( m->cursor >= 0 && m->cursor < m->nitems )
	{
		if ( ( citem = Menu_ItemAtCursor( m ) ) != 0 )
		{
			if ( citem->type != MTYPE_SEPARATOR )
				return;
		}
	}

	/*
	** it's not in a valid spot, so crawl in the direction indicated until we
	** find a valid spot
	*/
	if ( dir == 1 )
	{
		while ( 1 )
		{
			citem = Menu_ItemAtCursor( m );
			if ( citem )
				if ( citem->type != MTYPE_SEPARATOR )
					break;
			m->cursor += dir;
			if ( m->cursor >= m->nitems )
				m->cursor = 0;
		}
	}
	else
	{
		while ( 1 )
		{
			citem = Menu_ItemAtCursor( m );
			if ( citem )
				if ( citem->type != MTYPE_SEPARATOR )
					break;
			m->cursor += dir;
			if ( m->cursor < 0 )
				m->cursor = m->nitems - 1;
		}
	}
}

void Menu_Center( menuframework_t *menu )
{
	int height;

	height = ( ( menucommon_t * ) menu->items[menu->nitems-1])->y;
	height += 10;

	menu->y = ( VID_HEIGHT - height ) / 2;
}

void Menu_Draw( menuframework_t *menu )
{
	int i;
	menucommon_t *item;

	/*
	** draw contents
	*/
	for ( i = 0; i < menu->nitems; i++ )
	{
		switch ( ( ( menucommon_t * ) menu->items[i] )->type )
		{
		case MTYPE_FIELD:
			Field_Draw( ( menufield_t * ) menu->items[i] );
			break;
		case MTYPE_SLIDER:
			Slider_Draw( ( menuslider_t * ) menu->items[i] );
			break;
		case MTYPE_LIST:
			MenuList_Draw( ( menulist_t * ) menu->items[i] );
			break;
		case MTYPE_SPINCONTROL:
			SpinControl_Draw( ( menulist_t * ) menu->items[i] );
			break;
		case MTYPE_ACTION:
			Action_Draw( ( menuaction_t * ) menu->items[i] );
			break;
		case MTYPE_SEPARATOR:
			Separator_Draw( ( menuseparator_t * ) menu->items[i] );
			break;
		}
	}

	item = Menu_ItemAtCursor( menu );

	if ( item && item->cursordraw )
	{
		item->cursordraw( item );
	}
	else if ( menu->cursordraw )
	{
		menu->cursordraw( menu );
	}
	else if ( item && item->type != MTYPE_FIELD )
	{
		if ( item->flags & QMF_LEFT_JUSTIFY )
		{
			Draw_Char( menu->x + item->x - 24*vid_hudscale->value + item->cursor_offset*vid_hudscale->value, menu->y + item->y, 12 + ( ( int ) ( Sys_Milliseconds()/250 ) & 1 ) );
		}
		else
		{
			Draw_Char( menu->x + item->cursor_offset*vid_hudscale->value, menu->y + item->y, 12 + ( ( int ) ( Sys_Milliseconds()/250 ) & 1 ) );
		}
	}

	if ( item )
	{
		if ( item->statusbarfunc )
			item->statusbarfunc( ( void * ) item );
		else if ( item->statusbar )
			Menu_DrawStatusBar( item->statusbar );
		else
			Menu_DrawStatusBar( menu->statusbar );

	}
	else
	{
		Menu_DrawStatusBar( menu->statusbar );
	}
}

void Menu_DrawStatusBar( const char *string )
{
	int size_x = 0, size_y = 0;
	font_t* system_font_ptr = Font_GetByName(cl_system_font->string);

	if ( string )
	{
		Text_GetSize(cl_system_font->string, &size_x, &size_y, string);
		int l = (int)strlen( string );
		int maxcol = VID_WIDTH / (8*vid_hudscale->value);
		int col = maxcol / 2 - l / 2;

		Draw_Fill( 0, (VID_HEIGHT-system_font_ptr->line_height)*vid_hudscale->value, VID_WIDTH, 8*vid_hudscale->value, 63, 63, 63, 255 );

		Text_Draw(cl_system_font->string, col*size_x*vid_hudscale->value, VID_HEIGHT - size_y*vid_hudscale->value, string );
	}
	else
	{
		Draw_Fill( 0, VID_HEIGHT-8*vid_hudscale->value, VID_WIDTH, 8*vid_hudscale->value, 0, 0, 0, 255 );
	}
}

void *Menu_ItemAtCursor( menuframework_t *m )
{
	if ( m->cursor < 0 || m->cursor >= m->nitems )
		return 0;

	return m->items[m->cursor];
}

qboolean Menu_SelectItem( menuframework_t *s )
{
	menucommon_t *item = ( menucommon_t * ) Menu_ItemAtCursor( s );

	if ( item )
	{
		switch ( item->type )
		{
		case MTYPE_FIELD:
			return Field_DoEnter( ( menufield_t * ) item ) ;
		case MTYPE_ACTION:
			Action_DoEnter( ( menuaction_t * ) item );
			return true;
		}
	}
	return false;
}

void Menu_SetStatusBar( menuframework_t *m, const char *string )
{
	m->statusbar = string;
}

void Menu_SlideItem( menuframework_t *s, int dir )
{
	menucommon_t *item = ( menucommon_t * ) Menu_ItemAtCursor( s );

	if ( item )
	{
		switch ( item->type )
		{
		case MTYPE_SLIDER:
			Slider_DoSlide( ( menuslider_t * ) item, dir );
			break;
		case MTYPE_SPINCONTROL:
			SpinControl_DoSlide( ( menulist_t * ) item, dir );
			break;
		}
	}
}

int Menu_TallySlots( menuframework_t *menu )
{
	int i;
	int total = 0;

	for ( i = 0; i < menu->nitems; i++ )
	{
		if ( ( ( menucommon_t * ) menu->items[i] )->type == MTYPE_LIST )
		{
			int nitems = 0;
			const char **n = ( ( menulist_t * ) menu->items[i] )->itemnames;

			while (*n)
				nitems++, n++;

			total += nitems;
		}
		else
		{
			total++;
		}
	}

	return total;
}

void Menulist_DoEnter( menulist_t *l )
{
	int start;

	start = l->generic.y / 10 + 1;

	l->curvalue = l->generic.parent->cursor - start;

	if ( l->generic.callback )
		l->generic.callback( l );
}

void MenuList_Draw( menulist_t *l )
{
	const char **n;
	int y = 0;
	int size_x = 0, size_y = 0;

	Text_Draw(cl_system_font->string, l->generic.x + l->generic.parent->x + LCOLUMN_OFFSET, l->generic.y + l->generic.parent->y, l->generic.name);

	n = l->itemnames;

  	Draw_Fill( l->generic.x - 112 + l->generic.parent->x, l->generic.parent->y + l->generic.y + l->curvalue*10 + 10, 128, 10, 99, 76, 35, 255);
	while ( *n )
	{
		Text_GetSize(cl_system_font->string, &size_x, &size_y, *n);
		Text_Draw(cl_system_font->string, l->generic.x + l->generic.parent->x + LCOLUMN_OFFSET - size_x, l->generic.y + l->generic.parent->y + y + 10, *n );

		n++;
		y += 10;
	}
}

void Separator_Draw( menuseparator_t *s )
{
	int size_x = 0, size_y = 0; // emulate original code R2L drawing

	if (s->generic.name)
	{
		Text_GetSize(cl_system_font->string, &size_x, &size_y, s->generic.name);
		Text_Draw(cl_system_font->string, s->generic.x + s->generic.parent->x - size_x, s->generic.y + s->generic.parent->y, s->generic.name);
	}

}

void Slider_DoSlide( menuslider_t *s, int dir )
{
	s->curvalue += dir;

	if ( s->curvalue > s->maxvalue )
		s->curvalue = s->maxvalue;
	else if ( s->curvalue < s->minvalue )
		s->curvalue = s->minvalue;

	if ( s->generic.callback )
		s->generic.callback( s );
}

#define SLIDER_RANGE 10

void Slider_Draw( menuslider_t *s )
{
	int	i;
	int size_x = 0, size_y = 0; // emulate original code R2L drawing

	Text_GetSize(cl_system_font->string, &size_x, &size_y, s->generic.name);
	Text_Draw(cl_system_font->string, s->generic.x + s->generic.parent->x + LCOLUMN_OFFSET - size_x,
		                s->generic.y + s->generic.parent->y, 
						s->generic.name );

	s->range = ( s->curvalue - s->minvalue ) / ( float ) ( s->maxvalue - s->minvalue );

	if ( s->range < 0)
		s->range = 0;
	if ( s->range > 1)
		s->range = 1;
	Draw_Char( s->generic.x + s->generic.parent->x + RCOLUMN_OFFSET, s->generic.y + s->generic.parent->y, 128);
	for ( i = 0; i < SLIDER_RANGE; i++ )
		Draw_Char( RCOLUMN_OFFSET + s->generic.x + i*8*vid_hudscale->value + s->generic.parent->x + 8*vid_hudscale->value, s->generic.y + s->generic.parent->y, 129);
	Draw_Char( RCOLUMN_OFFSET + s->generic.x + i*8*vid_hudscale->value + s->generic.parent->x + 8*vid_hudscale->value, s->generic.y + s->generic.parent->y, 130);
	Draw_Char( ( int ) ( 8*vid_hudscale->value + RCOLUMN_OFFSET + s->generic.parent->x + s->generic.x + (SLIDER_RANGE-1)*8*vid_hudscale->value * s->range ), s->generic.y + s->generic.parent->y, 131);
}

void SpinControl_DoEnter( menulist_t *s )
{
	s->curvalue++;
	if ( s->itemnames[s->curvalue] == 0 )
		s->curvalue = 0;

	if ( s->generic.callback )
		s->generic.callback( s );
}

void SpinControl_DoSlide( menulist_t *s, int dir )
{
	s->curvalue += dir;

	if ( s->curvalue < 0 )
		s->curvalue = 0;
	else if ( s->itemnames[s->curvalue] == 0 )
		s->curvalue--;

	if ( s->generic.callback )
		s->generic.callback( s );
}

void SpinControl_Draw( menulist_t *s )
{
	char buffer[100];
	int size_x = 0, size_y = 0;
	if ( s->generic.name )
	{
		Text_GetSize(cl_system_font->string, &size_x, &size_y, s->generic.name);
		Text_Draw(cl_system_font->string, s->generic.x + s->generic.parent->x + LCOLUMN_OFFSET - size_x,
			s->generic.y + s->generic.parent->y,
			s->generic.name);
	}
	if ( !strchr( s->itemnames[s->curvalue], '\n' ) )
	{
		Text_Draw(cl_system_font->string, RCOLUMN_OFFSET + s->generic.x + s->generic.parent->x, s->generic.y + s->generic.parent->y, s->itemnames[s->curvalue] );
	}
	else
	{
		// old code also did this hack
		strcpy( buffer, s->itemnames[s->curvalue] );
		*strchr( buffer, '\n' ) = 0;
		Text_Draw(cl_system_font->string, RCOLUMN_OFFSET + s->generic.x + s->generic.parent->x, s->generic.y + s->generic.parent->y, buffer );
		strcpy( buffer, strchr( s->itemnames[s->curvalue], '\n' ) + 1 );
		Text_Draw(cl_system_font->string, RCOLUMN_OFFSET + s->generic.x + s->generic.parent->x, s->generic.y + s->generic.parent->y + 10 * vid_hudscale->value, buffer );
	}
}

