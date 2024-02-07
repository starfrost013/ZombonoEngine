#pragma once
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

#include "g_local.h"

// m_zombie_shared.h
// Defines shared code for all zombie types (NOT ogres or anything else)
// 
// ALL ZOMBIES COVERED BY THIS CODE MUST HAVE THE SAME ANIMATIONS!!
// REPLACE CODE IF REPLACING ANIMATIONS!!! (See MD2 format docs)
// 
// February 7, 2024