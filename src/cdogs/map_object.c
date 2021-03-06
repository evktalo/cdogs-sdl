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
#include "map_object.h"

#include "map.h"
#include "pics.h"

// +--------------------+
// |  Map objects info  |
// +--------------------+


static MapObject mapItems[] =
{
	{ OFSPIC_BARREL2, OFSPIC_WRECKEDBARREL2, "barrel_blue", 8, 6, 40, MAPOBJ_OUTSIDE },
	{ OFSPIC_BOX, OFSPIC_WRECKEDBOX, "box", 8, 6, 20, MAPOBJ_INOPEN },
	{ OFSPIC_BOX2, OFSPIC_WRECKEDBOX, "box2", 8, 6, 20, MAPOBJ_INOPEN },
	{ OFSPIC_CABINET, OFSPIC_WRECKEDCABINET, "cabinet", 8, 6, 20, MAPOBJ_INSIDE | MAPOBJ_ONEWALLPLUS | MAPOBJ_INTERIOR | MAPOBJ_FREEINFRONT },
	{ OFSPIC_PLANT, OFSPIC_WRECKEDPLANT, "plant", 4, 3, 20, MAPOBJ_INSIDE | MAPOBJ_ONEWALLPLUS },
	{ OFSPIC_TABLE, OFSPIC_WRECKEDTABLE, "table", 8, 6, 20, MAPOBJ_INSIDE | MAPOBJ_NOWALLS },
	{ OFSPIC_CHAIR, OFSPIC_WRECKEDCHAIR, "chair", 4, 3, 20, MAPOBJ_INSIDE | MAPOBJ_NOWALLS },
	{ OFSPIC_ROD, OFSPIC_WRECKEDCHAIR, "rod", 4, 3, 60, MAPOBJ_INOPEN },
	{ OFSPIC_SKULLBARREL_WOOD, OFSPIC_WRECKEDBARREL_WOOD, "barrel_skull", 8, 6, 40, MAPOBJ_OUTSIDE | MAPOBJ_EXPLOSIVE },
	{ OFSPIC_BARREL_WOOD, OFSPIC_WRECKEDBARREL_WOOD, "barrel_wood", 8, 6, 40, MAPOBJ_OUTSIDE },
	{ OFSPIC_GRAYBOX, OFSPIC_WRECKEDBOX_WOOD, "box_gray", 8, 6, 20, MAPOBJ_OUTSIDE },
	{ OFSPIC_GREENBOX, OFSPIC_WRECKEDBOX_WOOD, "box_green", 8, 6, 20, MAPOBJ_OUTSIDE },
	{ OFSPIC_OGRESTATUE, OFSPIC_WRECKEDSAFE, "statue_ogre", 8, 6, 80, MAPOBJ_INSIDE | MAPOBJ_ONEWALLPLUS | MAPOBJ_INTERIOR | MAPOBJ_FREEINFRONT },
	{ OFSPIC_WOODTABLE_CANDLE, OFSPIC_WRECKEDTABLE, "table_wood_candle", 8, 6, 20, MAPOBJ_INSIDE | MAPOBJ_NOWALLS },
	{ OFSPIC_WOODTABLE, OFSPIC_WRECKEDBOX_WOOD, "table_wood", 8, 6, 20, MAPOBJ_INSIDE | MAPOBJ_NOWALLS },
	{ OFSPIC_TREE, OFSPIC_TREE_REMAINS, "tree_dead", 4, 3, 40, MAPOBJ_INOPEN },
	{ OFSPIC_BOOKSHELF, OFSPIC_WRECKEDBOX_WOOD, "bookshelf", 8, 3, 20, MAPOBJ_INSIDE | MAPOBJ_ONEWALLPLUS | MAPOBJ_INTERIOR | MAPOBJ_FREEINFRONT },
	{ OFSPIC_WOODENBOX, OFSPIC_WRECKEDBOX_WOOD, "box_wood", 8, 6, 20, MAPOBJ_OUTSIDE },
	{ OFSPIC_CLOTHEDTABLE, OFSPIC_WRECKEDBOX_WOOD, "table_clothed", 8, 6, 20, MAPOBJ_INSIDE | MAPOBJ_NOWALLS },
	{ OFSPIC_STEELTABLE, OFSPIC_WRECKEDSAFE, "table_steel", 8, 6, 30, MAPOBJ_INSIDE | MAPOBJ_NOWALLS },
	{ OFSPIC_AUTUMNTREE, OFSPIC_AUTUMNTREE_REMAINS, "tree_autumn", 4, 3, 40, MAPOBJ_INOPEN },
	{ OFSPIC_GREENTREE, OFSPIC_GREENTREE_REMAINS, "tree", 8, 6, 40, MAPOBJ_INOPEN },

	// Used-to-be blow-ups
	{ OFSPIC_BOX3, OFSPIC_WRECKEDBOX3, "box3", 8, 6, 40, MAPOBJ_OUTSIDE | MAPOBJ_EXPLOSIVE | MAPOBJ_QUAKE },
	{ OFSPIC_SAFE, OFSPIC_WRECKEDSAFE, "safe", 8, 6, 100, MAPOBJ_INSIDE | MAPOBJ_ONEWALLPLUS | MAPOBJ_INTERIOR | MAPOBJ_FREEINFRONT },
	{ OFSPIC_REDBOX, OFSPIC_WRECKEDBOX_WOOD, "box_red", 8, 6, 40, MAPOBJ_OUTSIDE | MAPOBJ_FLAMMABLE },
	{ OFSPIC_LABTABLE, OFSPIC_WRECKEDSAFE, "table_lab", 8, 6, 60, MAPOBJ_INSIDE | MAPOBJ_ONEWALLPLUS | MAPOBJ_INTERIOR | MAPOBJ_FREEINFRONT | MAPOBJ_POISONOUS },
	{ OFSPIC_TERMINAL, OFSPIC_WRECKEDBOX_WOOD, "terminal", 8, 6, 40, MAPOBJ_INSIDE | MAPOBJ_NOWALLS },
	{ OFSPIC_BARREL, OFSPIC_WRECKEDBARREL, "barrel", 8, 6, 40, MAPOBJ_OUTSIDE | MAPOBJ_FLAMMABLE },
	{ OFSPIC_ROCKET, OFSPIC_BURN, "rocket", 8, 6, 40, MAPOBJ_OUTSIDE | MAPOBJ_EXPLOSIVE | MAPOBJ_QUAKE },
	{ OFSPIC_EGG, OFSPIC_EGG_REMAINS, "egg", 8, 6, 30, MAPOBJ_IMPASSABLE | MAPOBJ_CANBESHOT },
	{ OFSPIC_BLOODSTAIN, 0, "bloodstain", 0, 0, 0, MAPOBJ_ON_WALL },
	{ OFSPIC_WALL_SKULL, 0, "wall_skull", 0, 0, 0, MAPOBJ_ON_WALL },
	{ OFSPIC_BONE_N_BLOOD, 0, "bone_blood", 0, 0, 0, 0 },
	{ OFSPIC_BULLET_MARKS, 0, "bulletmarks", 0, 0, 0, MAPOBJ_ON_WALL },
	{ OFSPIC_SKULL, 0, "skull", 0, 0, 0, 0 },
	{ OFSPIC_BLOOD, 0, "blood", 0, 0, 0, 0 },
	{ OFSPIC_SCRATCH, 0, "scratch", 0, 0, 0, MAPOBJ_ON_WALL },
	{ OFSPIC_WALL_STUFF, 0, "wall_stuff", 0, 0, 0, MAPOBJ_ON_WALL },
	{ OFSPIC_WALL_GOO, 0, "wall_goo", 0, 0, 0, MAPOBJ_ON_WALL },
	{ OFSPIC_GOO, 0, "goo", 0, 0, 0, 0 }
};
#define ITEMS_COUNT (sizeof(mapItems)/sizeof(MapObject))

