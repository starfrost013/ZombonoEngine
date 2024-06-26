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

#pragma once
// qcommon.h -- definitions common between client and server, but not game*.dll
#include <qcommon/formats/pak.h>
#include <qcommon/formats/bsp.h>
#include <qcommon/formats/md2.h>
#include <qcommon/formats/sp2.h>
#include <game/q_shared.h>
#include "netservices/netservices.h" // hmm
#include "pdjson.h"
#include "version.h"

// Due to awk(wardness) we can't put this in version.h
#define ENGINE_VERSION STR(ZOMBONO_VERSION_MAJOR) "." STR(ZOMBONO_VERSION_MINOR) "." STR(ZOMBONO_VERSION_REVISION) "." STR(ZOMBONO_VERSION_BUILD)

//BUILD_PLATFORM - Used for updates

#ifdef RELEASE
#define BUILD_CONFIG "Release"
#elif PLAYTEST
#define BUILD_CONFIG "Playtest"
#else
#define BUILD_CONFIG "Debug"
#endif

#ifdef WIN32
#if _M_X86
#define BUILD_PLATFORM "win32"
#elif _M_X64
#define BUILD_PLATFORM "win64"
#elif _M_ARM64 // TEMP
#define BUILD_PLATFORM "winarm64"
#endif
#elif defined __linux__
#if defined __i386__
#define BUILD_PLATFORM "linux32"
#elif defined __x86_64__
#define BUILD_PLATFORM "linux64"
#elif defined __arm__
#define BUILD_PLATFORM "linuxarm32"
#elif defined __aarch64__
#define BUILD_PLATFORM "linuxarm64"
#endif	// !WIN32 and !__LINUX__

#define BUILD_PLATFORM "Define the Platform Name!"
#endif

//============================================================================

typedef struct sizebuf_s
{
	bool		allowoverflow;	// if false, do a Com_Error
	bool		overflowed;		// set to true if the buffer size failed
	uint8_t* data;
	int32_t 	maxsize;
	int32_t 	cursize;
	int32_t 	readcount;
} sizebuf_t;

void SZ_Init(sizebuf_t* buf, uint8_t* data, int32_t length);
void SZ_Clear(sizebuf_t* buf);
void* SZ_GetSpace(sizebuf_t* buf, int32_t length);
void SZ_Write(sizebuf_t* buf, void* data, int32_t length);
void SZ_Print(sizebuf_t* buf, char* data);	// strcats onto the sizebuf

//============================================================================

struct usercmd_s;
struct entity_state_s;

void MSG_WriteChar(sizebuf_t* sb, int32_t c);
void MSG_WriteByte(sizebuf_t* sb, int32_t c);
void MSG_WriteShort(sizebuf_t* sb, int32_t c);
void MSG_WriteInt(sizebuf_t* sb, int32_t c);
void MSG_WriteFloat(sizebuf_t* sb, float f);
void MSG_WriteString(sizebuf_t* sb, char* s);
void MSG_WriteCoord(sizebuf_t* sb, float f);
void MSG_WritePos(sizebuf_t* sb, vec3_t pos);
void MSG_WriteAngle(sizebuf_t* sb, float f);
void MSG_WriteAngle16(sizebuf_t* sb, float f);
void MSG_WriteDeltaUsercmd(sizebuf_t* buf, usercmd_t* from, usercmd_t* cmd);
void MSG_WriteDeltaEntity(entity_state_t* from, entity_state_t* to, sizebuf_t* msg, bool force, bool newentity);
void MSG_WriteDir(sizebuf_t* sb, vec3_t vector);
void MSG_WriteColor(sizebuf_t* msg_read, color4_t color);

void MSG_BeginReading(sizebuf_t* sb);

int32_t MSG_ReadChar(sizebuf_t* sb);
int32_t MSG_ReadByte(sizebuf_t* sb);
int32_t MSG_ReadShort(sizebuf_t* sb);
int32_t MSG_ReadInt(sizebuf_t* sb);
float MSG_ReadFloat(sizebuf_t* sb);
char* MSG_ReadString(sizebuf_t* sb);
char* MSG_ReadStringLine(sizebuf_t* sb);

