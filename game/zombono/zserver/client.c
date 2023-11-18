#include "defs.h"

/*  Copyright (C) 1996-1997  Id Software, Inc.
	Copyright (C) 2023 		 starfrost

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

	Client.qc : client code
*/

void W_WeaponFrame();
void W_SetCurrentAmmo();
void player_pain(entvars_t attacker, float damage);
void player_stand1();
void spawn_tfog(vec3_t org);
void spawn_tdeath(vec3_t org, entvars_t death_owner);
void CheckPowerups();
void PsuiStart();
void PsuiSetTeamDirector();
void PsuiSetTeamPlayer();
void PsuiSendTeamUpdate();

float modelindex_eyes, modelindex_player;

/*
=============================================================================

				LEVEL CHANGING / INTERMISSION

=============================================================================
*/

float intermission_running;
float intermission_exittime;

/*QUAKED info_intermission (1 0.5 0.5) (-16 -16 -16) (16 16 16)
This is the camera point for the intermission.
Use mangle instead of angle, so you can set pitch or roll as well as yaw.  'pitch roll yaw'
*/
void() info_intermission =
{
}

void() SetNewParms =
{
	//parm1 = IT_SHOTGUN | IT_AXE;
	parm2 = 100;
	parm3 = 0;
	parm4 = 25;
	parm5 = 0;
	parm6 = 0;
	parm7 = 0;
	//parm8 = 1;
	parm9 = 0;
}

void() SetChangeParms =
{
// remove items
	self.items -= self.items & (IT_INVISIBILITY | IT_INVULNERABILITY | IT_SUIT | IT_QUAD);

// cap super health
	if (self.health > 100)
		self.health = 100;
	if (self.health < 50)
		self.health = 50;
	//parm1 = self.items; 
	parm2 = self.health;
	parm3 = self.armorvalue;
	if (self.ammo_shells < 25)
		parm4 = 25;
	else
		parm4 = self.ammo_shells;
	parm5 = self.ammo_nails;
	parm6 = self.ammo_rockets;
	parm7 = self.ammo_cells;
	//parm8 = self.weapon;
	parm9 = self.armortype * 100;
}

void() DecodeLevelParms =
{
	// ZOMBONO: These parms are now a part of info_player_start_* for teamplay.
	// may need to gate these behind a cvar check
	//self.items = parm1;
	self.health = parm2;
	self.armorvalue = parm3;
	self.ammo_shells = parm4;
	self.ammo_nails = parm5;
	self.ammo_rockets = parm6;
	self.ammo_cells = parm7;
	//self.weapon = parm8;
	self.armortype = parm9 * 0.01;
}

/*
============
FindIntermission

Returns the entvars_t to view from
============
*/
entity() FindIntermission =
{
	entvars_t spot;
	float cyc;

// look for info_intermission first
	spot = find(world, classname, "info_intermission");
	if (spot)
	{ // pick a random one
		cyc = random() * 4;
		while (cyc > 1)
		{
			spot = find(spot, classname, "info_intermission");
			if (!spot)
				spot = find(spot, classname, "info_intermission");
			cyc = cyc - 1;
		}
		return spot;
	}

	if (gamemode == 0)
	{
		if (spot.team == ZOMBONO_TEAM_DIRECTOR) //temp
		{
			// then look for the start position
			spot = find(world, classname, "info_player_start_director");

		}
		else if (spot.team == ZOMBONO_TEAM_PLAYER)
		{
			spot = find(world, classname, "info_player_start_player");
		}

		if (spot)
			return spot;
	}

	objerror("FindIntermission: no spot");
	return world;
}


char* nextmap;

void() GotoNextMap =
{
	if (cvar("samelevel")) // if samelevel is set, stay on same level
		changelevel(mapname);
	else
		changelevel(nextmap);
}


void() ExitIntermission =
{
// skip any text in deathmatch
	if (deathmatch)
	{
		GotoNextMap();
		return;
	}

	intermission_exittime = time + 1;
	intermission_running = intermission_running + 1;

// no more episodes


	GotoNextMap();
}

/*
============
IntermissionThink

When the player presses attack or jump, change to the next level
============
*/
void() IntermissionThink =
{
	if (time < intermission_exittime)
		return;

	if (!self.button0 && !self.button1 && !self.button2)
		return;

	ExitIntermission();
}

