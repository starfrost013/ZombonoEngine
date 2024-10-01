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
// common.c -- misc functions used in client and server
#include "common.h"
#include <setjmp.h>

#define	MAXPRINTMSG	8192

#define MAX_NUM_ARGVS 50

int32_t com_argc;
char* com_argv[MAX_NUM_ARGVS + 1];

int32_t realtime;

jmp_buf abortframe;		// an ERR_DROP occured, exit the entire frame
static bool shutdown_game = false;

FILE* log_stats_file;

// profiling cvars
cvar_t* profile_all;

cvar_t* log_stats;
cvar_t* log_memalloc;
cvar_t* developer;
cvar_t* timescale;
cvar_t* fixedtime;
cvar_t* logfile_active;	// 1 = buffer log, 2 = flush after each print, 3 = append
cvar_t* showtrace;
cvar_t* dedicated;
cvar_t* engine_version;
cvar_t* debug_console;		// debug console toggle for Windows

FILE* logfile;

int32_t server_state;

// host_speeds times
int64_t time_before_game;
int64_t time_after_game;
int64_t time_before_ref;
int64_t time_after_ref;

#if DEBUG
cvar_t* hunk_total;
cvar_t* hunk_areas;
#endif

/*
============================================================================

CLIENT / SERVER interactions

============================================================================
*/

static int32_t rd_target;
static char* rd_buffer;
static int32_t rd_buffersize;
static void	(*rd_flush)(int32_t target, char* buffer);

extern void SV_ShutdownGameProgs();

void Com_BeginRedirect(int32_t target, char* buffer, int32_t buffersize, void(*flush))
{
	if (!target || !buffer || !buffersize || !flush)
		return;
	rd_target = target;
	rd_buffer = buffer;
	rd_buffersize = buffersize;
	rd_flush = flush;

	*rd_buffer = 0;
}

void Com_EndRedirect()
{
	rd_flush(rd_target, rd_buffer);

	rd_target = 0;
	rd_buffer = NULL;
	rd_buffersize = 0;
	rd_flush = NULL;
}

/*
=============
Com_Printf

Both client and server can use this, and it will output
to the apropriate place.
=============
*/
void Com_Printf(char* fmt, ...)
{
	va_list	 argptr;
	char	 msg[MAXPRINTMSG];

	va_start(argptr, fmt);
	vsnprintf(msg, MAXPRINTMSG, fmt, argptr);
	va_end(argptr);

	if (rd_target)
	{
		if ((strlen(msg) + strlen(rd_buffer)) > (rd_buffersize - 1))
		{
			rd_flush(rd_target, rd_buffer);
			*rd_buffer = 0;
		}
		strcat(rd_buffer, msg);
		return;
	}

	Con_Print(msg);

	// also echo to debugging console
	Sys_ConsoleOutput(msg);

	// logfile
	if (logfile_active && logfile_active->value)
	{
		char name[MAX_QPATH];

		if (!logfile)
		{
			snprintf(name, sizeof(name), "%s/qconsole.log", FS_Gamedir());
			if (logfile_active->value > 2)
				logfile = fopen(name, "a");
			else
				logfile = fopen(name, "w");
		}
		if (logfile)
			fprintf(logfile, "%s", msg);
		if (logfile_active->value > 1)
			fflush(logfile);		// force it to save every time
	}
}


/*
================
Com_DPrintf

A Com_Printf that only shows up if the "developer" cvar is set
================
*/
void Com_DPrintf(char* fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	if (!developer || !developer->value)
		return;			// don't confuse non-developers with techie stuff...

	va_start(argptr, fmt);
	vsnprintf(msg, MAXPRINTMSG, fmt, argptr);
	va_end(argptr);

	Com_Printf("%s", msg);
}

/*
=============
Com_Error

Both client and server can use this, and it will
do the apropriate things.
=============
*/
void Com_Error(int32_t code, char* fmt, ...)
{
	va_list		argptr;
	static char	msg[MAXPRINTMSG];
	static bool recursive;

	if (recursive)
		Sys_Error("recursive error after: %s", msg);
	recursive = true;

	va_start(argptr, fmt);
	vsnprintf(msg, MAXPRINTMSG, fmt, argptr);
	va_end(argptr);

	if (code == ERR_DISCONNECT)
	{
		CL_Drop();
		recursive = false;
		longjmp(abortframe, -1);
	}
	else if (code == ERR_DROP)
	{
		Com_Printf("********************\nERROR: %s\n********************\n", msg);
		SV_Shutdown(va("Server crashed: %s\n", msg), false);
		CL_Drop();
		recursive = false;
		shutdown_game = true;
		longjmp(abortframe, -1);
	}
	else
	{
		SV_Shutdown(va("Server fatal crashed: %s\n", msg), false);
		SV_ShutdownGameProgs();
		CL_Shutdown();
	}

	if (logfile)
	{
		fclose(logfile);
		logfile = NULL;
	}

	Sys_Error("%s", msg);
}

/*
=============
Com_Quit

Both client and server can use this, and it will
do the apropriate things.
=============
*/
void Com_Quit()
{
	SV_Shutdown("Server quit\n", false);
	SV_ShutdownGameProgs();
	CL_Shutdown();

	if (logfile)
	{
		fclose(logfile);
		logfile = NULL;
	}

	Sys_Quit();
}

/*
==================
Com_ServerState
==================
*/
int32_t Com_GetServerState()
{
	return server_state;
}

/*
==================
Com_SetServerState
==================
*/
void Com_SetServerState(int32_t state)
{
	server_state = state;
}

/*
==============================================================================

			MESSAGE IO FUNCTIONS

Handles byte ordering and avoids alignment errors
==============================================================================
*/

vec3_t	bytedirs[NUM_VERTEX_NORMALS] =
{
#include <client/include/anorms.h>
};

//
// writing functions
//

void MSG_WriteChar(sizebuf_t* sb, int32_t c)
{
	MSG_WriteByte(sb, c);
}

void MSG_WriteByte(sizebuf_t* sb, int32_t c)
{
	uint8_t* buf;

	buf = SZ_GetSpace(sb, 1);
	buf[0] = c;
}

void MSG_WriteShort(sizebuf_t* sb, int32_t c)
{
	uint8_t* buf;

	buf = SZ_GetSpace(sb, 2);
	buf[0] = c & 0xff;
	buf[1] = c >> 8;
}

void MSG_WriteInt(sizebuf_t* sb, int32_t c)
{
	uint8_t* buf;

	buf = SZ_GetSpace(sb, 4);
	buf[0] = c & 0xff;
	buf[1] = (c >> 8) & 0xff;
	buf[2] = (c >> 16) & 0xff;
	buf[3] = c >> 24;
}

