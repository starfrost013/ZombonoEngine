#include "quakedef.h"
#include "progdefs.h"

/*  Copyright (C) 1996-1997  Id Software, Inc.
	Copyright (C) 2023		 starfrost

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

    See file, 'COPYING', for details.
*/

/*
	| | | | | ZOMBONO | | | | | 
		© 2023 starfrost!!!
	| | | | | | | | | | | | | |

	Defs.qc: 
	
	Global defines.
		- Source for entvars_t C structure
		- Source for globalvars_t C structure
		- Other global defines used by the QC game
*/

//
// system globals
//
entvars_t self;
entvars_t other;
entvars_t world;
float time;
float frametime;

float force_retouch; // force all entities to touch triggers
					// next frame.  this is needed because
					// non-moving things don't normally scan
					// for triggers, and when a trigger is
					// created(like a teleport trigger), it
					// needs to catch everything.
					// decremented each frame, so set to 2
					// to guarantee everything is touched
char* mapname;

float deathmatch;
float coop;
float friendly_fire; // 0 - friendly fire, 1 - no friendly fire

float serverflags; // propagated from level to level, used to
				// keep track of completed episodes

float total_secrets;
float total_monsters;

float found_secrets; // number of secrets found
float killed_monsters; // number of monsters killed


// spawnparms are used to encode information about clients across server
// level changes
float parm1, parm2, parm3, parm4, parm5, parm6, parm7, parm8, parm9, parm10, parm11, parm12, parm13, parm14, parm15, parm16;

//
// global variables set by built in functions
//
vec3_t v_forward, v_up, v_right; // set bymakevectors()

// set by traceline / tracebox
float trace_allsolid;
float trace_startsolid;
float trace_fraction;
vec3_t trace_endpos;
vec3_t trace_plane_normal;
float trace_plane_dist;
entvars_t trace_ent;
float trace_inopen;
float trace_inwater;

entvars_t msg_entity; // destination of single entvars_t writes

//
// required prog functions
//
void ClientPostSpawn(); // only for testing

void StartFrame();

void PlayerPreThink();
void PlayerPostThink();

void ClientKill();
void ClientConnect();
void PutClientInServer(); // call after setting the parm1... parms
void ClientDisconnect();

void SetNewParms(); // called when a client first connects to
					// a server. sets parms so they can be
					// saved off for restarts

void SetChangeParms(); // call to set parms for self so they can
						// be saved for a level transition

float gamemode;	// Zombono

//================================================
void end_sys_globals; // flag for structure dumping
//================================================

/*
==============================================================================

			SOURCE FOR ENTVARS_T C STRUCTURE

==============================================================================
*/

//
// system fields(*** = do not set in prog code, maintained by C code)
//
float modelindex; // *** model index in the precached list
vec3_t absmin, absmax; // *** origin + mins / maxs

float ltime; // local time for entity
float movetype;
float solid;

vec3_t origin; // ***
vec3_t oldorigin; // ***
vec3_t velocity;
vec3_t angles;
vec3_t avelocity;

vec3_t punchangle; // temp angle adjust from damage or recoil

char* classname; // spawn function
char* model;
float frame;
float skin;
float effects;

vec3_t mins, maxs; // bounding box extents reletive to origin
vec3_t size; // maxs - mins

void touch;
void use;
void think;
void blocked; // for doors or plats, called when can't push other

float nextthink;
entvars_t groundentity;

// stats
float health;
float frags;
float weapon; // one of the IT_SHOTGUN, etc flags
char* weaponmodel;
float weaponframe;
float currentammo;
float ammo_shells, ammo_nails, ammo_rockets, ammo_cells;

float items; // bit flags

float takedamage;
entvars_t chain;
float deadflag;

vec3_t view_ofs; // add to origin to get eye point


float button0; // fire
float button1; // use
float button2; // jump
float button3; // sprint

float impulse; // weapon changes

float fixangle;
vec3_t v_angle; // view / targeting angle for players
float idealpitch; // calculated pitch angle for lookup up slopes

char* netname;

entvars_t enemy;

float flags;

float colormap;
float team;

float max_health; // players maximum health is stored here

float teleport_time; // don't back up

float armortype; // save this fraction of incoming damage
float armorvalue;

float waterlevel; // 0 = not in, 1 = feet, 2 = wast, 3 = eyes
float watertype; // a contents value

float ideal_yaw;
float yaw_speed;

entvars_t aiment;

entvars_t goalentity; // a movetarget or an enemy

float spawnflags;

char* target;
char* targetname;

// damage is accumulated through a frame. and sent as one single
// message, so the super shotgun doesn't generate huge messages
float dmg_take;
float dmg_save;
entvars_t dmg_inflictor;