void execute_changelevel()
{
	entvars_t pos;

	intermission_running = 1;

// enforce a wait time before allowing changelevel
	if (deathmatch)
		intermission_exittime = time + 5;
	else
		intermission_exittime = time + 2;

	WriteByte(MSG_ALL, SVC_CDTRACK);
	WriteByte(MSG_ALL, 3);
	WriteByte(MSG_ALL, 3);

	pos = FindIntermission();

	other = find(world, classname, "player");
	while (other != world)
	{
		other.view_ofs = '0 0 0';
		other.angles = other.v_angle = pos.mangle;
		other.fixangle = TRUE; // turn this way immediately
		other.nextthink = time + 0.5;
		other.takedamage = DAMAGE_NO;
		other.solid = SOLID_NOT;
		other.movetype = MOVETYPE_NONE;
		other.modelindex = 0;
		setorigin(other, pos.origin);
		other = find(other, classname, "player");
	}

	WriteByte(MSG_ALL, SVC_INTERMISSION);
}


void changelevel_touch()
{
	if (other.classname != "player")
		return;

	if (cvar("noexit"))
	{
		T_Damage(other, self, self, 50000);
		return;
	}
	bprint(other.netname);
	bprint(" exited the level\n");

	nextmap = self.map;

	SUB_UseTargets();

	if ((self.spawnflags & 1) && (deathmatch == 0))
	{ // NO_INTERMISSION
		GotoNextMap();
		return;
	}

	self.touch = SUB_Null;

// we can't move people right now, because touch functions are called
// in the middle of C movement code, so set a think time to do it
	self.think = execute_changelevel;
	self.nextthink = time + 0.1;
}

/*QUAKED trigger_changelevel (0.5 0.5 0.5) ? NO_INTERMISSION
When the player touches this, he gets sent to the map listed in the "map" variable.  Unless the NO_INTERMISSION flag is set, the view will go to the info_intermission spot and display stats.
*/
void trigger_changelevel()
{ 
	if (!self.map)
		objerror("changelevel trigger doesn't have map");

	InitTrigger();
	self.touch = changelevel_touch;
}


/*
=============================================================================

				PLAYER GAME EDGE FUNCTIONS

=============================================================================
*/

void set_suicide_frame()

// called by ClientKill and DeadThink
void respawn()
{
	// make a copy of the dead body for appearances sake
	CopyToBodyQue(self);
	// set default spawn parms
	SetNewParms();
	// respawn
	PutClientInServer();
}


/*
============
ClientKill

Player entered the suicide command
============
*/
void ClientKill()
{
	bprint(self.netname);
	bprint(" suicides\n");
	set_suicide_frame();
	self.modelindex = modelindex_player;
	self.frags = self.frags - 2; // extra penalty
	respawn();
}

float CheckSpawnPoint(vec3_t v)
{
	return FALSE;
}

/*
============
SelectSpawnPoint

Returns the entvars_t to spawn at
============
*/
entvars_t SelectSpawnPoint()
{
	entvars_t spot;
	float spawn_cycle;

	spawn_cycle = random() * 4;

	if (gamemode == 0)
	{
		if (world.level_has_prespawn
			&& !self.postspawn_done)
		{
			spot = find(world, classname, "info_player_start_prespawn");

			if (spot == world) spot = find(world, classname, "info_player_start_prespawn");

			// failsafe
			if (spot == world) spot = find(world, classname, "info_player_start_prespawn");
			return spot;
		}
		else
		{
			while (spawn_cycle > 1)
			{
				if (self.team == ZOMBONO_TEAM_DIRECTOR)
				{
					spot = find(world, classname, "info_player_start_director");
		
					if (spot == world)
					{
						spot = find(world, classname, "info_player_start_director");
					}
	
				}
				else if (self.team == ZOMBONO_TEAM_PLAYER)
				{
					spot = find(world, classname, "info_player_start_player");
		
					if (spot == world)
					{
						spot = find(world, classname, "info_player_start_player");
					}
				}

				spawn_cycle = spawn_cycle - 1;
			}

			// failsafe
			if (spot == world) spot = find(world, classname, "info_player_start_prespawn");
			return spot;
		}

	}
	else // no team
	{
		dprint("Legacy spawn behaviour activated -- invalid gamemode cvar!");
		dprint(ftos(gamemode));
		dprint("!\n");
	
		spot = find(world, classname, "info_player_start");
	}

	if (!spot)
		error("PutClientInServer: no info_player_start, info_player_start_prespawn, info_player_start_director or info_player_start_player in level");


	return spot;
}

/*
===========
PutClientInServer

called each time a player is spawned
============
*/
void DecodeLevelParms();
void PlayerDie();
void PrintTeamWelcomeMessage();