float MSG_ReadCoord(sizebuf_t* msg_read);
void MSG_ReadPos(sizebuf_t* msg_read, vec3_t pos);
float MSG_ReadAngle(sizebuf_t* msg_read);
float MSG_ReadAngle16(sizebuf_t* msg_read);
void MSG_ReadColor(sizebuf_t* msg_read, color4_t color);

void MSG_ReadDeltaUsercmd(sizebuf_t* msg_read, usercmd_t* from, usercmd_t* move);

void MSG_ReadDir(sizebuf_t* sb, vec3_t vector);

void MSG_ReadData(sizebuf_t* sb, void* buffer, int32_t size);

//============================================================================

extern bool big_endian;

int16_t	BigShort(int16_t l);
int16_t	LittleShort(int16_t l);
uint16_t BigShortUnsigned(int16_t l);
uint16_t LittleShortUnsigned(int16_t l);
int32_t BigInt(int32_t l);
int32_t LittleInt(int32_t l);
uint32_t BigIntUnsigned(int32_t l);
uint32_t LittleIntUnsigned(int32_t l);
float	BigFloat(float l);
float	LittleFloat(float l);

void	Swap_Init();
char* va(char* format, ...);

int32_t	COM_Argc();
char* COM_Argv(int32_t arg);	// range and null checked
void	COM_ClearArgv(int32_t arg);
int32_t COM_CheckParm(char* parm);
void	COM_AddParm(char* parm);

void	COM_InitArgv(int32_t argc, char** argv);

char* CopyString(char* in);

//============================================================================

void Info_Print(char* s);

/* crc.h */

void CRC_Init(uint16_t* crcvalue);
void CRC_ProcessByte(uint16_t* crcvalue, uint8_t data);
uint16_t CRC_Value(uint16_t crcvalue);
uint16_t CRC_Block(uint8_t* start, int32_t count);

/*
==============================================================

PROTOCOL

==============================================================
*/

// protocol.h -- communications protocols

// The game protocol version
// 34 - Vanilla Q2
// 35 - Zombono start (0.04).
// 36 - Zombono with larger map bounds
// 37 - Zombono 0.04 final
// Then incremented by one for each version.
#define	PROTOCOL_VERSION	44

//=========================================

#define	PORT_MASTER	27900
#define	PORT_CLIENT	27901
#define	PORT_SERVER	27910

//=========================================

#define	UPDATE_BACKUP	16	// copies of entity_state_t to keep buffered
							// must be power of two
#define	UPDATE_MASK		(UPDATE_BACKUP-1)


//==================
// the svc_strings[] array in cl_parse.c should mirror this
//==================

//
// server to client
//
typedef enum svc_ops_e
{
	svc_bad,

	// these ops are known to the game dll
	svc_muzzleflash,
	svc_muzzleflash2,
	svc_temp_entity,
	svc_layout,					// will be removed!!!
	svc_uidraw,					// <ui name>
	svc_uisettext,				// <ui control name> <text>
	svc_uisetimage,				// <ui control name> <image path in pics folder>
	svc_leaderboard,			// <leaderboard_t struct>
	svc_leaderboarddraw,		// nothing
	svc_drawtext,				// <font name> <x> <y> <text>
	svc_loadout_add,			// <loadout_entry_t struct>
	svc_loadout_remove,			// <index, or -1 for current>
	svc_loadout_setcurrent,		// <item id>
	svc_loadout_clear,			// none

	// the rest are private to the client and server
	svc_disconnect,
	svc_reconnect,
	svc_sound,					// <see code>
	svc_print,					// [byte] id [string] null terminated string
	svc_stufftext,				// [string] stuffed into client's console buffer, should be \n terminated
	svc_serverdata,				// [long] protocol ...
	svc_configstring,			// [short] [string]
	svc_spawnbaseline,
	svc_centerprint,			// [string] to put in center of the screen
	svc_download,				// [short] size [size bytes]
	svc_playerinfo,				// variable
	svc_packetentities,			// [...]
	svc_deltapacketentities,	// [...]
	svc_frame
} svc_ops;