void MSG_WriteFloat(sizebuf_t* sb, float f)
{
	union
	{
		float	f;
		int32_t l;
	} dat;


	dat.f = f;
	dat.l = LittleInt(dat.l);

	SZ_Write(sb, &dat.l, 4);
}

void MSG_WriteString(sizebuf_t* sb, char* s)
{
	if (!s)
		SZ_Write(sb, "", 1);
	else
		SZ_Write(sb, s, (int32_t)strlen(s) + 1);
}

void MSG_WriteCoord(sizebuf_t* sb, float f)
{
	MSG_WriteFloat(sb, f);
}

void MSG_WritePos(sizebuf_t* sb, vec3_t pos)
{
	MSG_WriteFloat(sb, pos[0]);
	MSG_WriteFloat(sb, pos[1]);
	MSG_WriteFloat(sb, pos[2]);
}

void MSG_WriteAngle(sizebuf_t* sb, float f)
{
	MSG_WriteByte(sb, (int32_t)(f * 256 / 360) & 255);
}

void MSG_WriteAngle16(sizebuf_t* sb, float f)
{
	MSG_WriteShort(sb, ANGLE2SHORT(f));
}


void MSG_WriteDeltaUsercmd(sizebuf_t* buf, usercmd_t* from, usercmd_t* cmd)
{
	int32_t 	bits;

	//
	// send the movement message
	//
	bits = 0;
	if (cmd->angles[0] != from->angles[0])
		bits |= CM_ANGLE1;
	if (cmd->angles[1] != from->angles[1])
		bits |= CM_ANGLE2;
	if (cmd->angles[2] != from->angles[2])
		bits |= CM_ANGLE3;
	if (cmd->forwardmove != from->forwardmove)
		bits |= CM_FORWARD;
	if (cmd->sidemove != from->sidemove)
		bits |= CM_SIDE;
	if (cmd->upmove != from->upmove)
		bits |= CM_UP;
	if (cmd->buttons != from->buttons)
		bits |= CM_BUTTONS;
	if (cmd->impulse != from->impulse)
		bits |= CM_IMPULSE;

	MSG_WriteByte(buf, bits);

	if (bits & CM_ANGLE1)
		MSG_WriteShort(buf, cmd->angles[0]);
	if (bits & CM_ANGLE2)
		MSG_WriteShort(buf, cmd->angles[1]);
	if (bits & CM_ANGLE3)
		MSG_WriteShort(buf, cmd->angles[2]);

	if (bits & CM_FORWARD)
		MSG_WriteShort(buf, cmd->forwardmove);
	if (bits & CM_SIDE)
		MSG_WriteShort(buf, cmd->sidemove);
	if (bits & CM_UP)
		MSG_WriteShort(buf, cmd->upmove);

	if (bits & CM_BUTTONS)
		MSG_WriteByte(buf, cmd->buttons);
	if (bits & CM_IMPULSE)
		MSG_WriteByte(buf, cmd->impulse);

	MSG_WriteByte(buf, cmd->msec);
	MSG_WriteByte(buf, cmd->lightlevel);
}


void MSG_WriteDir(sizebuf_t* sb, vec3_t dir)
{
	int32_t 	i, best;
	float	d, bestd;

	if (!dir)
	{
		MSG_WriteByte(sb, 0);
		return;
	}

	bestd = 0;
	best = 0;
	for (i = 0; i < NUM_VERTEX_NORMALS; i++)
	{
		d = DotProduct3(dir, bytedirs[i]);
		if (d > bestd)
		{
			bestd = d;
			best = i;
		}
	}
	MSG_WriteByte(sb, best);
}

void MSG_WriteColor(sizebuf_t* msg_read, color4_t color)
{
	MSG_WriteByte(msg_read, color[0]);
	MSG_WriteByte(msg_read, color[1]);
	MSG_WriteByte(msg_read, color[2]);
	MSG_WriteByte(msg_read, color[3]);
}

void MSG_ReadDir(sizebuf_t* sb, vec3_t dir)
{
	int32_t 	b;

	b = MSG_ReadByte(sb);
	if (b >= NUM_VERTEX_NORMALS)
		Com_Error(ERR_DROP, "MSF_ReadDir: out of range");

	VectorCopy3(bytedirs[b], dir);
}


