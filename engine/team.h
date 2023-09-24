#pragma once

// Zombono
// © 2023 starfrost
//
// team.h: Defines team utility functions.


// RGBA format team colors
color4_t team_colors[2];

color4_t team_color_invalid;

#define TEAM_COUNT 2

color4_t TEAM_GetColor(int team);		// Get team color
int TEAM_Rebalance(int team);	// Determine if we need to rebalance. Return a suggested team. 