void PutClientInServer()
{
	entvars_t spot;

	self.classname = "player";
	self.health = 100;
	self.takedamage = DAMAGE_AIM;
	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_WALK;
	self.show_hostile = 0;
	self.max_health = 100;
	self.flags = FL_CLIENT;
	self.air_finished = time + 12;
	self.dmg = 2; // initial water damage
	self.super_damage_finished = 0;
	self.radsuit_finished = 0;
	self.invisible_finished = 0;
	self.invincible_finished = 0;
	self.double_jump_finished = 0;
	self.effects = 0;
	self.invincible_time = 0;
	self.double_jump_time = 0;

	DecodeLevelParms();

	self.attack_finished = time;
	self.th_pain = player_pain;
	self.th_die = PlayerDie;

	self.deadflag = DEAD_NO;
// paustime is set by teleporters to keep the player from moving a while
	self.pausetime = 0;

	spot = SelectSpawnPoint();

	// give weapons based on what info_player_start_* specified
	self.items = spot.startweapons;

	// temp code for playtest1 build only
	// give starter ammo
	if (self.items & IT_GRENADE_LAUNCHER == IT_GRENADE_LAUNCHER
		|| self.items & IT_ROCKET_LAUNCHER == IT_ROCKET_LAUNCHER) self.ammo_rockets = 5; //starter amount

	if (self.items & IT_SHOTGUN == IT_SHOTGUN
		|| self.items & IT_SUPER_SHOTGUN == IT_SUPER_SHOTGUN) self.ammo_shells = 24;

	if (self.items & IT_NAILGUN == IT_NAILGUN
		|| self.items & IT_SUPER_NAILGUN == IT_SUPER_NAILGUN) self.ammo_nails = 16;

	if (self.items & IT_LIGHTNING == IT_LIGHTNING) self.ammo_cells = 12;

	W_SetCurrentAmmo(); // moved here for new method of giving player weapons

	self.origin = spot.origin + '0 0 1';
	self.angles = spot.angles;
	self.fixangle = TRUE; // turn this way immediately

// oh, this is a hack!
	setmodel(self, "progs/eyes.mdl");
	modelindex_eyes = self.modelindex;

	setmodel(self, "progs/player.mdl");
	modelindex_player = self.modelindex;

	setsize(self, VEC_HULL_MIN, VEC_HULL_MAX);

	self.view_ofs = '0 0 22';

	player_stand1();

	makevectors(self.angles);
	spawn_tfog(self.origin + v_forward*20);

	spawn_tdeath(self.origin, self);
}

void PsuiStart()

// HACK HACK HACK
// This is called main because FTEQCC hardcodes global functions, and main wasn't being used.
// Either GameDLL system or fteqcc fork will happen
// this is actually ClientPostSpawn
void main()
{
	if (gamemode <= GAME_ZOMBONO_COUNT)
	{
		float timelimit = cvar("timelimit");
		
		PsuiStart();
		PrintTeamWelcomeMessage();
		SUB_UIStart("gameui");
		SUB_UIAddText("gameui", "gameui_timer", "Start", vid_width/2 - 32, vid_height/10);
		SUB_UIEnd();
		// set visibility later
		SUB_UISetVisibility("gameui", (timelimit > 0));
		SUB_UISetFocus("gameui", FALSE);

		self.postspawn_done = TRUE;
	}
}

void PsuiStart()
{
	// Draw the team selection UI
	SUB_UIStart("psui");
	SUB_UIAddButton("PsuiSetTeamDirector", "PsuiSetTeamDirector", "GFX/PSUI_SPAWN_DIRECTOR.LMP", 256, 256, vid_width/2 - 192, vid_height/2 - 128);
	SUB_UIAddButton("PsuiSetTeamPlayer", "PsuiSetTeamPlayer", "GFX/PSUI_SPAWN_PLAYER.LMP", 256, 256, vid_width/2 + 64, vid_height/2 - 128);
	SUB_UIAddText(string_null, "PsuiDirectorText", "Nobody knows who gave\nthem this power!", vid_width/2 - 128, vid_height/2 + 136);
	SUB_UIAddText(string_null, "PsuiPlayerText", "They don't know why they're here\nand they want to live!", vid_width/2 + 128, vid_height/2 + 136);
	//SUB_UIAddCheckbox(string_null, "PsuiTestCheckbox", "This is a checkbox (test...)!", TRUE, vid_width/2, vid_height/2 - 208);
	//SUB_UIAddSlider(string_null, "PsuiTestSlider", "I'm a slider!", 0, 10, 100, 100, vid_width/2, vid_height / 2 - 272);
	SUB_UIEnd();
	
	// Disabled for ZTest2
	SUB_UISetVisibility("psui", TRUE);
	SUB_UISetFocus("psui", TRUE);
}

void PsuiSetTeamDirector()
{
	dprint("PsuiSetTeamDirector called\n");
	if (gamemode <= GAME_ZOMBONO_COUNT
		&& self.classname == "player")
	{
		// should be current player/
		self.team = ZOMBONO_TEAM_DIRECTOR;
		SUB_UISetFocus("psui", FALSE);
		SUB_UISetVisibility("psui", FALSE);
		PsuiSendTeamUpdate();
	}
}