//==============================================

//
// client to server
//
typedef enum clc_ops_e
{
	clc_bad,
	clc_nop,
	clc_move,				// [[usercmd_t]
	clc_userinfo,			// [[userinfo string]
	clc_stringcmd,			// [string] message
	clc_stringcmd_noconsole,			// [int] teamflag
} clc_ops;

//==============================================

// player_state_t communication

#define	PS_M_TYPE			(1<<0)
#define	PS_M_ORIGIN			(1<<1)
#define	PS_M_VELOCITY		(1<<2)
#define	PS_M_TIME			(1<<3)
#define	PS_M_FLAGS			(1<<4)
#define	PS_M_GRAVITY		(1<<5)
#define	PS_M_DELTA_ANGLES	(1<<6)

#define	PS_VIEWOFFSET		(1<<7)
#define	PS_VIEWANGLES		(1<<8)
#define	PS_KICKANGLES		(1<<9)
#define	PS_BLEND			(1<<10)
#define	PS_FOV				(1<<11)
#define	PS_WEAPONINDEX		(1<<12)
#define	PS_WEAPONFRAME		(1<<13)
#define	PS_RDFLAGS			(1<<14)

//==============================================

// user_cmd_t communication

// ms and light always sent, the others are optional
#define	CM_ANGLE1 	(1<<0)
#define	CM_ANGLE2 	(1<<1)
#define	CM_ANGLE3 	(1<<2)
#define	CM_FORWARD	(1<<3)
#define	CM_SIDE		(1<<4)
#define	CM_UP		(1<<5)
#define	CM_BUTTONS	(1<<6)
#define	CM_IMPULSE	(1<<7)

//==============================================

// a sound without an ent or pos will be a local only sound
#define	SND_VOLUME		(1<<0)		// a byte
#define	SND_ATTENUATION	(1<<1)		// a byte
#define	SND_POS			(1<<2)		// three coordinates
#define	SND_ENT			(1<<3)		// a int16_t 0-2: channel, 3-12: entity
#define	SND_OFFSET		(1<<4)		// a byte, msec offset from frame start

#define DEFAULT_SOUND_PACKET_VOLUME	1.0
#define DEFAULT_SOUND_PACKET_ATTENUATION 1.0

//==============================================

// entity_state_t communication

// try to pack the common update flags into the first byte
#define	U_ORIGIN1	(1<<0)
#define	U_ORIGIN2	(1<<1)
#define	U_ANGLE2	(1<<2)
#define	U_ANGLE3	(1<<3)
#define	U_FRAME8	(1<<4)		// frame is a byte
#define	U_EVENT		(1<<5)
#define	U_REMOVE	(1<<6)		// REMOVE this entity, don't add it
#define	U_MOREBITS1	(1<<7)		// read one additional byte

// second byte
#define	U_NUMBER16	(1<<8)		// NUMBER8 is implicit if not set
#define	U_ORIGIN3	(1<<9)
#define	U_ANGLE1	(1<<10)
#define	U_MODEL		(1<<11)
#define U_RENDERFX8	(1<<12)		// fullbright, etc
#define	U_EFFECTS8	(1<<14)		// autorotate, trails, etc
#define	U_MOREBITS2	(1<<15)		// read one additional byte

// third byte
#define	U_SKIN8		(1<<16)
#define	U_FRAME16	(1<<17)		// frame is a short
#define	U_RENDERFX16 (1<<18)	// 8 + 16 = 32
#define	U_EFFECTS16	(1<<19)		// 8 + 16 = 32
#define	U_MODEL2	(1<<20)		// weapons, flags, etc
#define	U_MODEL3	(1<<21)
#define	U_MODEL4	(1<<22)
#define	U_MOREBITS3	(1<<23)		// read one additional byte