/*
==================
MSG_WriteDeltaEntity

Writes part of a packetentities message.
Can delta from either a baseline or a previous packet_entity
==================
*/
void MSG_WriteDeltaEntity(entity_state_t* from, entity_state_t* to, sizebuf_t* msg, bool force, bool newentity)
{
	int32_t 	bits;

	if (!to->number)
		Com_Error(ERR_FATAL, "Unset entity number");
	if (to->number >= MAX_EDICTS)
		Com_Error(ERR_FATAL, "Entity number >= MAX_EDICTS");

	// send an update
	bits = 0;

	if (to->number >= 256)
		bits |= U_NUMBER16;		// number8 is implicit otherwise

	if (to->origin[0] != from->origin[0])
		bits |= U_ORIGIN1;
	if (to->origin[1] != from->origin[1])
		bits |= U_ORIGIN2;
	if (to->origin[2] != from->origin[2])
		bits |= U_ORIGIN3;

	if (to->angles[0] != from->angles[0])
		bits |= U_ANGLE1;
	if (to->angles[1] != from->angles[1])
		bits |= U_ANGLE2;
	if (to->angles[2] != from->angles[2])
		bits |= U_ANGLE3;

	if (to->skinnum != from->skinnum)
	{
		if ((uint32_t)to->skinnum < 256)
			bits |= U_SKIN8;
		else if ((uint32_t)to->skinnum < 0x10000)
			bits |= U_SKIN16;
		else
			bits |= (U_SKIN8 | U_SKIN16);
	}

	if (to->frame != from->frame)
	{
		if (to->frame < 256)
			bits |= U_FRAME8;
		else
			bits |= U_FRAME16;
	}

	if (to->effects != from->effects)
	{
		if (to->effects < 256)
			bits |= U_EFFECTS8;
		else if (to->effects < 0x8000)
			bits |= U_EFFECTS16;
		else
			bits |= U_EFFECTS8 | U_EFFECTS16;
	}

	if (to->renderfx != from->renderfx)
	{
		if (to->renderfx < 256)
			bits |= U_RENDERFX8;
		else if (to->renderfx < 0x8000)
			bits |= U_RENDERFX16;
		else
			bits |= U_RENDERFX8 | U_RENDERFX16;
	}

	if (to->solid != from->solid)
		bits |= U_SOLID;

	// event is not delta compressed, just 0 compressed
	if (to->event)
		bits |= U_EVENT;

	if (to->modelindex != from->modelindex)
		bits |= U_MODEL;
	if (to->modelindex2 != from->modelindex2)
		bits |= U_MODEL2;
	if (to->modelindex3 != from->modelindex3)
		bits |= U_MODEL3;
	if (to->modelindex4 != from->modelindex4)
		bits |= U_MODEL4;

	if (to->sound != from->sound)
		bits |= U_SOUND;

	if (newentity || (to->renderfx & RF_BEAM))
		bits |= U_OLDORIGIN;

	//
	// write the message
	//
	if (!bits && !force)
		return;		// nothing to send!

	//----------

	if (bits & 0xff000000)
		bits |= U_MOREBITS3 | U_MOREBITS2 | U_MOREBITS1;
	else if (bits & 0x00ff0000)
		bits |= U_MOREBITS2 | U_MOREBITS1;
	else if (bits & 0x0000ff00)
		bits |= U_MOREBITS1;

	MSG_WriteByte(msg, bits & 255);

	if (bits & 0xff000000)
	{
		MSG_WriteByte(msg, (bits >> 8) & 255);
		MSG_WriteByte(msg, (bits >> 16) & 255);
		MSG_WriteByte(msg, (bits >> 24) & 255);
	}
	else if (bits & 0x00ff0000)
	{
		MSG_WriteByte(msg, (bits >> 8) & 255);
		MSG_WriteByte(msg, (bits >> 16) & 255);
	}
	else if (bits & 0x0000ff00)
	{
		MSG_WriteByte(msg, (bits >> 8) & 255);
	}

	//----------

	if (bits & U_NUMBER16)
		MSG_WriteShort(msg, to->number);
	else
		MSG_WriteByte(msg, to->number);

	if (bits & U_MODEL)
		MSG_WriteByte(msg, to->modelindex);
	if (bits & U_MODEL2)
		MSG_WriteByte(msg, to->modelindex2);
	if (bits & U_MODEL3)
		MSG_WriteByte(msg, to->modelindex3);
	if (bits & U_MODEL4)
		MSG_WriteByte(msg, to->modelindex4);

	if (bits & U_FRAME8)
		MSG_WriteByte(msg, to->frame);
	if (bits & U_FRAME16)
		MSG_WriteShort(msg, to->frame);

	if ((bits & U_SKIN8) && (bits & U_SKIN16))		//used for laser colors
		MSG_WriteInt(msg, to->skinnum);
	else if (bits & U_SKIN8)
		MSG_WriteByte(msg, to->skinnum);
	else if (bits & U_SKIN16)
		MSG_WriteShort(msg, to->skinnum);


	if ((bits & (U_EFFECTS8 | U_EFFECTS16)) == (U_EFFECTS8 | U_EFFECTS16))
		MSG_WriteInt(msg, to->effects);
	else if (bits & U_EFFECTS8)
		MSG_WriteByte(msg, to->effects);
	else if (bits & U_EFFECTS16)
		MSG_WriteShort(msg, to->effects);

	if ((bits & (U_RENDERFX8 | U_RENDERFX16)) == (U_RENDERFX8 | U_RENDERFX16))
		MSG_WriteInt(msg, to->renderfx);
	else if (bits & U_RENDERFX8)
		MSG_WriteByte(msg, to->renderfx);
	else if (bits & U_RENDERFX16)
		MSG_WriteShort(msg, to->renderfx);

	if (bits & U_ORIGIN1)
		MSG_WriteCoord(msg, to->origin[0]);
	if (bits & U_ORIGIN2)
		MSG_WriteCoord(msg, to->origin[1]);
	if (bits & U_ORIGIN3)
		MSG_WriteCoord(msg, to->origin[2]);

	if (bits & U_ANGLE1)
		MSG_WriteAngle(msg, to->angles[0]);
	if (bits & U_ANGLE2)
		MSG_WriteAngle(msg, to->angles[1]);
	if (bits & U_ANGLE3)
		MSG_WriteAngle(msg, to->angles[2]);

	if (bits & U_OLDORIGIN)
	{
		MSG_WriteCoord(msg, to->old_origin[0]);
		MSG_WriteCoord(msg, to->old_origin[1]);
		MSG_WriteCoord(msg, to->old_origin[2]);
	}

	if (bits & U_SOUND)
		MSG_WriteByte(msg, to->sound);
	if (bits & U_EVENT)
		MSG_WriteByte(msg, to->event);
	if (bits & U_SOLID)
		MSG_WriteShort(msg, to->solid);
}


//============================================================

//
// reading functions
//

void MSG_BeginReading(sizebuf_t* msg)
{
	msg->readcount = 0;
}

// returns -1 if no more characters are available
int32_t MSG_ReadChar(sizebuf_t* msg_read)
{
	int32_t c;

	if (msg_read->readcount + 1 > msg_read->cursize)
		c = -1;
	else
		c = (signed char)msg_read->data[msg_read->readcount];
	msg_read->readcount++;

	return c;
}

int32_t MSG_ReadByte(sizebuf_t* msg_read)
{
	int32_t c;

	if (msg_read->readcount + 1 > msg_read->cursize)
		c = -1;
	else
		c = (uint8_t)msg_read->data[msg_read->readcount];
	msg_read->readcount++;

	return c;
}

int32_t MSG_ReadShort(sizebuf_t* msg_read)
{
	int32_t c;

	if (msg_read->readcount + 2 > msg_read->cursize)
		c = -1;
	else
		c = (int16_t)(msg_read->data[msg_read->readcount]
			+ (msg_read->data[msg_read->readcount + 1] << 8));

	msg_read->readcount += 2;

	return c;
}

int32_t MSG_ReadInt(sizebuf_t* msg_read)
{
	int32_t c;

	if (msg_read->readcount + 4 > msg_read->cursize)
		c = -1;
	else
		c = msg_read->data[msg_read->readcount]
		+ (msg_read->data[msg_read->readcount + 1] << 8)
		+ (msg_read->data[msg_read->readcount + 2] << 16)
		+ (msg_read->data[msg_read->readcount + 3] << 24);

	msg_read->readcount += 4;

	return c;
}

float MSG_ReadFloat(sizebuf_t* msg_read)
{
	union
	{
		uint8_t	b[4];
		float	f;
		int32_t l;
	} dat;

	if (msg_read->readcount + 4 > msg_read->cursize)
		dat.f = -1;
	else
	{
		dat.b[0] = msg_read->data[msg_read->readcount];
		dat.b[1] = msg_read->data[msg_read->readcount + 1];
		dat.b[2] = msg_read->data[msg_read->readcount + 2];
		dat.b[3] = msg_read->data[msg_read->readcount + 3];
	}
	msg_read->readcount += 4;

	dat.l = LittleInt(dat.l);

	return dat.f;
}

