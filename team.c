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

color4_t TEAM_GetColor(int team)
{
	if (!sv.active) return team_color_invalid; 
	if (team < 0 
		|| team >= TEAM_COUNT) return team_color_invalid;

	return team_colors[team];
}