// fourth byte
#define	U_OLDORIGIN	(1<<24)		// FIXME: get rid of this
#define	U_SKIN16	(1<<25)
#define	U_SOUND		(1<<26)
#define	U_SOLID		(1<<27)


/*
==============================================================

CMD

Command text buffering and command execution

==============================================================
*/

/*

Any number of commands can be added in a frame, from several different sources.
Most commands come from either keybindings or console line input, but remote
servers can also send across commands and entire text files can be execed.

The + command line options are also added to the command buffer.

The game starts with a Cbuf_AddText ("exec quake.rc\n"); Cbuf_Execute ();

*/

#define	EXEC_NOW	0		// don't return until completed
#define	EXEC_INSERT	1		// insert at current position, but don't run yet
#define	EXEC_APPEND	2		// add to end of the command buffer

void Cbuf_Init();
// allocates an initial text buffer that will grow as needed

void Cbuf_AddText(char* text);
// as new commands are generated from the console or keybindings,
// the text is added to the end of the command buffer.

void Cbuf_InsertText(char* text);
// when a command wants to issue other commands immediately, the text is
// inserted at the beginning of the buffer, before any remaining unexecuted
// commands.

void Cbuf_ExecuteText(int32_t exec_when, char* text);
// this can be used in place of either Cbuf_AddText or Cbuf_InsertText

void Cbuf_AddEarlyCommands(bool clear);
// adds all the +set commands from the command line

bool Cbuf_AddLateCommands();
// adds all the remaining + commands from the command line
// Returns true if any late commands were added, which
// will keep the demoloop from immediately starting

void Cbuf_Execute();
// Pulls off \n terminated lines of text from the command buffer and sends
// them through Cmd_ExecuteString.  Stops when the buffer is empty.
// Normally called once per frame, but may be explicitly invoked.
// Do not call inside a command function!

void Cbuf_CopyToDefer();
void Cbuf_InsertFromDefer();
// These two functions are used to defer any pending commands while a map
// is being loaded

//===========================================================================

/*

Command execution takes a null terminated string, breaks it into tokens,
then searches for a command or variable that matches the first token.

*/

typedef void (*xcommand_t) ();

void Cmd_Init();

void Cmd_AddCommand(char* cmd_name, xcommand_t function);
// called by the init functions of other parts of the program to
// register commands and functions to call for them.
// The cmd_name is referenced later, so it should not be in temp memory
// if function is NULL, the command will be forwarded to the server
// as a clc_stringcmd instead of executed locally
void Cmd_RemoveCommand(char* cmd_name);

bool Cmd_Exists(char* cmd_name);
// used by the cvar code to check for cvar / command name overlap

char* Cmd_CompleteCommand(char* partial);
// attempts to match a partial command for automatic command line completion
// returns NULL if nothing fits

int32_t 	Cmd_Argc();
char* Cmd_Argv(int32_t arg);
char* Cmd_Args();
// The functions that execute commands get their parameters with these
// functions. Cmd_Argv () will return an empty string, not a NULL
// if arg > argc, so string operations are always safe.

void Cmd_TokenizeString(char* text, bool macroExpand);
// Takes a null terminated string.  Does not need to be /n terminated.
// breaks the string up into arg tokens.

void Cmd_ExecuteString(char* text);
// Parses a single line of text into arguments and tries to execute it
// as if it was typed at the console

void Cmd_ForwardToServer();
// adds the current command line as a clc_stringcmd to the client message.
// things like godmode, noclip, etc, are commands directed to the server,
// so when they are typed in at the console, they will need to be forwarded.


/*
==============================================================

CVAR

==============================================================
*/

/*

cvar_t variables are used to hold scalar or string variables that can be changed or displayed at the console or prog code as well as accessed directly
in C code.

The user can access cvars from the console in three ways:
r_draworder			prints the current value
r_draworder 0		sets the current value to 0
set r_draworder 0	as above, but creates the cvar if not present
Cvars are restricted from having the same names as commands to keep this
interface from being ambiguous.
*/

