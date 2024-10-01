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

/*
swap.c: Endianness swap functions
*/

#pragma once
#include "common.h"

/*
============================================================================

					BYTE ORDER FUNCTIONS

============================================================================
*/

bool	big_endian;

// can't just use function pointers, or dll linkage can
// mess up when common is included in multiple places
int16_t(*_BigShort) (int16_t l);
int16_t(*_LittleShort) (int16_t l);
uint16_t(*_BigShortUnsigned) (int16_t l);
uint16_t(*_LittleShortUnsigned) (int16_t l);
int32_t(*_BigInt) (int32_t l);
int32_t(*_LittleInt) (int32_t l);
uint32_t(*_BigIntUnsigned) (int32_t l);
uint32_t(*_LittleIntUnsigned) (int32_t l);
float (*_BigFloat) (float l);
float (*_LittleFloat) (float l);

int16_t BigShort(int16_t l) { return _BigShort(l); }
int16_t LittleShort(int16_t l) { return _LittleShort(l); }
uint16_t BigShortUnsigned(int16_t l) { return _BigShortUnsigned(l); }
uint16_t LittleShortUnsigned(int16_t l) { return _LittleShortUnsigned(l); }
int32_t  BigInt(int32_t l) { return _BigInt(l); }
int32_t  LittleInt(int32_t l) { return _LittleInt(l); }
uint32_t BigIntUnsigned(int32_t l) { return _BigIntUnsigned(l); }
uint32_t LittleIntUnsigned(int32_t l) { return _LittleIntUnsigned(l); }
float BigFloat(float l) { return _BigFloat(l); }
float LittleFloat(float l) { return _LittleFloat(l); }

int16_t ShortSwap(int16_t l)
{
	uint8_t    b1, b2;

	b1 = l & 255;
	b2 = (l >> 8) & 255;

	return (b1 << 8) + b2;
}

int16_t ShortNoSwap(int16_t l)
{
	return l;
}

uint16_t ShortSwapUnsigned(int16_t l)
{
	uint8_t   b1, b2;

	b1 = l & 255;
	b2 = (l >> 8) & 255;

	return ((uint16_t)b1 << 8) + b2;
}

uint16_t ShortNoSwapUnsigned(int16_t l)
{
	return l;
}

int32_t IntSwap(int32_t l)
{
	uint8_t    b1, b2, b3, b4;

	b1 = l & 255;
	b2 = (l >> 8) & 255;
	b3 = (l >> 16) & 255;
	b4 = (l >> 24) & 255;

	return ((int32_t)b1 << 24) + ((int32_t)b2 << 16) + ((int32_t)b3 << 8) + b4;
}

uint32_t IntNoSwap(int32_t l)
{
	return l;
}

uint32_t IntSwapUnsigned(int32_t l)
{
	uint8_t    b1, b2, b3, b4;

	b1 = l & 255;
	b2 = (l >> 8) & 255;
	b3 = (l >> 16) & 255;
	b4 = (l >> 24) & 255;

	return ((uint32_t)b1 << 24) + ((uint32_t)b2 << 16) + ((uint32_t)b3 << 8) + b4;
}

uint32_t IntNoSwapUnsigned(int32_t l)
{
	return l;
}

float FloatSwap(float f)
{
	union
	{
		float	f;
		uint8_t	b[4];
	} dat1, dat2;


	dat1.f = f;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];
	return dat2.f;
}

float FloatNoSwap(float f)
{
	return f;
}

/*
================
Swap_Init
================
*/
void Swap_Init()
{
	uint8_t	swaptest[2] = { 1,0 };

	// set the byte swapping variables in a portable manner	
	if (*(int16_t*)swaptest == 1)
	{
		big_endian = false;
		_BigShort = ShortSwap;
		_LittleShort = ShortNoSwap;
		_BigShortUnsigned = ShortSwapUnsigned;
		_LittleShortUnsigned = ShortNoSwapUnsigned;
		_BigInt = IntSwap;
		_LittleInt = IntNoSwap;
		_BigIntUnsigned = IntSwapUnsigned;
		_LittleIntUnsigned = IntNoSwapUnsigned;
		_BigFloat = FloatSwap;
		_LittleFloat = FloatNoSwap;
	}
	else
	{
		big_endian = true;
		_BigShort = ShortNoSwap;
		_LittleShort = ShortSwap;
		_BigShortUnsigned = ShortNoSwapUnsigned;
		_LittleShortUnsigned = ShortSwapUnsigned;
		_BigInt = IntNoSwap;
		_LittleInt = IntSwap;
		_BigFloat = FloatNoSwap;
		_LittleFloat = FloatSwap;
	}

}