void PsuiSetTeamPlayer()
{
	dprint("PsuiSetTeamPlayer called\n");
	if (gamemode <= GAME_ZOMBONO_COUNT
		&& self.classname == "player")
	{
		// should be current player/
		self.team = ZOMBONO_TEAM_PLAYER;
		SUB_UISetFocus("psui", FALSE);
		SUB_UISetVisibility("psui", FALSE);
		PsuiSendTeamUpdate();
	}
}

void PsuiSendTeamUpdate()
{
	dprint("PsuiSendTeamUpdate called\n");

	self.movetype = MOVETYPE_TOSS; // Crashes otherwise even though it's a player and it gets overwritten anyway, WTF?
	self.model = string_null; // Prevent duplicate player models
	UpdateStats();
	respawn();
}

// todo: MAKE THIS RUN POST-SPAWN!!!
void PrintTeamWelcomeMessage()
{
	// terrible hack 
	// otherwise it doesn't work on join, only on respawn

	char* DIRECTOR_WELCOME_char* = "Welcome to Zombono.\nYou are on the Director team - press 2 to get the Zombinator.\nClick to spawn zombies and fuck those players!\n";
	char* PLAYER_WELCOME_char* = "Welcome to Zombono.\nYou are on the Player team\nSend those Alan Smithees back to film school!\n";

	if (self.team == ZOMBONO_TEAM_DIRECTOR)
	{
		sprint(self, DIRECTOR_WELCOME_char*);
	}
	else if (self.team == ZOMBONO_TEAM_PLAYER)
	{
		sprint(self, PLAYER_WELCOME_char*);
	}
}

/*
=============================================================================

				QUAKED FUNCTIONS

=============================================================================
*/


/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 24)
The normal starting point for a level.
*/
void info_player_start()
{
}

/* The starting point for levels with prespawn rooms. The players cannot fire weapons in these. */
void info_player_start_prespawn()
{
}

/*
Level starting point for Zombono director team.
*/
void info_player_start_director()
{
}

/*
Level starting point for Zombono player team.
*/
void info_player_start_player()
{
}


/*
===============================================================================

RULES

===============================================================================
*/

/*
go to the next level for deathmatch
*/
void NextLevel()
{
	entvars_t o;

// find a trigger changelevel
	o = find(world, classname, "trigger_changelevel");
	if (!o || mapname == "start")
	{ // go back to same map if no trigger_changelevel
		o = spawn();
		o.map = mapname;
	}

	nextmap = o.map;

	if (o.nextthink < time)
	{
		o.think = execute_changelevel;
		o.nextthink = time + 0.1;
	}
}

/*
============
CheckRules

Exit deathmatch games upon conditions
============
*/
void CheckRules()
{
	// Zombono Mode0 (TDM)
	/*
	if (gamemode == 0)
	{

	}
	*/

	// shared across all modes
	float timelimit;
	float fraglimit;

	if (gameover) // someone else quit the game already
		return;

	timelimit = cvar("timelimit") * 60;
	fraglimit = cvar("fraglimit");

	if (timelimit && time >= timelimit)
	{
		NextLevel();
		return;
	}

	if (fraglimit && self.frags >= fraglimit)
	{
		NextLevel();
		return;
	}
}

//============================================================================

void PlayerDeathThink()
{
	float forward;

	if ((self.flags & FL_ONGROUND))
	{
		forward = vlen(self.velocity);
		forward = forward - 20;
		if (forward <= 0)
			self.velocity = '0 0 0';
		else
			self.velocity = forward * normalize(self.velocity);
	}

// wait for all buttons released
	if (self.deadflag == DEAD_DEAD)
	{
		if (self.button2 || self.button1 || self.button0)
			return;
		self.deadflag = DEAD_RESPAWNABLE;
		return;
	}

// wait for any button down
	if (!self.button2 && !self.button1 && !self.button0)
		return;

	self.button0 = 0;
	self.button1 = 0;
	self.button2 = 0;
	respawn();
}


