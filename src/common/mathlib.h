/*
Euphoria Game Engine
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
#include <common/common.h>

// mathlib.c: Implements mathlib stuff
// September 29, 2024 (moved from engine blob to EuphoriaCommon)

extern vec3_t vec3_origin;

#define DotProduct3(x,y)		(x[0]*y[0]+x[1]*y[1]+x[2]*y[2])
#define VectorSubtract3(a,b,c)	(c[0]=a[0]-b[0],c[1]=a[1]-b[1],c[2]=a[2]-b[2])
#define VectorAdd3(a,b,c)		(c[0]=a[0]+b[0],c[1]=a[1]+b[1],c[2]=a[2]+b[2])
#define VectorCopy3(a,b)		(b[0]=a[0],b[1]=a[1],b[2]=a[2])
#define VectorClear3(a)			(a[0]=a[1]=a[2]=0)
#define VectorNegate3(a,b)		(b[0]=-a[0],b[1]=-a[1],b[2]=-a[2])
#define VectorSet3(v, x, y, z)	(v[0]=(x), v[1]=(y), v[2]=(z))

// I searched i
#define DotProduct4(x,y)		(x[0]*y[0]+x[1]*y[1]+x[2]*y[2]+x[3]*y[3])
#define VectorSubtract4(a,b,c)	(c[0]=a[0]-b[0],c[1]=a[1]-b[1],c[2]=a[2]-b[2],c[3]=a[3]-b[3])
#define VectorAdd4(a,b,c)		(c[0]=a[0]+b[0],c[1]=a[1]+b[1],c[2]=a[2]+b[2],c[3]=a[3]+b[3])
#define VectorCopy4(a,b)		(b[0]=a[0],b[1]=a[1],b[2]=a[2],b[3]=a[3])
#define VectorClear4(a)			(a[0]=a[1]=a[2]=a[3]=0)
#define VectorNegate4(a,b)		(b[0]=-a[0],b[1]=-a[1],b[2]=-a[2],b[3]=-a[3])
#define VectorSet4(v, x, y, z, w)	(v[0]=(x), v[1]=(y), v[2]=(z), v[3]=(w))

// Functions for colours, that are better named
// Dot product isn't needed

#define ColorSubtract3(a,b,c)	VectorSubtract3(a,b,c)
#define ColorAdd3(a,b,c)	VectorAdd3(a,b,c)
#define ColorCopy3(a,b)	VectorCopy3(a,b)
#define ColorClear3(a)	VectorClear3(a)
#define ColorNegate3(a,b)	VectorNegate3(a,b))
#define ColorSet3(v, x, y, z)	VectorSet3(v, x, y, z)

#define ColorSubtract4(a,b,c)	VectorSubtract4(a,b,c)
#define ColorAdd4(a,b,c)	VectorAdd4(a,b,c)
#define ColorCopy4(a,b)	VectorCopy4(a,b)
#define ColorClear4(a)	VectorClear4(a)
#define ColorNegate4(a,b)	VectorNegate4(a,b))
#define ColorSet4(v, x, y, z, w)	VectorSet4(v, x, y, z, w)

void VectorCrossProduct(vec3_t v1, vec3_t v2, vec3_t cross);

void VectorMA3(vec3_t veca, float scale, vec3_t vecb, vec3_t vecc);
void VectorAddPointToBounds3(vec3_t v, vec3_t mins, vec3_t maxs);
bool VectorCompare3(vec3_t v1, vec3_t v2);
vec_t VectorLength3(vec3_t v);
vec_t VectorNormalize3(vec3_t v);		// returns vector length
void VectorInverse3(vec3_t v);
void VectorScale3(vec3_t in, vec_t scale, vec3_t out);

void VectorMA4(vec4_t veca, float scale, vec4_t vecb, vec4_t vecc);
void VectorAddPointToBounds4(vec4_t v, vec4_t mins, vec4_t maxs);
bool VectorCompare4(vec4_t v1, vec4_t v2);
vec_t VectorLength4(vec4_t v);
// Here is an article that precisely formulates and demonstrates the non-existence of a cross product in Euclidean spaces of dimensions other than 3 and 7:
// http://www.jstor.org/stable/2323537. 
vec_t VectorNormalize4(vec4_t v);		// returns vector length
void VectorInverse4(vec4_t v);
void VectorScale4(vec4_t in, vec_t scale, vec4_t out);

void R_ConcatRotations(float in1[3][3], float in2[3][3], float out[3][3]);

void AngleVectors(vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
int32_t BoxOnPlaneSide(vec3_t emins, vec3_t emaxs, struct cplane_s* plane);
float	anglemod(float a);
float LerpAngle(float a1, float a2, float frac);

#define BOX_ON_PLANE_SIDE(emins, emaxs, p)	\
	(((p)->type < 3)?						\
	(										\
		((p)->dist <= (emins)[(p)->type])?	\
			1								\
		:									\
		(									\
			((p)->dist >= (emaxs)[(p)->type])?\
				2							\
			:								\
				3							\
		)									\
	)										\
	:										\
		BoxOnPlaneSide( (emins), (emaxs), (p)))

void ProjectPointOnPlane(vec3_t dst, const vec3_t p, const vec3_t normal);
void PerpendicularVector(vec3_t dst, const vec3_t src);
void RotatePointAroundVector(vec3_t dst, const vec3_t dir, const vec3_t point, float degrees);
