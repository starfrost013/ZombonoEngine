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
// Quake is a trademark of Id Software, Inc., (c) 1996 Id Software, Inc. All
// rights reserved.

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#if defined(__linux__)
#include <linux/cdrom.h>
#else
#include <sys/cdio.h>
#include <sys/disklabel.h>
#define CDROMREADTOCHDR CDIOREADTOCHEADER
#define CDROMREADTOCENTRY CDIOREADTOCENTRYS
#define CDROMPLAYTRKIND CDIOCPLAYTRACKS
#define CDROMCLOSETRAY CDIOCCLOSE
#define CDROMEJECT CDIOCEJECT
#define CDROMRESUME CDIOCRESUME
#define CDROMSTOP CDIOCSTOP
#define CDROMPAUSE CDIOCPAUSE
#define CDROMSUBCHNL CDIOCREADSUBCHANNEL
#endif

#include "../client/client.h"

static bool cdValid = false;
static bool	playing = false;
static bool	wasPlaying = false;
static bool	initialized = false;
static bool	enabled = true;
static bool playLooping = false;
static float	cdvolume;
static uint8_t 	remap[100];
static uint8_t		playTrack;
static uint8_t		maxTrack;

static int cdfile = -1;

//static char cd_dev[64] = "/dev/cdrom";

cvar_t	*cd_volume;
cvar_t *cd_dev;

void CDAudio_Pause(void);

static void CDAudio_Eject(void)
{
	if (cdfile == -1 || !enabled)
		return; // no cd init'd

	if ( ioctl(cdfile, CDROMEJECT) == -1 ) 
		Com_DPrintf("ioctl cdromeject failed\n");
}


static void CDAudio_CloseDoor(void)
{
	if (cdfile == -1 || !enabled)
		return; // no cd init'd

	if ( ioctl(cdfile, CDROMCLOSETRAY) == -1 ) 
		Com_DPrintf("ioctl cdromclosetray failed\n");
}

static int CDAudio_GetAudioDiskInfo(void)
{
#if defined(__linux__)
	struct cdrom_tochdr tochdr;
#else
	struct ioc_toc_header tochdr;
#endif

	cdValid = false;

	if ( ioctl(cdfile, CDROMREADTOCHDR, &tochdr) == -1 ) 
    {
      Com_DPrintf("ioctl cdromreadtochdr failed\n");
	  return -1;
    }

#if defined(__linux__)
	if (tochdr.cdth_trk0 < 1)
	{
		Com_DPrintf("CDAudio: no music tracks\n");
		return -1;
	}

	cdValid = true;
	maxTrack = tochdr.cdth_trk1;
#else
	if (tochdr.starting_track < 1)
	{
		Com_DPrintf("CDAudio: no music tracks\n");
		return -1;
	}

	cdValid = true;
	maxTrack = tochdr.ending_track;
#endif

	return 0;
}