char* MSG_ReadString(sizebuf_t* msg_read)
{
	static char	string[2048];
	int32_t 	l, c;

	l = 0;
	do
	{
		c = MSG_ReadChar(msg_read);
		if (c == -1 || c == 0)
			break;
		string[l] = c;
		l++;
	} while (l < sizeof(string) - 1);

	string[l] = 0;

	return string;
}

char* MSG_ReadStringLine(sizebuf_t* msg_read)
{
	static char	string[2048];
	int32_t 	l, c;

	l = 0;
	do
	{
		c = MSG_ReadChar(msg_read);
		if (c == -1 || c == 0 || c == '\n')
			break;
		string[l] = c;
		l++;
	} while (l < sizeof(string) - 1);

	string[l] = 0;

	return string;
}

float MSG_ReadCoord(sizebuf_t* msg_read)
{
	return MSG_ReadFloat(msg_read);
}

void MSG_ReadPos(sizebuf_t* msg_read, vec3_t pos)
{
	pos[0] = MSG_ReadFloat(msg_read);
	pos[1] = MSG_ReadFloat(msg_read);
	pos[2] = MSG_ReadFloat(msg_read);
}

void MSG_ReadColor(sizebuf_t* msg_read, color4_t color)
{
	color[0] = MSG_ReadByte(msg_read);
	color[1] = MSG_ReadByte(msg_read);
	color[2] = MSG_ReadByte(msg_read);
	color[3] = MSG_ReadByte(msg_read);
}

float MSG_ReadAngle(sizebuf_t* msg_read)
{
	return MSG_ReadChar(msg_read) * (360.0 / 256);
}

float MSG_ReadAngle16(sizebuf_t* msg_read)
{
	return SHORT2ANGLE(MSG_ReadShort(msg_read));
}

void MSG_ReadDeltaUsercmd(sizebuf_t* msg_read, usercmd_t* from, usercmd_t* move)
{
	int32_t bits;

	memcpy(move, from, sizeof(*move));

	bits = MSG_ReadByte(msg_read);

	// read current angles
	if (bits & CM_ANGLE1)
		move->angles[0] = MSG_ReadShort(msg_read);
	if (bits & CM_ANGLE2)
		move->angles[1] = MSG_ReadShort(msg_read);
	if (bits & CM_ANGLE3)
		move->angles[2] = MSG_ReadShort(msg_read);

	// read movement
	if (bits & CM_FORWARD)
		move->forwardmove = MSG_ReadShort(msg_read);
	if (bits & CM_SIDE)
		move->sidemove = MSG_ReadShort(msg_read);
	if (bits & CM_UP)
		move->upmove = MSG_ReadShort(msg_read);

	// read buttons
	if (bits & CM_BUTTONS)
		move->buttons = MSG_ReadByte(msg_read);

	if (bits & CM_IMPULSE)
		move->impulse = MSG_ReadByte(msg_read);

	// read time to run command
	move->msec = MSG_ReadByte(msg_read);

	// read the light level
	move->lightlevel = MSG_ReadByte(msg_read);
}


void MSG_ReadData(sizebuf_t* msg_read, void* data, int32_t len)
{
	int32_t 	i;

	for (i = 0; i < len; i++)
		((uint8_t*)data)[i] = MSG_ReadByte(msg_read);
}


//===========================================================================

void SZ_Init(sizebuf_t* buf, uint8_t* data, int32_t length)
{
	memset(buf, 0, sizeof(*buf));
	buf->data = data;
	buf->maxsize = length;
}

void SZ_Clear(sizebuf_t* buf)
{
	buf->cursize = 0;
	buf->overflowed = false;
}

void* SZ_GetSpace(sizebuf_t* buf, int32_t length)
{
	void* data;

	if (buf->cursize + length > buf->maxsize)
	{
		if (!buf->allowoverflow
			&& buf != NULL
			&& buf->cursize > 0)
		{
			Com_Error(ERR_FATAL, "SZ_GetSpace: overflow without allowoverflow set");
		}

		if (length > buf->maxsize)
			Com_Error(ERR_FATAL, "SZ_GetSpace: %i is > full buffer size", length);

		Com_Printf("SZ_GetSpace: overflow\n");
		SZ_Clear(buf);
		buf->overflowed = true;
	}

	data = buf->data + buf->cursize;
	buf->cursize += length;

	return data;
}

void SZ_Write(sizebuf_t* buf, void* data, int32_t length)
{
	memcpy(SZ_GetSpace(buf, length), data, length);
}

void SZ_Print(sizebuf_t* buf, char* data)
{
	int32_t 	len;

	len = (int32_t)strlen(data) + 1;

	if (buf->cursize)
	{
		if (buf->data[buf->cursize - 1])
			memcpy((uint8_t*)SZ_GetSpace(buf, len), data, len); // no trailing 0
		else
			memcpy((uint8_t*)SZ_GetSpace(buf, len - 1) - 1, data, len); // write over trailing 0
	}
	else
		memcpy((uint8_t*)SZ_GetSpace(buf, len), data, len);
}


//============================================================================


int32_t COM_Argc()
{
	return com_argc;
}

char* COM_Argv(int32_t arg)
{
	if (arg < 0 || arg >= com_argc || !com_argv[arg])
		return "";
	return com_argv[arg];
}

void COM_ClearArgv(int32_t arg)
{
	if (arg < 0 || arg >= com_argc || !com_argv[arg])
		return;
	com_argv[arg] = "";
}


/*
================
COM_InitArgv
================
*/
void COM_InitArgv(int32_t argc, char** argv)
{
	int32_t 	i;

	if (argc > MAX_NUM_ARGVS)
		Com_Error(ERR_FATAL, "argc > MAX_NUM_ARGVS");
	com_argc = argc;
	for (i = 0; i < argc; i++)
	{
		if (!argv[i] || strlen(argv[i]) >= MAX_TOKEN_CHARS)
			com_argv[i] = "";
		else
			com_argv[i] = argv[i];
	}
}


char* CopyString(char* in)
{
	char* out;

	out = Memory_ZoneMalloc((int32_t)strlen(in) + 1);
	strcpy(out, in);
	return out;
}

/*
============================================================================

					LIBRARY REPLACEMENT FUNCTIONS

============================================================================
*/

