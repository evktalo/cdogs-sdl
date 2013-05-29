/*
    C-Dogs SDL
    A port of the legendary (and fun) action/arcade cdogs.
    Copyright (C) 1995 Ronny Wester
    Copyright (C) 2003 Jeremy Chin
    Copyright (C) 2003-2007 Lucas Martin-King

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    This file incorporates work covered by the following copyright and
    permission notice:

    Copyright (c) 2013, Cong Xu
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/
#include "sounds.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <SDL.h>

#include "files.h"

SoundDevice gSoundDevice =
{
	0,
	64, 64,
	8,
	NULL,
	MUSIC_OK,
	"",
	{0, 0},
	{0, 0},
	{
		{"sounds/booom.wav",	0,	NULL},
		{"sounds/launch.wav",	0,	NULL},
		{"sounds/mg.wav",		0,	NULL},
		{"sounds/flamer.wav",	0,	NULL},
		{"sounds/shotgun.wav",	0,	NULL},
		{"sounds/fusion.wav",	0,	NULL},
		{"sounds/switch.wav",	0,	NULL},
		{"sounds/scream.wav",	0,	NULL},
		{"sounds/aargh1.wav",	0,	NULL},
		{"sounds/aargh2.wav",	0,	NULL},
		{"sounds/aargh3.wav",	0,	NULL},
		{"sounds/hahaha.wav",	0,	NULL},
		{"sounds/bang.wav",		0,	NULL},
		{"sounds/pickup.wav",	0,	NULL},
		{"sounds/click.wav",	0,	NULL},
		{"sounds/whistle.wav",	0,	NULL},
		{"sounds/powergun.wav",	0,	NULL},
		{"sounds/mg.wav",		0,	NULL}
	}
};


int OpenAudio(int frequency, Uint16 format, int channels, int chunkSize)
{
	int qFrequency;
	Uint16 qFormat;
	int qChannels;

	if (Mix_OpenAudio(frequency, format, channels, chunkSize) != 0)
	{
		printf("Couldn't open audio!: %s\n", SDL_GetError());
		return 1;
	}

	// Check that we got the specs we wanted
	Mix_QuerySpec(&qFrequency, &qFormat, &qChannels);
	debug(D_NORMAL, "spec: f=%d fmt=%d c=%d\n", qFrequency, qFormat, qChannels);
	if (qFrequency != frequency || qFormat != format || qChannels != channels)
	{
		printf("Audio not what we want.\n");
		return 1;
	}

	return 0;
}

void LoadSound(SoundData *sound)
{
	struct stat st;

	sound->isLoaded = 0;

	// Check that file exists
	if (stat(GetDataFilePath(sound->name), &st) == -1)
	{
		printf("Error finding sample '%s'\n", GetDataFilePath(sound->name));
		return;
	}

	// Load file data
	if ((sound->data = Mix_LoadWAV(GetDataFilePath(sound->name))) == NULL)
	{
		printf("Error loading sample '%s'\n", GetDataFilePath(sound->name));
		return;
	}

	sound->isLoaded = 1;
}

int SoundInitialize(void)
{
	int result = 1;
	int i;

	if (OpenAudio(22050, AUDIO_S16, 2, 512) != 0)
	{
		result = 0;
		goto bail;
	}

	if (Mix_AllocateChannels(gSoundDevice.channels) != gSoundDevice.channels)
	{
		printf("Couldn't allocate channels!\n");
		result = 0;
		goto bail;
	}

	for (i = 0; i < SND_COUNT; i++)
	{
		LoadSound(&gSoundDevice.sounds[i]);
	}

	gSoundDevice.isInitialised = 1;

bail:
	return result;
}

void SoundTerminate(void)
{
	if (!gSoundDevice.isInitialised)
	{
		return;
	}

	debug(D_NORMAL, "shutting down sound\n");
	MusicStop();
	Mix_CloseAudio();
}

void CalcLeftRightVolumeFromPanning(Uint8 *left, Uint8 *right, int panning)
{
	if (panning == 0)
	{
		*left = *right = 255;
	}
	else
	{
		if (panning < 0)
		{
			*left = (unsigned char)(255 + panning);
		}
		else
		{
			*left = (unsigned char)(panning);
		}

		*right = 255 - *left;
	}
}

void SoundPlay(sound_e sound, int panning, int volume)
{
	int c;
	Uint8 left, right;

	if (!gSoundDevice.isInitialised)
	{
		return;
	}

	debug(D_VERBOSE, "sound: %d panning: %d volume: %d\n", sound, panning, volume);

	CalcLeftRightVolumeFromPanning(&left, &right, panning);

	Mix_VolumeChunk(
		gSoundDevice.sounds[sound].data,
		(volume * gSoundDevice.volume) / 128);
	c = Mix_PlayChannel(-1, gSoundDevice.sounds[sound].data , 0);
	Mix_SetPanning(c, left, right);
}

void SoundSetVolume(int volume)
{
	debug(D_NORMAL, "volume: %d\n", volume);

	gSoundDevice.volume = volume;

	if (!gSoundDevice.isInitialised)
	{
		return;
	}
	Mix_Volume(-1, gSoundDevice.volume);
}

int SoundGetVolume(void)
{
	return gSoundDevice.volume;
}


void SoundSetLeftEar(int x, int y)
{
	gSoundDevice.earLeft.x = x;
	gSoundDevice.earLeft.y = y;
}

void SoundSetRightEar(int x, int y)
{
	gSoundDevice.earRight.x = x;
	gSoundDevice.earRight.y = y;
}

void SoundSetEars(int x, int y)
{
	SoundSetLeftEar(x, y);
	SoundSetRightEar(x, y);
}

#define RANGE_FULLVOLUME	70
#define RANGE_FACTOR		128
void SoundPlayAt(sound_e sound, int x, int y)
{
	int d = AXIS_DISTANCE(x, y, gSoundDevice.earLeft.x, gSoundDevice.earLeft.y);
	int volume, panning;

	if (gSoundDevice.earLeft.x != gSoundDevice.earRight.x ||
		gSoundDevice.earLeft.y != gSoundDevice.earRight.y)
	{
		int dLeft = d;
		int dRight = AXIS_DISTANCE(
			x, y, gSoundDevice.earRight.x, gSoundDevice.earRight.y);
		int leftVolume, rightVolume;

		d = (dLeft >
		     RANGE_FULLVOLUME ? dLeft - RANGE_FULLVOLUME : 0);
		leftVolume = 255 - (RANGE_FACTOR * d) / 256;
		if (leftVolume < 0)
			leftVolume = 0;

		d = (dRight >
		     RANGE_FULLVOLUME ? dRight - RANGE_FULLVOLUME : 0);
		rightVolume = 255 - (RANGE_FACTOR * d) / 256;
		if (rightVolume < 0)
			rightVolume = 0;

		volume = leftVolume + rightVolume;
		if (volume > 256)
			volume = 256;

		panning = rightVolume - leftVolume;
		panning /= 4;
	}
	else
	{
		d -= d / 4;
		d = (d > RANGE_FULLVOLUME ? d - RANGE_FULLVOLUME : 0);
		volume = 255 - (RANGE_FACTOR * d) / 256;
		if (volume < 0)
			volume = 0;

		panning = (x - gSoundDevice.earLeft.x) / 4;
	}

	if (volume > 0)
	{
		SoundPlay(sound, panning, volume);
	}
}

void SoundSetChannels(int channels)
{
	gSoundDevice.channels = CLAMP(channels, 2, 8);
}

int SoundGetChannels(void)
{
	return gSoundDevice.channels;
}