void PlayerJump()
{
	if (self.flags & FL_WATERJUMP)
		return;

	if (self.waterlevel >= 2)
	{
		if (self.watertype == CONTENT_WATER)
			self.velocity_z = 100;
		else if (self.watertype == CONTENT_SLIME)
			self.velocity_z = 80;
		else
			self.velocity_z = 50;

// play swiming sound
		if (self.swim_flag < time)
		{
			self.swim_flag = time + 1;
			if (random() < 0.5)
				sound(self, CHAN_BODY, "misc/water1.wav", 1, ATTN_NORM);
			else
				sound(self, CHAN_BODY, "misc/water2.wav", 1, ATTN_NORM);
		}

		return;
	}
	
	float can_jump = 1;

	// in the air? can't jump
	if (!(self.flags & FL_ONGROUND)) can_jump = 0;

	// but, if we have the double jump item we can!
	if (self.items & IT_DOUBLE_JUMP && self.jump_count < 2) can_jump = 1;

	// don't pogo stick
	if (!(self.flags & FL_JUMPRELEASE)) can_jump = 0;

	if (can_jump == 0)
		return;

	self.flags = self.flags - (self.flags & FL_JUMPRELEASE);

	self.flags = self.flags - FL_ONGROUND; // don't stairwalk

	self.button2 = 0;
// player jumping sound
	sound(self, CHAN_BODY, "player/plyrjmp8.wav", 1, ATTN_NORM);
	self.jump_count = self.jump_count + 1; 

	if (self.jump_count == 1)
	{
		self.velocity_z = self.velocity_z + jump_velocity; // increased for zombono
	}
	else
	{
		if (self.velocity_z > 0)
		{
			self.velocity_z = self.velocity_z + (jump_velocity - self.velocity_z);
		}
		else
		{
			self.velocity_z = self.velocity_z = jump_velocity;
		}
	}
}


/*
===========
WaterMove

============
*/
float dmgtime;

void WaterMove()
{
	if (self.movetype == MOVETYPE_NOCLIP)
		return;
	if (self.health < 0)
		return;

	if (self.waterlevel != 3)
	{
		if (self.air_finished < time)
			sound(self, CHAN_VOICE, "player/gasp2.wav", 1, ATTN_NORM);
		else if (self.air_finished < time + 9)
			sound(self, CHAN_VOICE, "player/gasp1.wav", 1, ATTN_NORM);
		self.air_finished = time + 12;
		self.dmg = 2;
	}
	else if (self.air_finished < time)
	{ // drown!
		if (self.pain_finished < time)
		{
			self.dmg = self.dmg + 2;
			if (self.dmg > 15)
				self.dmg = 10;
			T_Damage(self, world, world, self.dmg);
			self.pain_finished = time + 1;
		}
	}

	if (!self.waterlevel)
	{
		if (self.flags & FL_INWATER)
		{
			// play leave water sound
			sound(self, CHAN_BODY, "misc/outwater.wav", 1, ATTN_NORM);
			self.flags = self.flags - FL_INWATER;
		}
		return;
	}

	if (self.watertype == CONTENT_LAVA)
	{ // do damage
		if (self.dmgtime < time)
		{
			if (self.radsuit_finished > time)
				self.dmgtime = time + 1;
			else
				self.dmgtime = time + 0.2;

			T_Damage(self, world, world, 10*self.waterlevel);
		}
	}
	else if (self.watertype == CONTENT_SLIME)
	{ // do damage
		if (self.dmgtime < time && self.radsuit_finished < time)
		{
			self.dmgtime = time + 1;
			T_Damage(self, world, world, 4*self.waterlevel);
		}
	}

	if (!(self.flags & FL_INWATER))
	{

// player enter water sound

		if (self.watertype == CONTENT_LAVA)
			sound(self, CHAN_BODY, "player/inlava.wav", 1, ATTN_NORM);
		if (self.watertype == CONTENT_WATER)
			sound(self, CHAN_BODY, "player/inh2o.wav", 1, ATTN_NORM);
		if (self.watertype == CONTENT_SLIME)
			sound(self, CHAN_BODY, "player/slimbrn2.wav", 1, ATTN_NORM);

		self.flags = self.flags + FL_INWATER;
		self.dmgtime = 0;
	}

	if (!(self.flags & FL_WATERJUMP))
		self.velocity = self.velocity - 0.8*self.waterlevel*frametime*self.velocity;
}

void CheckWaterJump()
{
	vec3_t start, end;

// check for a jump-out-of-water
	makevectors(self.angles);
	start = self.origin;
	start_z = start_z + 8;
	v_forward_z = 0;
	normalize(v_forward);
	end = start + v_forward*24;
	traceline(start, end, TRUE, self);
	if (trace_fraction < 1)
	{ // solid at waist
		start_z = start_z + self.maxs_z - 8;
		end = start + v_forward*24;
		self.movedir = trace_plane_normal * -50;
		traceline(start, end, TRUE, self);
		if (trace_fraction == 1)
		{ // open at eye level
			self.flags = self.flags | FL_WATERJUMP;
			self.velocity_z = 225;
			self.flags = self.flags - (self.flags & FL_JUMPRELEASE);
			self.teleport_time = time + 2; // safety net
			return;
		}
	}
}


