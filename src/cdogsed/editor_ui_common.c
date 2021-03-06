/*
    C-Dogs SDL
    A port of the legendary (and fun) action/arcade cdogs.
    Copyright (c) 2014, Cong Xu
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
#include "editor_ui_common.h"

#include <cdogs/events.h>
#include <cdogs/font.h>
#include <cdogs/gamedata.h>
#include <cdogs/palette.h>


void DisplayMapItem(Vec2i pos, MapObject *mo)
{
	Vec2i offset;
	Pic *pic = MapObjectGetPic(mo, &gPicManager, &offset);
	Blit(&gGraphicsDevice, pic, Vec2iAdd(pos, offset));
}

void DrawKey(UIObject *o, GraphicsDevice *g, Vec2i pos, void *vData)
{
	UNUSED(g);
	EditorBrushAndCampaign *data = vData;
	PicPaletted *keyPic = PicManagerGetOldPic(
		&gPicManager,
		cGeneralPics[gMission.keyPics[data->Brush.ItemIndex]].picIndex);
	pos = Vec2iAdd(Vec2iAdd(pos, o->Pos), Vec2iScaleDiv(o->Size, 2));
	DrawTPic(pos.x, pos.y, keyPic);
}

void InsertMission(CampaignOptions *co, Mission *mission, int idx)
{
	if (mission == NULL)
	{
		Mission defaultMission;
		MissionInit(&defaultMission);
		defaultMission.Size = Vec2iNew(48, 48);
		mission = &defaultMission;
	}
	CArrayInsert(&co->Setting.Missions, idx, mission);
}
void DeleteMission(CampaignOptions *co)
{
	CASSERT(
		co->MissionIndex < (int)co->Setting.Missions.size,
		"invalid mission index");
	MissionTerminate(CampaignGetCurrentMission(co));
	CArrayDelete(&co->Setting.Missions, co->MissionIndex);
	if (co->MissionIndex >= (int)co->Setting.Missions.size)
	{
		co->MissionIndex = MAX(0, (int)co->Setting.Missions.size - 1);
	}
}

bool ConfirmScreen(const char *info, const char *msg)
{
	int w = gGraphicsDevice.cachedConfig.Res.x;
	int h = gGraphicsDevice.cachedConfig.Res.y;
	ClearScreen(&gGraphicsDevice);
	FontStr(info, Vec2iNew((w - FontStrW(info)) / 2, (h - FontH()) / 2));
	FontStr(msg, Vec2iNew((w - FontStrW(msg)) / 2, (h + FontH()) / 2));
	BlitFlip(&gGraphicsDevice, &gConfig.Graphics);

	int c = GetKey(&gEventHandlers);
	return (c == 'Y' || c == 'y');
}

void ClearScreen(GraphicsDevice *g)
{
	color_t color = { 32, 32, 60, 255 };
	Uint32 pixel = PixelFromColor(&gGraphicsDevice, color);
	for (int i = 0; i < GraphicsGetScreenSize(&g->cachedConfig); i++)
	{
		g->buf[i] = pixel;
	}
}