// FIXME: replace all Q_stricmp with Q_strcasecmp
int32_t Q_stricmp(char* s1, char* s2)
{
#if defined(WIN32)
	return _stricmp(s1, s2);
#else
	return strcasecmp(s1, s2);
#endif
}


int32_t Q_strncasecmp(char* s1, char* s2, int32_t n)
{
	int32_t 	c1, c2;

	do
	{
		c1 = *s1++;
		c2 = *s2++;

		if (!n--)
			return 0;		// strings are equal until end point

		if (c1 != c2)
		{
			if (c1 >= 'a' && c1 <= 'z')
				c1 -= ('a' - 'A');
			if (c2 >= 'a' && c2 <= 'z')
				c2 -= ('a' - 'A');
			if (c1 != c2)
				return -1;		// strings not equal
		}
	} while (c1);

	return 0;		// strings are equal
}

int32_t Q_strcasecmp(char* s1, char* s2)
{
	return Q_strncasecmp(s1, s2, 99999);
}

void Info_Print(char* s)
{
	char	key[512] = { 0 };
	char	value[512] = { 0 };
	char* o;
	int32_t l;

	if (*s == '\\')
		s++;
	while (*s)
	{
		o = key;
		while (*s && *s != '\\')
			*o++ = *s++;

		l = o - key;
		if (l < 20)
		{
			memset(o, ' ', 20 - l);
			key[20] = 0;
		}
		else
			*o = 0;
		Com_Printf("%s", key);

		if (!*s)
		{
			Com_Printf("MISSING VALUE\n");
			return;
		}

		o = value;
		s++;
		while (*s && *s != '\\')
			*o++ = *s++;
		*o = 0;

		if (*s)
			s++;
		Com_Printf("%s\n", value);
	}
}


/*
==============================================================================

						ZONE MEMORY ALLOCATION

just cleared malloc with counters now...

==============================================================================
*/

#define	Z_MAGIC		0x1d1d

typedef struct zhead_s
{
	struct zhead_s* prev, * next;
	short	magic;
	short	tag;			// for group free
	int32_t 	size;
} zhead_t;

zhead_t	 z_chain;
int32_t  z_count;
int32_t	 z_bytes;

/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday
============
*/
char* va(char* format, ...)
{
	va_list		argptr;
	static char	string[1024];

	va_start(argptr, format);
	vsnprintf(string, 1024, format, argptr);
	va_end(argptr);

	return string;
}


/*
========================
Z_Free
========================
*/
void Memory_ZoneFree(void* ptr)
{
	zhead_t* z;

	z = ((zhead_t*)ptr) - 1;

	if (z->magic != Z_MAGIC)
		Com_Error(ERR_FATAL, "Z_Free: bad magic");

	z->prev->next = z->next;
	z->next->prev = z->prev;

	z_count--;
	z_bytes -= z->size;
	free(z);
}


/*
========================
Z_Stats_f
========================
*/
void Memory_ZoneStats_f()
{
	Com_Printf("%i bytes in %i blocks\n", z_bytes, z_count);
}

/*
========================
Z_FreeTags
========================
*/
void Memory_ZoneFreeTags(int32_t tag)
{
	zhead_t* z, * next;

	for (z = z_chain.next; z != &z_chain; z = next)
	{
		next = z->next;
		if (z->tag == tag)
			Memory_ZoneFree((void*)(z + 1));
	}
}

/*
========================
Z_TagMalloc
========================
*/
void* Memory_ZoneMallocTagged(int32_t size, int32_t tag)
{
	zhead_t* z;

	size = size + sizeof(zhead_t);
	z = calloc(1, size);

	if (!z)
		Com_Error(ERR_FATAL, "Z_Malloc: failed on allocation of %i bytes", size);

	z_count++;
	z_bytes += size;
	z->magic = Z_MAGIC;
	z->tag = tag;
	z->size = size;

	z->next = z_chain.next;
	z->prev = &z_chain;
	z_chain.next->prev = z;
	z_chain.next = z;

	if (log_memalloc
		&& log_memalloc->value)
	{
		Com_DPrintf("Z_TagMalloc: Allocated %d bytes for tag ID %d @ 0x%0X\n", size, tag, z);
	}

	return (void*)(z + 1);
}

/*
========================
Z_Malloc
========================
*/
void* Memory_ZoneMalloc(int32_t size)
{
	return Memory_ZoneMallocTagged(size, 0);
}