/*
================
PlayerPreThink

Called every frame before physics are run
================
*/
void PlayerPreThink =
{

	if (intermission_running)
	{
		IntermissionThink(); // otherwise a button could be missed between
		return; // the think tics
	}

	if (self.view_ofs == '0 0 0')
		return; // intermission or finale


	CheckRules();
	CheckPowerups();
	WaterMove();

	if (self.waterlevel == 2)
		CheckWaterJump();

	if (self.deadflag >= DEAD_DEAD)
	{
		PlayerDeathThink();
		return;
	}

	if (self.deadflag == DEAD_DYING)
		return; // dying, so do nothing

	if (self.button2)
	{
		PlayerJump();
	}
	else
		self.flags = self.flags | FL_JUMPRELEASE;

	if (self.button3)
	{

	}
	else
		self.flags = self.flags - FL_SPRINTING;
		
// teleporters can force a non-moving pause time
	if (time < self.pausetime)
		self.velocity = '0 0 0';

	// convert to seconds (todo: actual minute:second display)
	float timelimit = cvar("timelimit") * 60;

	if (self.postspawn_done
		&& timelimit > 0)
	{
		ui_last_entity = self; // set ui_last_entity to player

		float time_remaining = timelimit - time;
		if (time_remaining > 0)
		{
			SUB_UISetText("gameui", "gameui_timer", ftos(floor(time_remaining)));
		}
		else
		{
			SUB_UISetText("gameui", "gameui_timer", "It's Over");
		}
	}
}

/*
================
CheckPowerups

Check for turning off powerups
================
*/
void CheckPowerups =
{
	if (self.health <= 0)
		return;

// invisibility
	if (self.invisible_finished)
	{
// sound and screen flash when items starts to run out
		if (self.invisible_sound < time)
		{
			sound(self, CHAN_AUTO, "items/inv3.wav", 0.5, ATTN_IDLE);
			self.invisible_sound = time + ((random() * 3) + 1);
		}


		if (self.invisible_finished < time + 3)
		{
			if (self.invisible_time == 1)
			{
				sprint(self, "Ring of Shadows magic is fading\n");
				stuffcmd(self, "bf\n");
				sound(self, CHAN_AUTO, "items/inv2.wav", 1, ATTN_NORM);
				self.invisible_time = time + 1;
			}

			if (self.invisible_time < time)
			{
				self.invisible_time = time + 1;
				stuffcmd(self, "bf\n");
			}
		}

		if (self.invisible_finished < time)
		{ // just stopped
			self.items = self.items - IT_INVISIBILITY;
			self.invisible_finished = 0;
			self.invisible_time = 0;
		}

	// use the eyes
		self.frame = 0;
		self.modelindex = modelindex_eyes;
	}
	else
		self.modelindex = modelindex_player; // don't use eyes

// invincibility
	if (self.invincible_finished)
	{
// sound and screen flash when items starts to run out
		if (self.invincible_finished < time + 3)
		{
			if (self.invincible_time == 1)
			{
				sprint(self, "Protection is almost burned out\n");
				stuffcmd(self, "bf\n");
				sound(self, CHAN_AUTO, "items/protect2.wav", 1, ATTN_NORM);
				self.invincible_time = time + 1;
			}

			if (self.invincible_time < time)
			{
				self.invincible_time = time + 1;
				stuffcmd(self, "bf\n");
			}
		}

		if (self.invincible_finished < time)
		{ // just stopped
			self.items = self.items - IT_INVULNERABILITY;
			self.invincible_time = 0;
			self.invincible_finished = 0;
		}
		if (self.invincible_finished > time)
			self.effects = self.effects | EF_DIMLIGHT;
		else
			self.effects = self.effects - (self.effects & EF_DIMLIGHT);
	}

// super damage
	if (self.super_damage_finished)
	{

// sound and screen flash when items starts to run out

		if (self.super_damage_finished < time + 3)
		{
			if (self.super_time == 1)
			{
				sprint(self, "Quad Damage is wearing off\n");
				stuffcmd(self, "bf\n");
				sound(self, CHAN_AUTO, "items/damage2.wav", 1, ATTN_NORM);
				self.super_time = time + 1;
			}

			if (self.super_time < time)
			{
				self.super_time = time + 1;
				stuffcmd(self, "bf\n");
			}
		}

		if (self.super_damage_finished < time)
		{ // just stopped
			self.items = self.items - IT_QUAD;
			self.super_damage_finished = 0;
			self.super_time = 0;
		}
		if (self.super_damage_finished > time)
			self.effects = self.effects | EF_DIMLIGHT;
		else
			self.effects = self.effects - (self.effects & EF_DIMLIGHT);
	}

// suit
	if (self.radsuit_finished)
	{
		self.air_finished = time + 12; // don't drown

// sound and screen flash when items starts to run out
		if (self.radsuit_finished < time + 3)
		{
			if (self.rad_time == 1)
			{
				sprint(self, "Air supply in Biosuit expiring\n");
				stuffcmd(self, "bf\n");
				sound(self, CHAN_AUTO, "items/suit2.wav", 1, ATTN_NORM);
				self.rad_time = time + 1;
			}

			if (self.rad_time < time)
			{
				self.rad_time = time + 1;
				stuffcmd(self, "bf\n");
			}
		}

		if (self.radsuit_finished < time)
		{ // just stopped
			self.items = self.items - IT_SUIT;
			self.rad_time = 0;
			self.radsuit_finished = 0;
		}
	}

	// Double Jump time remaining
	if (self.double_jump_finished)
	{
		// flash the screen if less than 3s left.
		if (self.double_jump_finished < time + 3)
		{
			if (self.double_jump_time == 1)
			{
				sprint(self, "Double Jump about to end!\n");
				stuffcmd(self, "bf\n");
				self.double_jump_time = time + 1;
			}


			if (self.double_jump_time < time)
			{
				self.rad_time = time + 1;
				stuffcmd(self, "bf\n");
			}

		}
		if (self.double_jump_finished < time)
		{
			sprint(self, "Your double jump has ended.");
			self.items = self.items - IT_DOUBLE_JUMP;
			self.double_jump_time = 0;
			self.double_jump_finished = 0;
		}
	}


}


