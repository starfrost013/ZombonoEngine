#pragma once

#include "quakedef.h"

// Zombono
// © 2023 starfrost
//
// team.c: Implements team utility functions.

color4_t team_colors[2] =
{
	{ 255, 140, 0, 255, },			// 0 (Director)
	{ 0, 255, 238, 255 }			// 1 (Player)
};

color4_t team_color_invalid = { 127, 127, 127, 255 };	// invalid

// Gets the color for the team team.
color4_t TEAM_GetColor(int team)
{
	if (team < 0 
		|| team >= TEAM_COUNT) return team_color_invalid;

	return team_colors[team];
}

// Checks if a rebalance is needed when a player joins by seeing if the directors and players are equal.
// If it is, returns a suggested team for that player.
int TEAM_Rebalance(int team)
{
	//if (!sv.active) return team; // harmless

	// if rebalance is off
	if (!sv_zombie_no_rebalance.value) return team;

	qboolean needRebalance = false;

	int num_directors = 0;
	int num_players = 0;


	for (int client_num = 0; client_num < cl.maxclients; client_num++)
	{
		scoreboard_t client = cl.scores[client_num];

		if (!client.name[0]) continue; // not connected. ignore.

		int team = client.team;
	
		if (team == ZOMBONO_TEAM_DIRECTOR) num_directors++;
		if (team == ZOMBONO_TEAM_PLAYER) num_players++;
	}

	// only one player, don't bother
	if (num_directors + num_players == 1) return team; 

	// check based on player team.
	if (team == ZOMBONO_TEAM_PLAYER)
	{
		if (num_players > num_directors) return ZOMBONO_TEAM_DIRECTOR;
		return ZOMBONO_TEAM_PLAYER;
	}
	else if (ZOMBONO_TEAM_DIRECTOR)
	{
		if (num_directors > num_players) return ZOMBONO_TEAM_PLAYER;
		return ZOMBONO_TEAM_DIRECTOR;
	}

	return ZOMBONO_TEAM_DIRECTOR; // default
}