static uint8_t chktbl[1024] = {
0x84, 0x47, 0x51, 0xc1, 0x93, 0x22, 0x21, 0x24, 0x2f, 0x66, 0x60, 0x4d, 0xb0, 0x7c, 0xda,
0x88, 0x54, 0x15, 0x2b, 0xc6, 0x6c, 0x89, 0xc5, 0x9d, 0x48, 0xee, 0xe6, 0x8a, 0xb5, 0xf4,
0xcb, 0xfb, 0xf1, 0x0c, 0x2e, 0xa0, 0xd7, 0xc9, 0x1f, 0xd6, 0x06, 0x9a, 0x09, 0x41, 0x54,
0x67, 0x46, 0xc7, 0x74, 0xe3, 0xc8, 0xb6, 0x5d, 0xa6, 0x36, 0xc4, 0xab, 0x2c, 0x7e, 0x85,
0xa8, 0xa4, 0xa6, 0x4d, 0x96, 0x19, 0x19, 0x9a, 0xcc, 0xd8, 0xac, 0x39, 0x5e, 0x3c, 0xf2,
0xf5, 0x5a, 0x72, 0xe5, 0xa9, 0xd1, 0xb3, 0x23, 0x82, 0x6f, 0x29, 0xcb, 0xd1, 0xcc, 0x71,
0xfb, 0xea, 0x92, 0xeb, 0x1c, 0xca, 0x4c, 0x70, 0xfe, 0x4d, 0xc9, 0x67, 0x43, 0x47, 0x94,
0xb9, 0x47, 0xbc, 0x3f, 0x01, 0xab, 0x7b, 0xa6, 0xe2, 0x76, 0xef, 0x5a, 0x7a, 0x29, 0x0b,
0x51, 0x54, 0x67, 0xd8, 0x1c, 0x14, 0x3e, 0x29, 0xec, 0xe9, 0x2d, 0x48, 0x67, 0xff, 0xed,
0x54, 0x4f, 0x48, 0xc0, 0xaa, 0x61, 0xf7, 0x78, 0x12, 0x03, 0x7a, 0x9e, 0x8b, 0xcf, 0x83,
0x7b, 0xae, 0xca, 0x7b, 0xd9, 0xe9, 0x53, 0x2a, 0xeb, 0xd2, 0xd8, 0xcd, 0xa3, 0x10, 0x25,
0x78, 0x5a, 0xb5, 0x23, 0x06, 0x93, 0xb7, 0x84, 0xd2, 0xbd, 0x96, 0x75, 0xa5, 0x5e, 0xcf,
0x4e, 0xe9, 0x50, 0xa1, 0xe6, 0x9d, 0xb1, 0xe3, 0x85, 0x66, 0x28, 0x4e, 0x43, 0xdc, 0x6e,
0xbb, 0x33, 0x9e, 0xf3, 0x0d, 0x00, 0xc1, 0xcf, 0x67, 0x34, 0x06, 0x7c, 0x71, 0xe3, 0x63,
0xb7, 0xb7, 0xdf, 0x92, 0xc4, 0xc2, 0x25, 0x5c, 0xff, 0xc3, 0x6e, 0xfc, 0xaa, 0x1e, 0x2a,
0x48, 0x11, 0x1c, 0x36, 0x68, 0x78, 0x86, 0x79, 0x30, 0xc3, 0xd6, 0xde, 0xbc, 0x3a, 0x2a,
0x6d, 0x1e, 0x46, 0xdd, 0xe0, 0x80, 0x1e, 0x44, 0x3b, 0x6f, 0xaf, 0x31, 0xda, 0xa2, 0xbd,
0x77, 0x06, 0x56, 0xc0, 0xb7, 0x92, 0x4b, 0x37, 0xc0, 0xfc, 0xc2, 0xd5, 0xfb, 0xa8, 0xda,
0xf5, 0x57, 0xa8, 0x18, 0xc0, 0xdf, 0xe7, 0xaa, 0x2a, 0xe0, 0x7c, 0x6f, 0x77, 0xb1, 0x26,
0xba, 0xf9, 0x2e, 0x1d, 0x16, 0xcb, 0xb8, 0xa2, 0x44, 0xd5, 0x2f, 0x1a, 0x79, 0x74, 0x87,
0x4b, 0x00, 0xc9, 0x4a, 0x3a, 0x65, 0x8f, 0xe6, 0x5d, 0xe5, 0x0a, 0x77, 0xd8, 0x1a, 0x14,
0x41, 0x75, 0xb1, 0xe2, 0x50, 0x2c, 0x93, 0x38, 0x2b, 0x6d, 0xf3, 0xf6, 0xdb, 0x1f, 0xcd,
0xff, 0x14, 0x70, 0xe7, 0x16, 0xe8, 0x3d, 0xf0, 0xe3, 0xbc, 0x5e, 0xb6, 0x3f, 0xcc, 0x81,
0x24, 0x67, 0xf3, 0x97, 0x3b, 0xfe, 0x3a, 0x96, 0x85, 0xdf, 0xe4, 0x6e, 0x3c, 0x85, 0x05,
0x0e, 0xa3, 0x2b, 0x07, 0xc8, 0xbf, 0xe5, 0x13, 0x82, 0x62, 0x08, 0x61, 0x69, 0x4b, 0x47,
0x62, 0x73, 0x44, 0x64, 0x8e, 0xe2, 0x91, 0xa6, 0x9a, 0xb7, 0xe9, 0x04, 0xb6, 0x54, 0x0c,
0xc5, 0xa9, 0x47, 0xa6, 0xc9, 0x08, 0xfe, 0x4e, 0xa6, 0xcc, 0x8a, 0x5b, 0x90, 0x6f, 0x2b,
0x3f, 0xb6, 0x0a, 0x96, 0xc0, 0x78, 0x58, 0x3c, 0x76, 0x6d, 0x94, 0x1a, 0xe4, 0x4e, 0xb8,
0x38, 0xbb, 0xf5, 0xeb, 0x29, 0xd8, 0xb0, 0xf3, 0x15, 0x1e, 0x99, 0x96, 0x3c, 0x5d, 0x63,
0xd5, 0xb1, 0xad, 0x52, 0xb8, 0x55, 0x70, 0x75, 0x3e, 0x1a, 0xd5, 0xda, 0xf6, 0x7a, 0x48,
0x7d, 0x44, 0x41, 0xf9, 0x11, 0xce, 0xd7, 0xca, 0xa5, 0x3d, 0x7a, 0x79, 0x7e, 0x7d, 0x25,
0x1b, 0x77, 0xbc, 0xf7, 0xc7, 0x0f, 0x84, 0x95, 0x10, 0x92, 0x67, 0x15, 0x11, 0x5a, 0x5e,
0x41, 0x66, 0x0f, 0x38, 0x03, 0xb2, 0xf1, 0x5d, 0xf8, 0xab, 0xc0, 0x02, 0x76, 0x84, 0x28,
0xf4, 0x9d, 0x56, 0x46, 0x60, 0x20, 0xdb, 0x68, 0xa7, 0xbb, 0xee, 0xac, 0x15, 0x01, 0x2f,
0x20, 0x09, 0xdb, 0xc0, 0x16, 0xa1, 0x89, 0xf9, 0x94, 0x59, 0x00, 0xc1, 0x76, 0xbf, 0xc1,
0x4d, 0x5d, 0x2d, 0xa9, 0x85, 0x2c, 0xd6, 0xd3, 0x14, 0xcc, 0x02, 0xc3, 0xc2, 0xfa, 0x6b,
0xb7, 0xa6, 0xef, 0xdd, 0x12, 0x26, 0xa4, 0x63, 0xe3, 0x62, 0xbd, 0x56, 0x8a, 0x52, 0x2b,
0xb9, 0xdf, 0x09, 0xbc, 0x0e, 0x97, 0xa9, 0xb0, 0x82, 0x46, 0x08, 0xd5, 0x1a, 0x8e, 0x1b,
0xa7, 0x90, 0x98, 0xb9, 0xbb, 0x3c, 0x17, 0x9a, 0xf2, 0x82, 0xba, 0x64, 0x0a, 0x7f, 0xca,
0x5a, 0x8c, 0x7c, 0xd3, 0x79, 0x09, 0x5b, 0x26, 0xbb, 0xbd, 0x25, 0xdf, 0x3d, 0x6f, 0x9a,
0x8f, 0xee, 0x21, 0x66, 0xb0, 0x8d, 0x84, 0x4c, 0x91, 0x45, 0xd4, 0x77, 0x4f, 0xb3, 0x8c,
0xbc, 0xa8, 0x99, 0xaa, 0x19, 0x53, 0x7c, 0x02, 0x87, 0xbb, 0x0b, 0x7c, 0x1a, 0x2d, 0xdf,
0x48, 0x44, 0x06, 0xd6, 0x7d, 0x0c, 0x2d, 0x35, 0x76, 0xae, 0xc4, 0x5f, 0x71, 0x85, 0x97,
0xc4, 0x3d, 0xef, 0x52, 0xbe, 0x00, 0xe4, 0xcd, 0x49, 0xd1, 0xd1, 0x1c, 0x3c, 0xd0, 0x1c,
0x42, 0xaf, 0xd4, 0xbd, 0x58, 0x34, 0x07, 0x32, 0xee, 0xb9, 0xb5, 0xea, 0xff, 0xd7, 0x8c,
0x0d, 0x2e, 0x2f, 0xaf, 0x87, 0xbb, 0xe6, 0x52, 0x71, 0x22, 0xf5, 0x25, 0x17, 0xa1, 0x82,
0x04, 0xc2, 0x4a, 0xbd, 0x57, 0xc6, 0xab, 0xc8, 0x35, 0x0c, 0x3c, 0xd9, 0xc2, 0x43, 0xdb,
0x27, 0x92, 0xcf, 0xb8, 0x25, 0x60, 0xfa, 0x21, 0x3b, 0x04, 0x52, 0xc8, 0x96, 0xba, 0x74,
0xe3, 0x67, 0x3e, 0x8e, 0x8d, 0x61, 0x90, 0x92, 0x59, 0xb6, 0x1a, 0x1c, 0x5e, 0x21, 0xc1,
0x65, 0xe5, 0xa6, 0x34, 0x05, 0x6f, 0xc5, 0x60, 0xb1, 0x83, 0xc1, 0xd5, 0xd5, 0xed, 0xd9,
0xc7, 0x11, 0x7b, 0x49, 0x7a, 0xf9, 0xf9, 0x84, 0x47, 0x9b, 0xe2, 0xa5, 0x82, 0xe0, 0xc2,
0x88, 0xd0, 0xb2, 0x58, 0x88, 0x7f, 0x45, 0x09, 0x67, 0x74, 0x61, 0xbf, 0xe6, 0x40, 0xe2,
0x9d, 0xc2, 0x47, 0x05, 0x89, 0xed, 0xcb, 0xbb, 0xb7, 0x27, 0xe7, 0xdc, 0x7a, 0xfd, 0xbf,
0xa8, 0xd0, 0xaa, 0x10, 0x39, 0x3c, 0x20, 0xf0, 0xd3, 0x6e, 0xb1, 0x72, 0xf8, 0xe6, 0x0f,
0xef, 0x37, 0xe5, 0x09, 0x33, 0x5a, 0x83, 0x43, 0x80, 0x4f, 0x65, 0x2f, 0x7c, 0x8c, 0x6a,
0xa0, 0x82, 0x0c, 0xd4, 0xd4, 0xfa, 0x81, 0x60, 0x3d, 0xdf, 0x06, 0xf1, 0x5f, 0x08, 0x0d,
0x6d, 0x43, 0xf2, 0xe3, 0x11, 0x7d, 0x80, 0x32, 0xc5, 0xfb, 0xc5, 0xd9, 0x27, 0xec, 0xc6,
0x4e, 0x65, 0x27, 0x76, 0x87, 0xa6, 0xee, 0xee, 0xd7, 0x8b, 0xd1, 0xa0, 0x5c, 0xb0, 0x42,
0x13, 0x0e, 0x95, 0x4a, 0xf2, 0x06, 0xc6, 0x43, 0x33, 0xf4, 0xc7, 0xf8, 0xe7, 0x1f, 0xdd,
0xe4, 0x46, 0x4a, 0x70, 0x39, 0x6c, 0xd0, 0xed, 0xca, 0xbe, 0x60, 0x3b, 0xd1, 0x7b, 0x57,
0x48, 0xe5, 0x3a, 0x79, 0xc1, 0x69, 0x33, 0x53, 0x1b, 0x80, 0xb8, 0x91, 0x7d, 0xb4, 0xf6,
0x17, 0x1a, 0x1d, 0x5a, 0x32, 0xd6, 0xcc, 0x71, 0x29, 0x3f, 0x28, 0xbb, 0xf3, 0x5e, 0x71,
0xb8, 0x43, 0xaf, 0xf8, 0xb9, 0x64, 0xef, 0xc4, 0xa5, 0x6c, 0x08, 0x53, 0xc7, 0x00, 0x10,
0x39, 0x4f, 0xdd, 0xe4, 0xb6, 0x19, 0x27, 0xfb, 0xb8, 0xf5, 0x32, 0x73, 0xe5, 0xcb, 0x32
};

