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

#include <util/shared.h>


/*
============
COM_SkipPath
============
*/
char* COM_SkipPath(char* pathname)
{
	char* last;

	last = pathname;
	while (*pathname)
	{
		if (*pathname == '/')
			last = pathname + 1;
		pathname++;
	}
	return last;
}

/*
============
COM_StripExtension
============
*/
void COM_StripExtension(char* in, char* out)
{
	while (*in && *in != '.')
		*out++ = *in++;
	*out = 0;
}

/*
============
COM_FileExtension
============
*/
char* COM_FileExtension(char* in)
{
	static char exten[8];
	int32_t 	i;

	while (*in && *in != '.')
		in++;
	if (!*in)
		return "";
	in++;
	for (i = 0; i < 7 && *in; i++, in++)
		exten[i] = *in;
	exten[i] = 0;
	return exten;
}

/*
============
COM_FileBase
============
*/
void COM_FileBase(char* in, char* out)
{
	char* s, * s2;

	s = in + strlen(in) - 1;

	while (s != in && *s != '.')
		s--;

	for (s2 = s; s2 != in && *s2 != '/'; s2--)
		;

	if (s - s2 < 2)
		out[0] = 0;
	else
	{
		s--;
		strncpy(out, s2 + 1, s - s2);
		out[s - s2] = 0;
	}
}

/*
============
COM_FilePath

Returns the path up to, but not including the last /
============
*/
void COM_FilePath(char* in, char* out)
{
	char* s;

	s = in + strlen(in) - 1;

	while (s != in && *s != '/')
		s--;

	strncpy(out, in, s - in);
	out[s - in] = 0;
}


/*
==================
COM_DefaultExtension
==================
*/
void COM_DefaultExtension(char* path, char* extension)
{
	char* src;
	//
	// if path doesn't have a .EXT, append extension
	// (extension should include the .)
	//
	src = path + strlen(path) - 1;

	while (*src != '/' && src != path)
	{
		if (*src == '.')
			return;                 // it has an extension
		src--;
	}

	strcat(path, extension);
}



char com_token[MAX_TOKEN_CHARS];

/*
==============
COM_Parse

Parse a token out of a string
==============
*/
char* COM_Parse(char** data_p)
{
	int32_t c;
	int32_t len;
	char* data;

	data = *data_p;
	len = 0;
	com_token[0] = 0;

	if (!data)
	{
		*data_p = NULL;
		return "";
	}

	// skip whitespace
skipwhite:
	while ((c = *data) <= ' ')
	{
		if (c == 0)
		{
			*data_p = NULL;
			return "";
		}
		data++;
	}

	// skip // comments
	if (c == '/' && data[1] == '/')
	{
		while (*data && *data != '\n')
			data++;
		goto skipwhite;
	}

	// handle quoted strings specially
	if (c == '\"')
	{
		data++;
		while (1)
		{
			c = *data++;
			if (c == '\"' || !c)
			{
				com_token[len] = 0;
				*data_p = data;
				return com_token;
			}
			if (len < MAX_TOKEN_CHARS)
			{
				com_token[len] = c;
				len++;
			}
		}
	}

	// parse a regular word
	do
	{
		if (len < MAX_TOKEN_CHARS)
		{
			com_token[len] = c;
			len++;
		}
		data++;
		c = *data;
	} 
	while (c > 32);

	if (len == MAX_TOKEN_CHARS)
	{
		//		Com_Printf ("Token exceeded %i chars, discarded.\n", MAX_TOKEN_CHARS);
		len = 0;
	}
	com_token[len] = 0;

	*data_p = data;
	return com_token;
}


/*
===============
Com_PageInMemory

===============
*/
int32_t paged_total;

void Com_PageInMemory(uint8_t* buffer, int32_t size)
{
	int32_t 	i;

	for (i = size - 1; i > 0; i -= 4096)
		paged_total += buffer[i];
}



/*
=====================================================================

  INFO STRINGS

=====================================================================
*/

/*
===============
Info_ValueForKey

Searches the string for the given
key and returns the associated value, or an empty string.
===============
*/
char* Info_ValueForKey(char* s, char* key)
{
	char pkey[512] = { 0 };
	static char value[2][512];	// use two buffers so compares
	// work without stomping on each other
	static int32_t valueindex;
	char* o;

	valueindex ^= 1;
	if (*s == '\\')
		s++;
	while (1)
	{
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value[valueindex];

		while (*s != '\\' && *s)
		{
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;

		if (!strcmp(key, pkey))
			return value[valueindex];

		if (!*s)
			return "";
		s++;
	}
}

void Info_RemoveKey(char* s, char* key)
{
	char* start;
	char pkey[512];
	char value[512];
	char* o;

	if (strstr(key, "\\"))
	{
		//		Com_Printf ("Can't use a key with a \\\n");
		return;
	}

	while (1)
	{
		start = s;
		if (*s == '\\')
			s++;
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while (*s != '\\' && *s)
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;

		if (!strcmp(key, pkey))
		{
			strcpy(start, s);	// remove this part
			return;
		}

		if (!*s)
			return;
	}

}


/*
==================
Info_Validate

Some characters are illegal in info strings because they
can mess up the server's parsing
==================
*/
bool Info_Validate(char* s)
{
	if (strstr(s, "\""))
		return false;
	if (strstr(s, ";"))
		return false;
	return true;
}

void Info_SetValueForKey(char* s, char* key, char* value)
{
	char	newi[MAX_INFO_STRING], * v;
	int32_t c;
	int32_t maxsize = MAX_INFO_STRING;

	if (strstr(key, "\\") || strstr(value, "\\"))
	{
		Com_Printf("Can't use keys or values with a \\\n");
		return;
	}

	if (strstr(key, ";"))
	{
		Com_Printf("Can't use keys or values with a semicolon\n");
		return;
	}

	if (strstr(key, "\"") || strstr(value, "\""))
	{
		Com_Printf("Can't use keys or values with a \"\n");
		return;
	}

	if (strlen(key) > MAX_INFO_KEY - 1 || strlen(value) > MAX_INFO_KEY - 1)
	{
		Com_Printf("Keys and values must be < 64 characters.\n");
		return;
	}
	Info_RemoveKey(s, key);
	if (!value || !strlen(value))
		return;

	snprintf(newi, sizeof(newi), "\\%s\\%s", key, value);

	if (strlen(newi) + strlen(s) > maxsize)
	{
		Com_Printf("Info string length exceeded\n");
		return;
	}

	// only copy ascii values
	s += strlen(s);
	v = newi;
	while (*v)
	{
		c = *v++;
		c &= 127;		// strip high bits
		if (c >= 32 && c < 127)
			*s++ = c;
	}
	*s = 0;
}

//====================================================================