bool CDAudio_Play(int track, bool looping)
{
#if defined(__linux__)
	struct cdrom_tocentry entry;
	struct cdrom_ti ti;
#else
	struct ioc_read_toc_entry entry;
	struct ioc_play_track ti;
#endif

	if (cdfile == -1 || !enabled)
		return false;
	
	if (!cdValid)
	{
		CDAudio_GetAudioDiskInfo();
		if (!cdValid)
			return false;
	}

	track = remap[track];

	if (track < 1 || track > maxTrack)
	{
		Com_DPrintf("CDAudio: Bad track number %u.\n", track);
		return false;
	}

	// don't try to play a non-audio track
#if defined(__linux__)
	entry.cdte_track = track;
	entry.cdte_format = CDROM_MSF;
#else
	entry.starting_track = track;
	entry.address_format = CD_MSF_FORMAT;
#endif
	if ( ioctl(cdfile, CDROMREADTOCENTRY, &entry) == -1 )
	{
		Com_DPrintf("ioctl cdromreadtocentry failed\n");
		return false;
	}
#if defined(__linux__)
	if (entry.cdte_ctrl == CDROM_DATA_TRACK)
	{
		Com_Printf("CDAudio: track %i is not audio\n", track);
		return false;
	}
#endif

	if (playing)
	{
		if (playTrack == track)
			return true;
		CDAudio_Stop();
	}

#if defined(__linux__)
	ti.cdti_trk0 = track;
	ti.cdti_trk1 = track;
	ti.cdti_ind0 = 1;
	ti.cdti_ind1 = 99;
#else
	ti.start_track = track;
	ti.end_track = track;
	ti.start_index = 1;
	ti.end_index = 99;
#endif

	if ( ioctl(cdfile, CDROMPLAYTRKIND, &ti) == -1 ) 
	{
		Com_DPrintf("ioctl cdromplaytrkind failed\n");
		return false;
	}

	if ( ioctl(cdfile, CDROMRESUME) == -1 ) 
		Com_DPrintf("ioctl cdromresume failed\n");

	playLooping = looping;
	playTrack = track;
	playing = true;

	if (cd_volume->value == 0.0)
		CDAudio_Pause ();

	return true;
}


void CDAudio_Stop(void)
{
	if (cdfile == -1 || !enabled)
		return;
	
	if (!playing)
		return;

	if ( ioctl(cdfile, CDROMSTOP) == -1 )
		Com_DPrintf("ioctl cdromstop failed (%d)\n", errno);

	wasPlaying = false;
	playing = false;
}

void CDAudio_Pause(void)
{
	if (cdfile == -1 || !enabled)
		return;

	if (!playing)
		return;

	if ( ioctl(cdfile, CDROMPAUSE) == -1 ) 
		Com_DPrintf("ioctl cdrompause failed\n");

	wasPlaying = playing;
	playing = false;
}


void CDAudio_Resume(void)
{
	if (cdfile == -1 || !enabled)
		return;
	
	if (!cdValid)
		return;

	if (!wasPlaying)
		return;
	
	if ( ioctl(cdfile, CDROMRESUME) == -1 ) 
		Com_DPrintf("ioctl cdromresume failed\n");
	playing = true;
}

static void CD_f (void)
{
	char	*command;
	int		ret;
	int		n;

	if (Cmd_Argc() < 2)
		return;

	command = Cmd_Argv (1);

	if (Q_strcasecmp(command, "on") == 0)
	{
		enabled = true;
		return;
	}

	if (Q_strcasecmp(command, "off") == 0)
	{
		if (playing)
			CDAudio_Stop();
		enabled = false;
		return;
	}

	if (Q_strcasecmp(command, "reset") == 0)
	{
		enabled = true;
		if (playing)
			CDAudio_Stop();
		for (n = 0; n < 100; n++)
			remap[n] = n;
		CDAudio_GetAudioDiskInfo();
		return;
	}

	if (Q_strcasecmp(command, "remap") == 0)
	{
		ret = Cmd_Argc() - 2;
		if (ret <= 0)
		{
			for (n = 1; n < 100; n++)
				if (remap[n] != n)
					Com_Printf("  %u -> %u\n", n, remap[n]);
			return;
		}
		for (n = 1; n <= ret; n++)
			remap[n] = atoi(Cmd_Argv (n+1));
		return;
	}

	if (Q_strcasecmp(command, "close") == 0)
	{
		CDAudio_CloseDoor();
		return;
	}

	if (!cdValid)
	{
		CDAudio_GetAudioDiskInfo();
		if (!cdValid)
		{
			Com_Printf("No CD in player.\n");
			return;
		}
	}

	if (Q_strcasecmp(command, "play") == 0)
	{
		CDAudio_Play((uint8_t)atoi(Cmd_Argv (2)), false);
		return;
	}

	if (Q_strcasecmp(command, "loop") == 0)
	{
		CDAudio_Play((uint8_t)atoi(Cmd_Argv (2)), true);
		return;
	}

	if (Q_strcasecmp(command, "stop") == 0)
	{
		CDAudio_Stop();
		return;
	}

	if (Q_strcasecmp(command, "pause") == 0)
	{
		CDAudio_Pause();
		return;
	}

	if (Q_strcasecmp(command, "resume") == 0)
	{
		CDAudio_Resume();
		return;
	}

	if (Q_strcasecmp(command, "eject") == 0)
	{
		if (playing)
			CDAudio_Stop();
		CDAudio_Eject();
		cdValid = false;
		return;
	}

	if (Q_strcasecmp(command, "info") == 0)
	{
		Com_Printf("%u tracks\n", maxTrack);
		if (playing)
			Com_Printf("Currently %s track %u\n", playLooping ? "looping" : "playing", playTrack);
		else if (wasPlaying)
			Com_Printf("Paused %s track %u\n", playLooping ? "looping" : "playing", playTrack);
		Com_Printf("Volume is %f\n", cdvolume);
		return;
	}
}

