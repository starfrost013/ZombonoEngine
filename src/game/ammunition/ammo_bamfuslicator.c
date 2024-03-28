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
#include "../game_local.h"
#include "../entities/entity_base.h"

// ammo_bamfuslicator.c: Code for the Director team's bamfuslicator weapon ammo (split from g_weapon.c)
// WHERE IS THE FUCKING AMMUNITION?

#define BAMFUSLICATOR_MIN_DISTANCE		56		// so you don't get stuck
#define BAMFUSLICATOR_MAX_DISTANCE		768

/* Bamfuslicator */
void Ammo_Bamfuslicator(edict_t* self, vec3_t start, vec3_t aimdir, zombie_type zombie_type)
{
	edict_t*	monster;
	trace_t		trace;
	vec3_t		trace_start = { 0 };
	vec3_t		trace_end = { 0 };
	vec3_t		distance_to_player = { 0 };
	vec3_t		distance_to_target = { 0 };
	vec_t		vec_length_player = { 0 }, vec_length_target = { 0 };

	trace_start[0] = start[0] + (aimdir[0] * BAMFUSLICATOR_MIN_DISTANCE);
	trace_start[1] = start[1] + (aimdir[1] * BAMFUSLICATOR_MIN_DISTANCE);
	trace_start[2] = start[2] + (aimdir[2] * BAMFUSLICATOR_MIN_DISTANCE);

	// set up positions 
	trace_end[0] = trace_start[0] + (aimdir[0] * BAMFUSLICATOR_MAX_DISTANCE);
	trace_end[1] = trace_start[1] + (aimdir[1] * BAMFUSLICATOR_MAX_DISTANCE);
	trace_end[2] = trace_start[2] + (aimdir[2] * BAMFUSLICATOR_MAX_DISTANCE);

	// raycast from where we fired the weapon
	// check if we hit somethoing
	trace = gi.trace(trace_start, NULL, NULL, trace_end, self, CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_SLIME | CONTENTS_LAVA | CONTENTS_WINDOW | CONTENTS_WATER); // zombies don't like water!

	// completed, too far away to spawn a zombie
	if (trace.fraction == 1.0f)
	{
		return;
	}

	// stupid hack to AVOID spawning zombies in the wall or floor
	trace.endpos[0] -= aimdir[0] * BAMFUSLICATOR_MIN_DISTANCE;
	trace.endpos[1] -= aimdir[1] * BAMFUSLICATOR_MIN_DISTANCE;
	trace.endpos[2] += 32;

	// don't get the player spawning the zombie, or any other players stuck
	VectorSubtract(trace.endpos, start, distance_to_player);
	vec_length_target = 32768.0f; //dummy value

	if (trace.ent != NULL)
	{
		VectorSubtract(trace.endpos, trace.ent->s.origin, distance_to_target);
		vec_length_target = abs(VectorLength(distance_to_target));
	}

	vec_length_player = abs(VectorLength(distance_to_player));

	// push out if we are close to target or the spawning player
	if (vec_length_player <= BAMFUSLICATOR_MIN_DISTANCE
		|| vec_length_target <= BAMFUSLICATOR_MIN_DISTANCE)
	{
		// epsilon pulled out the ass to deal with the case where you are right below the zombie
		// force moving by BAMFUSLICATOR_MIN_DISTANCE could let people spawn zombies through close enough walls (<=56 units)
		float stupid_epsilon = 0.625f; // value hand tuned by a retard

		if (aimdir[0] > 0 && aimdir[0] < stupid_epsilon) aimdir[0] = stupid_epsilon;
		if (aimdir[0] > -stupid_epsilon && aimdir[0] < 0) aimdir[0] = -stupid_epsilon;
		if (aimdir[1] > 0 && aimdir[1] < stupid_epsilon) aimdir[1] = stupid_epsilon;
		if (aimdir[1] > -stupid_epsilon && aimdir[1] < 0) aimdir[1] = -stupid_epsilon;

		// don't shove vertical
		trace.endpos[0] += aimdir[0] * BAMFUSLICATOR_MIN_DISTANCE;
		trace.endpos[1] += aimdir[1] * BAMFUSLICATOR_MIN_DISTANCE;
	}

	if (trace.ent != NULL)
	{
		if (!strncmp(trace.ent->classname, "player", 6))
		{
			// can't spawn if inside a player
			return;
		}

	}

	// spawn some nice particles
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_TELEPORT);
	gi.WritePosition(trace.endpos);

	gi.sound(self, CHAN_VOICE, gi.soundindex("zombie/zombie_spawn.wav"), 1, ATTN_NORM, 0);
	//gi.WriteDir(zombie->s.angles);
	// 
	// spawn the zombie
	monster = G_Spawn();

	switch (zombie_type)
	{
	case zombie_type_normal:
		SP_monster_zombie(monster);
		break;
	case zombie_type_fast:
		SP_monster_zombie_fast(monster);
		break;
	case zombie_type_ogre:
		SP_monster_ogre(monster);
		break;
	}

	// move the zombie to where the player spawned it
	// the zombie is on director team (this is used so they don't harm directors unless the requisite gameflag is set)

	monster->team = team_director;
	VectorCopy(trace.endpos, monster->s.origin);
	VectorCopy(aimdir, monster->s.angles);

}