/*
================
PlayerPostThink

Called every frame after physics are run
================
*/
void PlayerPostThink =
{
	if (self.view_ofs == '0 0 0')
		return; // intermission or finale
	if (self.deadflag)
		return;

	// if ever on ground, set jump count to 0
	if ((self.flags & FL_ONGROUND))
		self.jump_count = 0; //doublejump
// do weapon stuff

	W_WeaponFrame();

// check to see if player landed and play landing sound
	if ((self.jump_flag < -300) && (self.flags & FL_ONGROUND) && (self.health > 0))
	{
		if (self.watertype == CONTENT_WATER)
			sound(self, CHAN_BODY, "player/h2ojump.wav", 1, ATTN_NORM);
		else if (self.jump_flag < -650)
		{
			T_Damage(self, world, world, 5);
			sound(self, CHAN_VOICE, "player/land2.wav", 1, ATTN_NORM);
			self.deathtype = "falling";
		}
		else
			sound(self, CHAN_VOICE, "player/land.wav", 1, ATTN_NORM);

		self.jump_flag = 0;
		self.jump_count = 0;
	}

	if (!(self.flags & FL_ONGROUND))
		self.jump_flag = self.velocity_z;
	
	//dprint("Velocity end: ");
	//dprint(vtos(self.velocity));
	//dprint("\n");
}


/*
===========
ClientConnect

called when a player connects to a server
============
*/
void ClientConnect =
{
	bprint(self.netname);
	bprint(" entered the game\n");

// a client connecting during an intermission can cause problems
	if (intermission_running)
		ExitIntermission();
}


/*
===========
ClientDisconnect

called when a player disconnects from a server
============
*/
void ClientDisconnect =
{
	if (gameover)
		return;
	// if the level end trigger has been activated, just return
	// since they aren't *really* leaving

	// let everyone else know
	bprint(self.netname);
	bprint(" left the game with ");
	bprint(ftos(self.frags));
	bprint(" frags\n");
	sound(self, CHAN_BODY, "player/tornoff2.wav", 1, ATTN_NONE);
	set_suicide_frame();
}