entvars_t owner; // who launched a missile
vec3_t movedir; // mostly for doors, but also used for waterjump

char* message; // trigger messages

float sounds; // either a cd track number or sound number

char* noise, noise1, noise2, noise3; // contains names of wavs to play

float startweapons; // Weapons at startup

float jump_count; // The number of times you have jumped. Used for double jump item.

float level_has_prespawn; // Does thelevel have prespawn?
float postspawn_done; // Is postspawn done?
//================================================
void end_sys_fields; // flag for structure dumping
//================================================

/*
==============================================================================

				VARS NOT REFERENCED BY C CODE

==============================================================================
*/

// point content values

float CONTENT_EMPTY = -1;
float CONTENT_SOLID = -2;
float CONTENT_WATER = -3;
float CONTENT_SLIME = -4;
float CONTENT_LAVA = -5;
float CONTENT_SKY = -6;

float STATE_TOP = 0;
float STATE_BOTTOM = 1;
float STATE_UP = 2;
float STATE_DOWN = 3;

vec3_t VEC_ORIGIN = '0 0 0';
vec3_t VEC_HULL_MIN = '-16 -16 -24';
vec3_t VEC_HULL_MAX = '16 16 32';

vec3_t VEC_HULL2_MIN = '-32 -32 -24';
vec3_t VEC_HULL2_MAX = '32 32 64';

//================================================

//
// globals
//
float movedist;
float gameover; // set when a rule exits

char* string_null; // null char*, nothing should be held here

entvars_t newmis; // launch_spike sets this after spawning it

entvars_t activator; // the entvars_t that activated a trigger or brush

entvars_t damage_attacker; // set by T_Damage
float framecount;

float skill;

float sv_cheats; // Cheats enabled

// Screen size
float vid_width;
float vid_height;

// Last entvars_t that triggered a user interface element.
// Hack to ensure we only ever use players for UI.
entvars_t ui_last_entity;

//================================================

//
// world fields(FIXME: make globals)
//
char* wad;
char* map;
float worldtype; // 0 = medieval 1 = metal 2 = base

//================================================

char* killtarget;

//
// quakeed fields
//
float light_lev; // not used by game, but parsed by light util
float style;
float start_weapons_director; 
float start_weapons_player;

//
// monster ai
//
void th_stand();
void th_walk();
void th_run();
void th_missile();
void th_melee();
void th_pain(entvars_t attacker, float damage);
void th_die();

entvars_t oldenemy; // mad at this player before taking damage

float speed;

float lefty;

float search_time;
float attack_state;

float AS_STRAIGHT = 1;
float AS_SLIDING = 2;
float AS_MELEE = 3;
float AS_MISSILE = 4;

//
// player only fields
//
float walkframe;

float attack_finished;
float pain_finished;

float invincible_finished;
float invisible_finished;
float super_damage_finished;
float radsuit_finished;
float double_jump_finished;

float invincible_time, invincible_sound;
float invisible_time, invisible_sound;
float super_time, super_sound;
float rad_time;
float double_jump_time;
float fly_sound;

float axhitme;

float show_hostile; // set to time+0.2 whenever a client fires a
					// weapon or takes damage.  Used to alert
					// monsters that otherwise would let the player go
float jump_flag; // player jump flag
float swim_flag; // player swimming sound flag
float air_finished; // when time > air_finished, start drowning
float bubble_count; // keeps track of the number of bubbles
char* deathtype; // keeps track of how the player died

float jump_velocity = 300; // jump velocity

//
// object stuff
//
char* mdl;
vec3_t mangle; // angle at start

vec3_t oldorigin; // only used by secret door

float t_length, t_width;

//
// doors, etc
//
vec3_t dest, dest1, dest2;
float wait; // time from firing to restarting
float delay; // time from activation to firing
entvars_t trigger_field; // door's trigger entity
char* noise4;

//
// monsters
//
float pausetime;
entvars_t movetarget;

//
// doors
//
float aflag;
float dmg; // damage done by door when hit

//
// misc
//
float cnt; // misc flag

//
// subs
//
void think1;
vec3_t finaldest, finalangle;

//
// triggers
//
float count; // for counting triggers

//
// plats / doors / buttons
//
float lip;
float state;
vec3_t pos1, pos2; // top and bottom positions
float height;

//
// sounds
//
float waitmin, waitmax;
float distance;
float volume;

//===========================================================================

//
// builtin functions
//

void makevectors(vec3_t ang); // sets v_forward, etc globals
void setorigin(entvars_t e, vec3_t o);
void setmodel(entvars_t e, char* m); // set movetype and solid first
void setsize(entvars_t e, vec3_t min, vec3_t max);