MapObject *MapObjectGet(int item)
{
	return &mapItems[item];
}
int MapObjectGetCount(void)
{
	return ITEMS_COUNT;
}
int MapObjectGetDestructibleCount(void)
{
	int i;
	for (i = 0; i < (int)ITEMS_COUNT; i++)
	{
		if (!(mapItems[i].flags & MAPOBJ_IMPASSABLE))
		{
			break;
		}
	}
	return i;
}

Pic *MapObjectGetPic(MapObject *mo, PicManager *pm, Vec2i *offset)
{
	const TOffsetPic *ofpic = &cGeneralPics[mo->pic];
	Pic *pic;
	if (!mo->picName || mo->picName[0] == '\0')
	{
		goto defaultPic;
	}
	pic = PicManagerGetPic(pm, mo->picName);
	if (!pic)
	{
		goto defaultPic;
	}
	*offset = Vec2iNew(ofpic->dx, ofpic->dy);
	return pic;

defaultPic:
	pic = PicManagerGetFromOld(pm, ofpic->picIndex);
	*offset = Vec2iNew(ofpic->dx, ofpic->dy);
	return pic;
}

int MapObjectIsTileOK(
	MapObject *obj, unsigned short tile, int isEmpty, unsigned short tileAbove)
{
	tile &= MAP_MASKACCESS;
	if (tile != MAP_FLOOR && tile != MAP_SQUARE && tile != MAP_ROOM)
	{
		return 0;
	}
	if (!isEmpty)
	{
		return 0;
	}
	tileAbove &= MAP_MASKACCESS;
	if ((obj->flags & MAPOBJ_ON_WALL) && tileAbove != MAP_WALL)
	{
		return 0;
	}
	return 1;
}
int MapObjectIsTileOKStrict(
	MapObject *obj, unsigned short tile, int isEmpty,
	unsigned short tileAbove, unsigned short tileBelow,
	int numWallsAdjacent, int numWallsAround)
{
	if (!MapObjectIsTileOK(obj, tile, isEmpty, tileAbove))
	{
		return 0;
	}
	unsigned short tileAccess = tile & MAP_MASKACCESS;
	if (tile & MAP_LEAVEFREE)
	{
		return 0;
	}

	if ((obj->flags & MAPOBJ_ROOMONLY) && tileAccess != MAP_ROOM)
	{
		return 0;
	}

	if ((obj->flags & MAPOBJ_NOTINROOM) && tileAccess == MAP_ROOM)
	{
		return 0;
	}

	if ((obj->flags & MAPOBJ_FREEINFRONT) != 0 &&
		(tileBelow & MAP_MASKACCESS) != MAP_FLOOR &&
		(tileBelow & MAP_MASKACCESS) != MAP_SQUARE &&
		(tileBelow & MAP_MASKACCESS) != MAP_ROOM)
	{
		return 0;
	}

	if ((obj->flags & MAPOBJ_ONEWALL) && numWallsAdjacent != 1)
	{
		return 0;
	}

	if ((obj->flags & MAPOBJ_ONEWALLPLUS) && numWallsAdjacent < 1)
	{
		return 0;
	}

	if ((obj->flags & MAPOBJ_NOWALLS) && numWallsAround != 0)
	{
		return 0;
	}

	return 1;
}