/*
===========
ClientObituary

called when a player dies
============
*/
void(entvars_t targ, entvars_t attacker) ClientObituary =
{
	float rnum;
	char* deathchar*, deathchar*2;
	rnum = random();

	if (targ.classname == "player")
	{
		if (attacker.classname == "teledeath")
		{
			bprint(targ.netname);
			bprint(" was telefragged by ");
			bprint(attacker.owner.netname);
			bprint("\n");

			attacker.owner.frags = attacker.owner.frags + 1;
			return;
		}

		if (attacker.classname == "teledeath2")
		{
			bprint("Satan's power deflects ");
			bprint(targ.netname);
			bprint("'s telefrag\n");

			targ.frags = targ.frags - 1;
			return;
		}

		if (attacker.classname == "player")
		{
			if (targ == attacker)
			{
				// killed self
				attacker.frags = attacker.frags - 1;
				bprint(targ.netname);

				if (targ.weapon == 64 && targ.waterlevel > 1)
				{
					bprint(" discharges into the water.\n");
					return;
				}
				if (targ.weapon == 16)
					bprint(" tries to put the pin back in\n");
				else if (rnum)
					bprint(" becomes bored with life\n");
				else
					bprint(" checks if his weapon is loaded\n");
				return;
			}
			else
			{
				attacker.frags = attacker.frags + 1;

				rnum = attacker.weapon;
				if (rnum == IT_AXE)
				{
					deathchar* = " was ax-murdered by ";
					deathchar*2 = "\n";
				}
				if (rnum == IT_SHOTGUN)
				{
					deathchar* = " chewed on ";
					deathchar*2 = "'s boomstick\n";
				}
				if (rnum == IT_SUPER_SHOTGUN)
				{
					deathchar* = " ate 2 loads of ";
					deathchar*2 = "'s buckshot\n";
				}
				if (rnum == IT_NAILGUN)
				{
					deathchar* = " was nailed by ";
					deathchar*2 = "\n";
				}
				if (rnum == IT_SUPER_NAILGUN)
				{
					deathchar* = " was punctured by ";
					deathchar*2 = "\n";
				}
				if (rnum == IT_GRENADE_LAUNCHER)
				{
					deathchar* = " eats ";
					deathchar*2 = "'s pineapple\n";
					if (targ.health < -40)
					{
						deathchar* = " was gibbed by ";
						deathchar*2 = "'s grenade\n";
					}
				}
				if (rnum == IT_ROCKET_LAUNCHER)
				{
					deathchar* = " rides ";
					deathchar*2 = "'s rocket\n";
					if (targ.health < -40)
					{
						deathchar* = " was gibbed by ";
						deathchar*2 = "'s rocket\n" ;
					}
				}
				if (rnum == IT_LIGHTNING)
				{
					deathchar* = " accepts ";
					if (attacker.waterlevel > 1)
						deathchar*2 = "'s discharge\n";
					else
						deathchar*2 = "'s shaft\n";
				}
				bprint(targ.netname);
				bprint(deathchar*);
				bprint(attacker.netname);
				bprint(deathchar*2);
			}
			return;
		}
		else
		{
			targ.frags = targ.frags - 1; // killed self
			bprint(targ.netname);

			if (attacker.flags & FL_MONSTER)
			{
				if (attacker.classname == "monster_army")
					bprint(" was shot by a Grunt\n");
				if (attacker.classname == "monster_demon1")
					bprint(" was eviscerated by a Fiend\n");
				if (attacker.classname == "monster_dog")
					bprint(" was mauled by a Rottweiler\n");
				if (attacker.classname == "monster_dragon")
					bprint(" was fried by a Dragon\n");
				if (attacker.classname == "monster_enforcer")
					bprint(" was blasted by an Enforcer\n");
				if (attacker.classname == "monster_fish")
					bprint(" was fed to the Rotfish\n");
				if (attacker.classname == "monster_hell_knight")
					bprint(" was slain by a Death Knight\n");
				if (attacker.classname == "monster_knight")
					bprint(" was slashed by a Knight\n");
				if (attacker.classname == "monster_ogre")
					bprint(" was destroyed by an Ogre\n");
				if (attacker.classname == "monster_oldone")
					bprint(" became one with Shub-Niggurath\n");
				if (attacker.classname == "monster_shalrath")
					bprint(" was exploded by a Vore\n");
				if (attacker.classname == "monster_shambler")
					bprint(" was smashed by a Shambler\n");
				if (attacker.classname == "monster_tarbaby")
					bprint(" was slimed by a Spawn\n");
				// if (attacker.classname == "monster_vomit")
				//	bprint(" was vomited on by a Vomitus\n");
				if (attacker.classname == "monster_wizard")
					bprint(" was scragged by a Scrag\n");
				if (attacker.classname == "monster_zombie")
					bprint(" joins the Zombies\n");

				return;
			}
			if (attacker.classname == "explo_box")
			{
				bprint(" blew up\n");
				return;
			}
			if (attacker.solid == SOLID_BSP && attacker != world)
			{
				bprint(" was squished\n");
				return;
			}
			if (targ.deathtype == "falling")
			{
				targ.deathtype = "";
				bprint(" fell to his death\n");
				return;
			}
			if (attacker.classname == "trap_shooter" || attacker.classname == "trap_spikeshooter")
			{
				bprint(" was spiked\n");
				return;
			}
			if (attacker.classname == "fireball")
			{
				bprint(" ate a lavaball\n");
				return;
			}
			if (attacker.classname == "trigger_changelevel")
			{
				bprint(" tried to leave\n");
				return;
			}

			rnum = targ.watertype;
			if (rnum == -3)
			{
				if (random() < 0.5)
					bprint(" sleeps with the fishes\n");
				else
					bprint(" sucks it down\n");
				return;
			}
			else if (rnum == -4)
			{
				if (random() < 0.5)
					bprint(" gulped a load of slime\n");
				else
					bprint(" can't exist on slime alone\n");
				return;
			}
			else if (rnum == -5)
			{
				if (targ.health < -15)
				{
					bprint(" burst into flames\n");
					return;
				}
				if (random() < 0.5)
					bprint(" turned into hot slag\n");
				else
					bprint(" visits the Volcano God\n");
				return;
			}

			bprint(" died\n");
		}
	}
}
