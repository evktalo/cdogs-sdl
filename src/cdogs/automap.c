/*
    C-Dogs SDL
    A port of the legendary (and fun) action/arcade cdogs.
    Copyright (C) 1995 Ronny Wester
    Copyright (C) 2003 Jeremy Chin 
    Copyright (C) 2003 Lucas Martin-King 

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

    Copyright (c) 2013-2014, Cong Xu
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
#include "automap.h"

#include <stdio.h>
#include <string.h>

#include "actors.h"
#include "config.h"
#include "drawtools.h"
#include "font.h"
#include "map.h"
#include "mission.h"
#include "objs.h"
#include "pic_manager.h"


#define MAP_FACTOR 2
#define MASK_ALPHA 128;

color_t colorWall = { 72, 152, 72, 255 };
color_t colorFloor = { 12, 92, 12, 255 };
color_t colorRoom = { 24, 112, 24, 255 };
color_t colorDoor = { 172, 172, 172, 255 };
color_t colorYellowDoor = { 252, 224, 0, 255 };
color_t colorGreenDoor = { 0, 252, 0, 255 };
color_t colorBlueDoor = { 0, 252, 252, 255 };
color_t colorRedDoor = { 132, 0, 0, 255 };
color_t colorExit = { 255, 255, 255, 255 };



static void DisplayPlayer(TActor *player, Vec2i pos, int scale)
{
	Vec2i playerPos = Vec2iToTile(
		Vec2iNew(player->tileItem.x, player->tileItem.y));
	pos = Vec2iAdd(pos, Vec2iScale(playerPos, scale));
	if (scale >= 2)
	{
		Character *c = player->character;
		int picIdx = cHeadPic[c->looks.face][DIRECTION_DOWN][STATE_IDLE];
		PicPaletted *pic = PicManagerGetOldPic(&gPicManager, picIdx);
		pos.x -= pic->w / 2;
		pos.y -= pic->h / 2;
		DrawTTPic(pos.x, pos.y, pic, c->table);
	}
	else
	{
		Draw_Point(pos.x, pos.y, colorWhite);
	}
}

static void DisplayObjective(
	TTileItem *t, int objectiveIndex, Vec2i pos, int scale, int flags)
{
	Vec2i objectivePos = Vec2iNew(t->x / TILE_WIDTH, t->y / TILE_HEIGHT);
	struct Objective *o = CArrayGet(&gMission.Objectives, objectiveIndex);
	color_t color = o->color;
	pos = Vec2iAdd(pos, Vec2iScale(objectivePos, scale));
	if (flags & AUTOMAP_FLAGS_MASK)
	{
		color.a = MASK_ALPHA;
	}
	if (scale >= 2)
	{
		DrawCross(&gGraphicsDevice, pos.x, pos.y, color);
	}
	else
	{
		Draw_Point(pos.x, pos.y, color);
	}
}

static void DisplayExit(Vec2i pos, int scale, int flags)
{
	Vec2i exitPos = gMap.ExitStart;
	Vec2i exitSize = Vec2iAdd(Vec2iMinus(gMap.ExitEnd, exitPos), Vec2iUnit());
	color_t color = colorExit;

	if (gCampaign.Entry.Mode == CAMPAIGN_MODE_DOGFIGHT)
	{
		return;
	}
	
	exitPos = Vec2iScale(exitPos, scale);
	exitSize = Vec2iScale(exitSize, scale);
	exitPos = Vec2iAdd(exitPos, pos);

	if (flags & AUTOMAP_FLAGS_MASK)
	{
		color.a = MASK_ALPHA;
	}
	DrawRectangle(&gGraphicsDevice, exitPos, exitSize, color, DRAW_FLAG_LINE);
}

static void DisplaySummary(void)
{
	int i;
	char sScore[20];
	Vec2i pos;
	pos.y = gGraphicsDevice.cachedConfig.Res.y - 5 - FontH();

	for (i = 0; i < (int)gMission.missionData->Objectives.size; i++)
	{
		struct Objective *o = CArrayGet(&gMission.Objectives, i);
		MissionObjective *mo = CArrayGet(&gMission.missionData->Objectives, i);
		if (mo->Required > 0 || o->done > 0)
		{
			color_t textColor = colorWhite;
			pos.x = 5;
			// Objective color dot
			Draw_Rect(pos.x, (pos.y + 3), 2, 2, o->color);

			pos.x += 5;

			sprintf(sScore, "(%d)", o->done);

			if (mo->Required <= 0)
			{
				textColor = colorPurple;
			}
			else if (o->done >= mo->Required)
			{
				textColor = colorRed;
			}
			pos = FontStrMask(mo->Description, pos, textColor);
			pos.x += 5;
			FontStrMask(sScore, pos, textColor);
			pos.y -= (FontH() + 1);
		}
	}
}

color_t DoorColor(int x, int y)
{
	int l = MapGetDoorKeycardFlag(&gMap, Vec2iNew(x, y));

	switch (l) {
	case FLAGS_KEYCARD_YELLOW:
		return colorYellowDoor;
	case FLAGS_KEYCARD_GREEN:
		return colorGreenDoor;
	case FLAGS_KEYCARD_BLUE:
		return colorBlueDoor;
	case FLAGS_KEYCARD_RED:
		return colorRedDoor;
	default:
		return colorDoor;
	}
}

void DrawDot(TTileItem *t, color_t color, Vec2i pos, int scale)
{
	Vec2i dotPos = Vec2iNew(t->x / TILE_WIDTH, t->y / TILE_HEIGHT);
	pos = Vec2iAdd(pos, Vec2iScale(dotPos, scale));
	Draw_Rect(pos.x, pos.y, scale, scale, color);
}

static void DrawMap(
	Map *map,
	Vec2i center, Vec2i centerOn, Vec2i size,
	int scale, int flags)
{
	int x, y;
	Vec2i mapPos = Vec2iAdd(center, Vec2iScale(centerOn, -scale));
	for (y = 0; y < gMap.Size.y; y++)
	{
		int i;
		for (i = 0; i < scale; i++)
		{
			for (x = 0; x < gMap.Size.x; x++)
			{
				Tile *tile = MapGetTile(map, Vec2iNew(x, y));
				if (!(tile->flags & MAPTILE_IS_NOTHING) &&
					(tile->isVisited || (flags & AUTOMAP_FLAGS_SHOWALL)))
				{
					int j;
					for (j = 0; j < scale; j++)
					{
						Vec2i drawPos = Vec2iNew(
							mapPos.x + x*scale + j,
							mapPos.y + y*scale + i);
						color_t color = colorBlack;
						if (tile->flags & MAPTILE_IS_WALL)
						{
							color = colorWall;
						}
						else if (tile->flags & MAPTILE_NO_WALK)
						{
							color = DoorColor(x, y);
						}
						else if (tile->flags & MAPTILE_IS_NORMAL_FLOOR)
						{
							color = colorFloor;
						}
						else
						{
							color = colorRoom;
						}
						if (!ColorEquals(color, colorBlack))
						{
							if (flags & AUTOMAP_FLAGS_MASK)
							{
								color.a = MASK_ALPHA;
							}
							Draw_Point(drawPos.x, drawPos.y, color);
						}
					}
				}
			}
		}
	}
	if (flags & AUTOMAP_FLAGS_MASK)
	{
		color_t color = { 255, 255, 255, 128 };
		Draw_Rect(
			center.x - size.x / 2,
			center.y - size.y / 2,
			size.x, size.y,
			color);
	}
}

static void DrawTileItem(
	TTileItem *t, Tile *tile, Vec2i pos, int scale, int flags);
static void DrawObjectivesAndKeys(Map *map, Vec2i pos, int scale, int flags)
{
	for (int y = 0; y < map->Size.y; y++)
	{
		for (int x = 0; x < map->Size.x; x++)
		{
			Tile *tile = MapGetTile(map, Vec2iNew(x, y));
			for (int i = 0; i < (int)tile->things.size; i++)
			{
				ThingId *tid = CArrayGet(&tile->things, i);
				DrawTileItem(
					ThingIdGetTileItem(tid), tile, pos, scale, flags);
			}
		}
	}
}
static void DrawTileItem(
	TTileItem *t, Tile *tile, Vec2i pos, int scale, int flags)
{
	if ((t->flags & TILEITEM_OBJECTIVE) != 0)
	{
		int obj = ObjectiveFromTileItem(t->flags);
		MissionObjective *mobj =
			CArrayGet(&gMission.missionData->Objectives, obj);
		int objFlags = mobj->Flags;
		if (!(objFlags & OBJECTIVE_HIDDEN) ||
			(flags & AUTOMAP_FLAGS_SHOWALL))
		{
			if ((objFlags & OBJECTIVE_POSKNOWN) ||
				tile->isVisited ||
				(flags & AUTOMAP_FLAGS_SHOWALL))
			{
				DisplayObjective(t, obj, pos, scale, flags);
			}
		}
	}
	else if (t->kind == KIND_OBJECT && tile->isVisited)
	{
		color_t dotColor = colorBlack;
		switch (((TObject *)CArrayGet(&gObjs, t->id))->Type)
		{
		case OBJ_KEYCARD_RED:
			dotColor = colorRedDoor;
			break;
		case OBJ_KEYCARD_BLUE:
			dotColor = colorBlueDoor;
			break;
		case OBJ_KEYCARD_GREEN:
			dotColor = colorGreenDoor;
			break;
		case OBJ_KEYCARD_YELLOW:
			dotColor = colorYellowDoor;
			break;
		default:
			break;
		}
		if (!ColorEquals(dotColor, colorBlack))
		{
			DrawDot(t, dotColor, pos, scale);
		}
	}
}

void AutomapDraw(int flags, bool showExit)
{
	color_t mask = { 0, 128, 0, 255 };
	Vec2i mapCenter = Vec2iNew(
		gGraphicsDevice.cachedConfig.Res.x / 2,
		gGraphicsDevice.cachedConfig.Res.y / 2);
	Vec2i centerOn = Vec2iNew(gMap.Size.x / 2, gMap.Size.y / 2);
	Vec2i pos = Vec2iAdd(mapCenter, Vec2iScale(centerOn, -MAP_FACTOR));

	// Draw faded green overlay
	for (int y = 0; y < gGraphicsDevice.cachedConfig.Res.y; y++)
	{
		for (int x = 0; x < gGraphicsDevice.cachedConfig.Res.x; x++)
		{
			DrawPointMask(&gGraphicsDevice, Vec2iNew(x, y), mask);
		}
	}

	DrawMap(&gMap, mapCenter, centerOn, gMap.Size, MAP_FACTOR, flags);
	DrawObjectivesAndKeys(&gMap, pos, MAP_FACTOR, flags);

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		if (IsPlayerAlive(i))
		{
			DisplayPlayer(CArrayGet(&gActors, gPlayerIds[i]), pos, MAP_FACTOR);
		}
	}

	if (showExit)
	{
		DisplayExit(pos, MAP_FACTOR, flags);
	}
	DisplaySummary();
}

void AutomapDrawRegion(
	Map *map,
	Vec2i pos, Vec2i size, Vec2i mapCenter,
	int scale, int flags, bool showExit)
{
	Vec2i centerOn;
	BlitClipping oldClip = gGraphicsDevice.clipping;
	int i;
	GraphicsSetBlitClip(
		&gGraphicsDevice,
		pos.x, pos.y, pos.x + size.x - 1, pos.y + size.y - 1);
	pos = Vec2iAdd(pos, Vec2iScaleDiv(size, 2));
	DrawMap(map, pos, mapCenter, size, scale, flags);
	centerOn = Vec2iAdd(pos, Vec2iScale(mapCenter, -scale));
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		if (IsPlayerAlive(i))
		{
			TActor *player = CArrayGet(&gActors, gPlayerIds[i]);
			DisplayPlayer(player, centerOn, scale);
		}
	}
	DrawObjectivesAndKeys(&gMap, centerOn, scale, flags);
	if (showExit)
	{
		DisplayExit(centerOn, scale, flags);
	}
	GraphicsSetBlitClip(
		&gGraphicsDevice,
		oldClip.left, oldClip.top, oldClip.right, oldClip.bottom);
}