extern	cvar_t* cvar_vars;

cvar_t* Cvar_Get(char* var_name, char* value, int32_t flags);
// creates the variable if it doesn't exist, or returns the existing one
// if it exists, the value will not be changed, but flags will be ORed in
// that allows variables to be unarchived without needing bitflags

cvar_t* Cvar_Set(char* var_name, char* value);
// will create the variable if it doesn't exist

cvar_t* Cvar_ForceSet(char* var_name, char* value);
// will set the variable even if NOSET or LATCH

cvar_t* Cvar_FullSet(char* var_name, char* value, int32_t flags);

void Cvar_SetValue(char* var_name, float value);
// expands value to a string and calls Cvar_Set

float Cvar_VariableValue(char* var_name);
// returns 0 if not defined or non numeric

char* Cvar_VariableString(char* var_name);
// returns an empty string if not defined

char* Cvar_CompleteVariable(char* partial);
// attempts to match a partial variable name for command line completion
// returns NULL if nothing fits

void Cvar_GetLatchedVars();
// any CVAR_LATCHED variables that have been set will now take effect

bool Cvar_Command();
// called by Cmd_ExecuteString when Cmd_Argv(0) doesn't match a known
// command.  Returns true if the command was a variable reference that
// was handled. (print or change)

void Cvar_WriteVariables(char* path);
// appends lines containing "set variable value" for all variables
// with the archive flag set to true.

void Cvar_Init();

char* Cvar_Userinfo();
// returns an info string containing all the CVAR_USERINFO cvars

char* Cvar_Serverinfo();
// returns an info string containing all the CVAR_SERVERINFO cvars

extern	bool	userinfo_modified;
// this is set each time a CVAR_USERINFO variable is changed
// so that the client knows to send it to the server

/*
==============================================================
Gameinfo stuff
==============================================================
*/

typedef struct gameinfo_s
{
	char name[MAX_QPATH];
	char asset_path[MAX_QPATH];
} gameinfo_t;

extern gameinfo_t gameinfo;

// copied from struct above, internal to gameinfo.c
extern cvar_t* game_name;
extern cvar_t* game_asset_path;

// Sys_Error on fail
void Gameinfo_Load();

/*
==============================================================
NET
==============================================================
*/

// net.h -- quake's interface to the networking layer

#define	PORT_ANY	-1

#define	MAX_MSGLEN		16384		// max length of a message (Quake 3)
#define	PACKET_HEADER	10			// two ints and a short

typedef enum { NA_LOOPBACK, NA_BROADCAST, NA_IP } netadrtype_t;

typedef enum { NS_CLIENT, NS_SERVER } netsrc_t;

// Defines aNetwork address
typedef struct netadr_s
{
	netadrtype_t	type;
	uint8_t			ip[4];
	uint16_t		port;
} netadr_t;

void NET_Init();
void NET_Shutdown();

void NET_Config(bool multiplayer);

bool NET_GetPacket(netsrc_t sock, netadr_t* net_from, sizebuf_t* net_message);
void NET_SendPacket(netsrc_t sock, int32_t length, void* data, netadr_t to);

bool NET_CompareAdr(netadr_t a, netadr_t b);
bool NET_CompareBaseAdr(netadr_t a, netadr_t b);
bool NET_IsLocalAddress(netadr_t adr);
char* NET_AdrToString(netadr_t a);
bool NET_StringToAdr(char* s, netadr_t* a);
void NET_Sleep(int32_t msec);

//============================================================================

#define	OLD_AVG		0.99		// total = oldtotal*OLD_AVG + new*(1-OLD_AVG)

#define	MAX_LATENT	32