/*
====================
COM_BlockSequenceCRCByte

For proxy protecting
====================
*/
uint8_t	Com_BlockSequenceCRCByte(uint8_t* base, int32_t length, int32_t sequence)
{
	int32_t		n;
	uint8_t*	p;
	int32_t 	x;
	uint8_t		chkb[60 + 4];
	uint16_t	crc;


	if (sequence < 0)
		Sys_Error("sequence < 0, this shouldn't happen\n");

	p = chktbl + (sequence % (sizeof(chktbl) - 4));

	if (length > 60)
		length = 60;
	memcpy(chkb, base, length);

	chkb[length] = p[0];
	chkb[length + 1] = p[1];
	chkb[length + 2] = p[2];
	chkb[length + 3] = p[3];

	length += 4;

	crc = CRC_Block(chkb, length);

	for (x = 0, n = 0; n < length; n++)
		x += chkb[n];

	crc = (crc ^ x) & 0xff;

	return crc;
}

//========================================================

float frand()
{
	return (rand() & 32767) * (1.0 / 32767);
}

float crand()
{
	return (rand() & 32767) * (2.0 / 32767) - 1;
}

void Key_Init();
void Render2D_EndLoadingPlaque();

/*
=============
Com_Error_f

Just throw a fatal error to
test error shutdown procedures
=============
*/
void Com_Error_f()
{
	Com_Error(ERR_FATAL, "%s", Cmd_Argv(1));
}


