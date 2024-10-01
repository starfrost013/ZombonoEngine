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

#include <common/common.h>
#include "winquake.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>
#include <conio.h>

//===============================================================================

int32_t hunk_count;

uint8_t* membase;
int32_t hunk_max_size;
int32_t hunk_current_size;
int32_t hunk_total_size = 0;
#define	VIRTUAL_ALLOC

void* Memory_HunkBegin(int32_t maxsize)
{
	// reserve a huge chunk of memory, but don't commit any yet
	hunk_current_size = 0;
	hunk_max_size = maxsize;
#ifdef VIRTUAL_ALLOC
	membase = VirtualAlloc(NULL, maxsize, MEM_RESERVE, PAGE_NOACCESS);
#else
	membase = malloc(maxsize);
	memset(membase, 0, maxsize);
#endif
	if (!membase)
		Sys_Error("VirtualAlloc reserve failed");
	return (void*)membase;
}

void* Memory_HunkAlloc(int32_t size)
{
	void* buf;

	// round to cacheline
	size = (size + 31) & ~31;

#ifdef VIRTUAL_ALLOC
	// commit pages as needed
	buf = VirtualAlloc(membase, hunk_current_size + size, MEM_COMMIT, PAGE_READWRITE);
	if (!buf)
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&buf, 0, NULL);
		Sys_Error("VirtualAlloc commit failed.\n%s", buf);
	}
#endif
	hunk_current_size += size;
	hunk_total_size += size;

	if (hunk_current_size > hunk_max_size)
		Sys_Error("Hunk_Alloc: overflow");

	return (void*)(membase + hunk_current_size - size);
}

int32_t Memory_HunkEnd()
{
	// free the remaining unused virtual memory

	hunk_count++;
	//Com_Printf ("hunkcount: %i\n", hunkcount);
	return hunk_current_size;
}

void Memory_HunkFree(void* base)
{
	if (base)
#ifdef VIRTUAL_ALLOC
		VirtualFree(base, 0, MEM_RELEASE);
#else
		free(base);
#endif
	hunk_total_size -= hunk_current_size;

	hunk_count--;
}

//===============================================================================


/*
================
Sys_Milliseconds
================
*/
int32_t curtime;
int64_t curtime_ns;

// Returns the number of milliseconds since the engine started.
int32_t Sys_Milliseconds()
{
	static int32_t 	base;
	static bool	initialized = false;

	if (!initialized)
	{	// let base retain 16 bits of effectively random data
		base = timeGetTime() & 0xffff0000;
		initialized = true;
	}

	curtime = timeGetTime() - base;

	return curtime;
}

// Returns the number of nanoseconds since the engine started.
int64_t Sys_Nanoseconds()
{
	static int64_t base;
	static struct timespec timespec;
	static bool initialized = true;

	if (!initialized)
	{
		if (!timespec_get(&timespec, TIME_UTC))
			Sys_Error("**** BUG **** Timespec_Get failed! This should never happen...");

		base = ((int64_t)timespec.tv_sec * (int64_t)1000000000) + timespec.tv_nsec;

		// determine the base
		initialized = true;
	}

	if (!timespec_get(&timespec, TIME_UTC))
		Sys_Error("**** BUG **** Timespec_Get failed! This should never happen...");

	int64_t time_now = ((int64_t)timespec.tv_sec * (int64_t)1000000000) + timespec.tv_nsec;

	curtime_ns = time_now - base;

	return curtime_ns;
}