void CDAudio_Update(void)
{
#if defined(__linux__)
	struct cdrom_subchnl subchnl;
#else
	struct ioc_read_subchannel subchnl;
#endif
	static time_t lastchk;

	if (cdfile == -1 || !enabled)
		return;

	if (cd_volume && cd_volume->value != cdvolume)
	{
		if (cdvolume)
		{
			Cvar_SetValue ("cd_volume", 0.0);
			cdvolume = cd_volume->value;
			CDAudio_Pause ();
		}
		else
		{
			Cvar_SetValue ("cd_volume", 1.0);
			cdvolume = cd_volume->value;
			CDAudio_Resume ();
		}
	}

	if (playing && lastchk < time(NULL)) {
		lastchk = time(NULL) + 2; //two seconds between chks
#if defined(__linux__)
		subchnl.cdsc_format = CDROM_MSF;
#else
		subchnl.data_format = CD_MSF_FORMAT;
#endif
		if (ioctl(cdfile, CDROMSUBCHNL, &subchnl) == -1 ) {
			Com_DPrintf("ioctl cdromsubchnl failed\n");
			playing = false;
			return;
		}
#if defined(__linux__)
		if (subchnl.cdsc_audiostatus != CDROM_AUDIO_PLAY &&
			subchnl.cdsc_audiostatus != CDROM_AUDIO_PAUSED) {
#else
		if (subchnl.track != playTrack) {
#endif
			playing = false;
			if (playLooping)
				CDAudio_Play(playTrack, true);
		}
	}
}

int CDAudio_Init(void)
{
	int i;
	cvar_t			*cv;
	extern uid_t saved_euid;

	cv = Cvar_Get ("nocdaudio", "0", CVAR_NOSET);
	if (cv->value)
		return -1;

	cd_volume = Cvar_Get ("cd_volume", "1", CVAR_ARCHIVE);
	if ( cd_volume->value == 0 )
		return -1;

	cd_dev = Cvar_Get("cd_dev", "/dev/cdrom", CVAR_ARCHIVE);

	seteuid(saved_euid);

	cdfile = open(cd_dev->string, O_RDONLY);

	seteuid(getuid());

	if (cdfile == -1) {
		Com_Printf("CDAudio_Init: open of \"%s\" failed (%i)\n", cd_dev->string, errno);
		cdfile = -1;
		return -1;
	}

	for (i = 0; i < 100; i++)
		remap[i] = i;
	initialized = true;
	enabled = true;

	if (CDAudio_GetAudioDiskInfo())
	{
		Com_Printf("CDAudio_Init: No CD in player.\n");
		cdValid = false;
	}

	Cmd_AddCommand ("cd", CD_f);

	Com_Printf("CD Audio Initialized\n");

	return 0;
}

void CDAudio_Activate (bool active)
{
	if (active)
		CDAudio_Resume ();
	else
		CDAudio_Pause ();
}

void CDAudio_Shutdown(void)
{
	if (!initialized)
		return;
	CDAudio_Stop();
	close(cdfile);
	cdfile = -1;
}