/*
=================
Qcommon_Init
=================
*/
void Common_Init(int32_t argc, char** argv)
{
	char* s;

	if (setjmp(abortframe))
		Sys_Error("Error during initialization");

	z_chain.next = z_chain.prev = &z_chain;

	// prepare enough of the subsystems to handle
	// cvar and command buffer management
	COM_InitArgv(argc, argv);

	Swap_Init();
	Cbuf_Init();

	Cmd_Init();
	Cvar_Init();

	Key_Init();

	Gameinfo_Load();

	// we need to add the early commands twice, because
	// a basedir needs to be set before execing
	// config files, but we want other parms to override
	// the settings of the config files
	Cbuf_AddEarlyCommands(false);
	Cbuf_Execute();

	FS_InitFilesystem();

	Cbuf_AddText("exec default.cfg\n");

#ifdef PLAYTEST
	Cbuf_AddText("exec config_playtest.cfg\n");
#else
	Cbuf_AddText("exec config.cfg\n");
#endif

	Cbuf_AddEarlyCommands(true);
	Cbuf_Execute();

	//
	// init commands and vars
	//
	Cmd_AddCommand("memory_zonestats", Memory_ZoneStats_f);
	Cmd_AddCommand("error", Com_Error_f);

	profile_all = Cvar_Get("profile_all", "0", 0);
	log_memalloc = Cvar_Get("log_memalloc", "0", 0);
#ifdef DEBUG
	log_stats = Cvar_Get("log_stats", "1", 0);

	developer = Cvar_Get("developer", "1", 0);
	hunk_total = Cvar_Get("hunk_total", "0", 0); // not cvar_noset because i can't be bothered to write code to convert to string and i didn't add setvalueforce yet
	hunk_areas = Cvar_Get("hunk_areas", "0", 0); // not cvar_noset because i can't be bothered to write code to convert to string and i didn't add setvalueforce yet
#else
	log_stats = Cvar_Get("log_stats", "0", 0);
	developer = Cvar_Get("developer", "0", 0);
#endif
	// force developer mode on on debug builds

	timescale = Cvar_Get("timescale", "1", 0);
	fixedtime = Cvar_Get("fixedtime", "0", 0);
	logfile_active = Cvar_Get("logfile", "0", 0);
	showtrace = Cvar_Get("showtrace", "0", 0);
#ifdef DEDICATED_ONLY
	dedicated = Cvar_Get("dedicated", "1", CVAR_NOSET);
#else
	dedicated = Cvar_Get("dedicated", "0", CVAR_NOSET);
#endif
#ifdef WIN_DEBUG_CONSOLE
	debug_console = Cvar_Get("debug_console", "1", CVAR_NOSET);
#else
	debug_console = Cvar_Get("debug_console", "0", CVAR_NOSET);
#endif

	s = va("%d.%d.%d.%d %s %s %s %s", ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_REVISION, ENGINE_VERSION_BUILD, BUILD_PLATFORM, __DATE__, __TIME__, BUILD_CONFIG);
	engine_version = Cvar_Get("version", s, CVAR_SERVERINFO | CVAR_NOSET);

	if (dedicated->value)
		Cmd_AddCommand("quit", Com_Quit);

	Sys_Init();

	Net_Init();					// Open sockets
	Netchan_Init();				// Initialise networking channels

	Localisation_Init();		// Initialise localisaiton system
	CPUID_Init();				// Initialise CPUID

	if (!Netservices_Init())	// Initialise CURL/the game's network services
	{
		if (!ns_disabled->value
			&& !ns_nointernetcheck->value) // make sure we actually KNOW netservices are down and the user didn't jsut turn it off
		{
			Com_Printf("Not connected to the internet!\n");
		}
		else
		{
			Com_Printf("Net services disabled - not contacting zombono.com at all\n");
		}
	}

	// start the update check
	if (!ns_noupdatecheck->value
		&& netservices_connected)
	{
		Netservices_UpdaterGetUpdate();
	}

	SV_Init();						// Initialise server variables
	CL_Init();						// Initialise the actual game if it's not a dedicated server

	// add + commands from command line
	if (!Cbuf_AddLateCommands())
	{	// if the user didn't give any commands, run default action
		if (!dedicated->value)
		{
			//TEMP
#ifndef PLAYTEST
			Cbuf_AddText("d1\n");
#endif
		}
		else
		{
			Cbuf_AddText("dedicated_start\n");
		}

		Cbuf_Execute();
	}
	else
	{	// the user asked for something explicit
		// so drop the loading plaque

#ifndef DEDICATED_ONLY
		Render2D_EndLoadingPlaque();
#endif
	}
	Com_Printf("====== Zombono Initialized ======\n\n");

}

/*
=================
Qcommon_Frame
=================
*/
void Common_Frame(int32_t msec)
{
	char* s;
	int64_t 	time_before, time_between, time_after;

	if (setjmp(abortframe))
	{
		if (shutdown_game)
			SV_ShutdownGameProgs();
		shutdown_game = false;
		return;			// an ERR_DROP was thrown
	}

	if (log_stats->modified)
	{
		log_stats->modified = false;
		if (log_stats->value)
		{
			if (log_stats_file)
			{
				fclose(log_stats_file);
				log_stats_file = 0;
			}
			log_stats_file = fopen("stats.log", "w");
			if (log_stats_file)
				fprintf(log_stats_file, "entities,dlights,parts,frame time\n");
		}
		else
		{
			if (log_stats_file)
			{
				fclose(log_stats_file);
				log_stats_file = 0;
			}
		}
	}

	if (fixedtime->value)
		msec = fixedtime->value;
	else if (timescale->value)
	{
		msec *= timescale->value;
		if (msec < 1)
			msec = 1;
	}

	if (showtrace->value)
	{
		extern	int32_t c_traces, c_brush_traces;
		extern	int32_t c_pointcontents;

		Com_Printf("%4i traces  %4i points\n", c_traces, c_pointcontents);
		c_traces = 0;
		c_brush_traces = 0;
		c_pointcontents = 0;
	}

	do
	{
		s = Sys_ConsoleInput();
		if (s)
			Cbuf_AddText(va("%s\n", s));
	} while (s);
	Cbuf_Execute();

	// Poll for netservices transfers
	Netservices_Frame();

	if (profile_all->value)
		time_before = Sys_Nanoseconds();

	SV_Frame(msec);

	if (profile_all->value)
		time_between = Sys_Nanoseconds();

	CL_Frame(msec);

	if (profile_all->value)
		time_after = Sys_Nanoseconds();

	if (profile_all->value)
	{
		float all, sv, gm, cl, rf;

		all = (time_after - time_before) / 1000000.0f;
		sv = (time_between - time_before) / 1000000.0f;
		cl = (time_after - time_between) / 1000000.0f;
		gm = (time_after_game - time_before_game) / 1000000.0f;
		rf = (time_after_ref - time_before_ref) / 1000000.0f;
		sv -= gm;
		cl -= rf;
		Com_Printf("Server: %3f GameDLL: %3f Client: %3f Renderer: %3f Total: %3f\n",
			sv, gm, cl, rf, all);
	}
}

/*
=================
Qcommon_Shutdown
=================
*/
void Common_Shutdown()
{
	Localisation_Shutdown();
}