typedef struct
{
	bool		fatal_error;
	netsrc_t	sock;

	int32_t 	last_received;		// for timeouts
	int32_t 	last_sent;			// for retransmits

	netadr_t	remote_address;
	int32_t 	qport;				// qport value to write when transmitting

	// sequencing variables
	int32_t 	incoming_sequence;
	int32_t 	incoming_acknowledged;
	int32_t 	incoming_reliable_acknowledged;	// single bit

	int32_t 	incoming_reliable_sequence;		// single bit, maintained local

	int32_t 	outgoing_sequence;
	int32_t 	reliable_sequence;			// single bit
	int32_t 	last_reliable_sequence;		// sequence number of last send

	// reliable staging and holding areas
	sizebuf_t	message;		// writing buffer to send to server
	uint8_t		message_buf[MAX_MSGLEN - 16];		// leave space for header

	// message is copied to this buffer when it is first transfered
	int32_t 	reliable_length;
	uint8_t		reliable_buf[MAX_MSGLEN - 16];	// unacked reliable message
} netchan_t;

extern	netadr_t	net_from;
extern	sizebuf_t	net_message;
extern	uint8_t		net_message_buffer[MAX_MSGLEN];

void Netchan_Init();
void Netchan_Setup(netsrc_t sock, netchan_t* chan, netadr_t adr, int32_t qport);

bool Netchan_NeedReliable(netchan_t* chan);
void Netchan_Transmit(netchan_t* chan, int32_t length, uint8_t* data);
void Netchan_OutOfBand(int32_t net_socket, netadr_t adr, int32_t length, uint8_t* data);
void Netchan_OutOfBandPrint(int32_t net_socket, netadr_t adr, char* format, ...);
bool Netchan_Process(netchan_t* chan, sizebuf_t* msg);

bool Netchan_CanReliable(netchan_t* chan);

/*
==============================================================

Map Loader

==============================================================
*/

extern char	map_name[MAX_QPATH];

cmodel_t*	Map_Load(char* name, bool clientload, uint32_t* checksum);
cmodel_t*	Map_LoadInlineModel(char* name);	// *1, *2, etc

int32_t 	Map_GetNumClusters();
int32_t 	Map_NumInlineModels();
char*		Map_GetEntityString();

// creates a clipping hull for an arbitrary box
int32_t 	Map_HeadnodeForBox(vec3_t mins, vec3_t maxs);

// returns an ORed contents mask
int32_t 	Map_PointContents(vec3_t p, int32_t headnode);
int32_t 	Map_TransformedPointContents(vec3_t p, int32_t headnode, vec3_t origin, vec3_t angles);

trace_t		Map_BoxTrace(vec3_t start, vec3_t end, vec3_t mins, vec3_t maxs, int32_t headnode, int32_t brushmask);
trace_t		Map_TransformedBoxTrace(vec3_t start, vec3_t end, vec3_t mins, vec3_t maxs, int32_t headnode, int32_t brushmask, vec3_t origin, vec3_t angles);

uint8_t*	Map_ClusterPVS(int32_t cluster);
uint8_t*	Map_ClusterPHS(int32_t cluster);

// call with topnode set to the headnode, returns with topnode
// set to the first node that splits the box
int32_t 	Map_BoxLeafnums(vec3_t mins, vec3_t maxs, int32_t* list, int32_t listsize, int32_t* topnode);

int32_t 	Map_GetLeafContents(int32_t leafnum);
int32_t 	Map_GetLeafCluster(int32_t leafnum);
int32_t 	Map_LeafArea(int32_t leafnum);

void		Map_SetAreaPortalState(int32_t portalnum, bool open);
bool		Map_AreasConnected(int32_t area1, int32_t area2);

int32_t 	Map_WriteAreaBits(uint8_t* buffer, int32_t area);
bool		Map_HeadnodeVisible(int32_t headnode, uint8_t* visbits);

void		Map_WritePortalState(FILE* f);
void		Map_ReadPortalState(FILE* f);
int32_t 	CM_PointLeafnum(vec3_t p);

/*
==============================================================

PLAYER MOVEMENT CODE

Common between server and client so prediction matches

==============================================================
*/

extern float pm_airaccelerate;

void Pmove(pmove_t* pmove);