void _break();
float random(); // returns 0 - 1
void sound(entvars_t e, float chan, char* samp, float vol, float atten);
vec_t normalize(vec3_t v);
void error(char* e);
void objerror(char* e);
float vlen(vec3_t v);
float vectoyaw(vec3_t v);
entvars_t spawn();
void remove(entvars_t e);

// sets trace_* globals
// nomonsters can be:
// An entvars_t will also be ignored for testing if forent == test,
// forent->owner == test, or test->owner == forent
// a forent of world is ignored
void traceline(vec3_t v1, vec3_t v2, float nomonsters, entvars_t forent);

entvars_t checkclient(); // returns a client to look for
entvars_t find(entvars_t start, char* fld, char* match);
char* precache_sound(char* s);
char* precache_model(char* s);
void stuffcmd(entvars_t client, char* s);
entvars_t findradius(vec3_t org, float rad);
void bprint(char* s);
void sprint(entvars_t client, char* s);
void dprint(char* s);
char* ftos(float f);
char* vtos(vec3_t v);
void coredump(); // prints all edicts
void traceon(); // turns statment trace on
void traceoff();
void eprint(entvars_t e); // prints an entire edict
float walkmove(float yaw, float dist); // returns TRUE or FALSE
// float(float yaw, float dist) droptofloor= #34; // The engine code doesn't indicate the params to be used
float droptofloor(); // TRUE if landed on floor
void lightstyle(float style, char* value);
float rint(float v); // round to nearest int
float floor(float v); // largest integer <= v
float ceil(float v); // smallest integer >= v
float checkbottom(entvars_t e); // true if self is on ground
float pointcontents(vec3_t v); // returns a CONTENT_*

float fabs(float f);
vec3_t aim(entvars_t e, float speed); // returns the shooting vec3_t
float cvar(char* s); // return cvar.value
void localcmd(char* s); // put char* into local que
entvars_t nextent(entvars_t e); // for looping through all ents
void particle(vec3_t o, vec3_t d, float color, float count) = #48;// start a particle effect
void ChangeYaw(); // turn towards self.ideal_yaw
						// at self.yaw_speed
vec3_t vectoangle(vec3_t v);

//
// direct client message generation
//
void WriteByte();
void WriteChar(float to, float f);
void WriteShort(float to, float f);
void WriteLong(float to, float f);
void WriteCoord(float to, float f);
void WriteAngle(float to, float f);
void WriteString(float to, char* s);
void WriteEntity(float to, entvars_t s);
void WriteFloat(float to, float f);

void movetogoal(float step);

char* precache_file(char* s); // no effect except for -copy
void makestatic(entvars_t e);
void changelevel(char* s);

void cvar_set(char* var, char* val); // sets cvar.value

void centerprint(entvars_t client, char* s); // sprint, but in middle

void ambientsound(vec3_t pos, char* samp, float vol, float atten);

void setspawnparms(entvars_t e); // set parm1... to the values at level start for coop respawn

void UpdateStats(); // Send update stat message to all clients
//============================================================================

//
// subs.qc
//
void SUB_CalcMove(vec3_t tdest, float tspeed, void func);
void SUB_CalcMoveEntd(entvars_t ent, vec3_t tdest, float tspeed, void func);
void SUB_CalcAngleMove(vec3_t destangle, float tspeed, void func);
void SUB_CalcMoveDone();
void SUB_CalcAngleMoveDone();
void SUB_Null();
void SUB_UseTargets();
void SUB_Remove();

// modernui.qc: Core
void SUB_UIStart(char* name);
void SUB_UIEnd();

// modernui.qc: Attributes
void SUB_UISetVisibility(char* name, float visible);
void SUB_UISetFocus(char* name, float focused);
void SUB_UISetText(char* name, char* element_name, char* text);

// modernui.qc: Elements
void SUB_UIAddButton(char* on_click, char* element_name, char* texture, float size_x, float size_y, float position_x, float position_y);
void SUB_UIAddCheckbox(char* on_click, char* element_name, char* text, float checked, float position_x, float position_y);
void SUB_UIAddSlider(char* on_click, char* element_name, char* text, float value_min, float value_max, float size_x, float size_y, float position_x, float position_y);
void SUB_UIAddText(char* on_click, char* element_name, char* text, float position_x, float position_y);

//
// combat.qc
//
void T_Damage(entvars_t targ, entvars_t inflictor, entvars_t attacker, float damage);

float T_Heal(entvars_t e, float healamount, float ignore); // health function

float CanDamage(entvars_t targ, entvars_t inflictor);

//
// triggers.qc
//
void teleport_perform();

//
// zombie.qc
//
void monster_zombie_dospawn();