/*
==============================================================

FILESYSTEM

==============================================================
*/

void	FS_InitFilesystem();
void	FS_SetGamedir(char* dir);
char*	FS_Gamedir();
char*	FS_NextPath(char* prevpath);
void	FS_ExecAutoexec();

int32_t FS_FOpenFile(char* filename, FILE** file);
void	FS_FCloseFile(FILE* f);
// note: this can't be called from another DLL, due to MS libc issues

int32_t FS_LoadFile(char* path, void** buffer);
// a null buffer will just return the file length without loading
// a -1 length is not present

void	FS_Read(void* buffer, int32_t len, FILE* f);
// properly handles partial reads

void	FS_FreeFile(void* buffer);

void	FS_CreatePath(char* path);


/*
==============================================================

MISC

==============================================================
*/


#define	ERR_FATAL	0		// exit the entire game with a popup window
#define	ERR_DROP	1		// print to console and disconnect from game
#define	ERR_QUIT	2		// not an error, just a normal exit

#define	EXEC_NOW	0		// don't return until completed
#define	EXEC_INSERT	1		// insert at current position, but don't run yet
#define	EXEC_APPEND	2		// add to end of the command buffer

#define	PRINT_ALL		0
#define PRINT_DEVELOPER	1	// only print when "developer 1"

void	Com_BeginRedirect(int32_t target, char* buffer, int32_t buffersize, void(*flush));
void	Com_EndRedirect();
void 	Com_Printf(char* fmt, ...);
void 	Com_DPrintf(char* fmt, ...);
void 	Com_Error(int32_t code, char* fmt, ...);
void 	Com_Quit();

int32_t Com_ServerState();		// this should have just been a cvar...
void	Com_SetServerState(int32_t state);

uint32_t Com_BlockChecksum(void* buffer, int32_t length);
uint8_t	Com_BlockSequenceCRCByte(uint8_t* base, int32_t length, int32_t sequence);

float frand();	// 0 ti 1
float crand();	// -1 to 1

extern cvar_t* developer;
extern cvar_t* dedicated;
extern cvar_t* host_speeds;
extern cvar_t* log_stats;
extern cvar_t* debug_console;

extern FILE* log_stats_file;

// host_speeds times
extern int32_t 	time_before_game;
extern int32_t 	time_after_game;
extern int32_t 	time_before_ref;
extern int32_t 	time_after_ref;

void Z_Free(void* ptr);
void* Z_Malloc(int32_t size);			// returns 0 filled memory
void* Z_TagMalloc(int32_t size, int32_t tag);
void Z_FreeTags(int32_t tag);

void Qcommon_Init(int32_t argc, char** argv);
void Qcommon_Frame(int32_t msec);
void Qcommon_Shutdown();

#define NUMVERTEXNORMALS	162

extern	vec3_t	bytedirs[NUMVERTEXNORMALS];

// this is in the client code, but can be used for debugging from server
void Render2D_DebugGraph(float value, int32_t r, int32_t g, int32_t b, int32_t a);

/*
==============================================================

NON-PORTABLE SYSTEM SERVICES

==============================================================
*/

void	Sys_Init();

void	Sys_UnloadGame();
void*	Sys_GetGameAPI(void* parms);
// loads the game dll and calls the api init function

char*	Sys_ConsoleInput();
void	Sys_ConsoleOutput(char* string);
void	Sys_Error(char* error, ...);
int32_t	Sys_Msgbox(char* title, uint32_t buttons, char* text, ...);
void	Sys_Quit();
char*	Sys_GetClipboardData(void);

/*
==============================================================

CLIENT / SERVER SYSTEMS

==============================================================
*/

void CL_Init();
void CL_Drop();
void CL_Shutdown();
void CL_Frame(int32_t msec);
void Con_Print(char* text);
void Render2D_BeginLoadingPlaque();

void SV_Init();
void SV_Shutdown(char* finalmsg, bool reconnect);
void SV_Frame(int32_t msec);