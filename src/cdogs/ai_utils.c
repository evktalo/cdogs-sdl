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
#include "ai_utils.h"

#include <assert.h>
#include <math.h>

#include "algorithms.h"
#include "collision.h"
#include "map.h"
#include "objs.h"
#include "weapon.h"


TActor *AIGetClosestPlayer(Vec2i fullpos)
{
	int i;
	int minDistance = -1;
	TActor *closestPlayer = NULL;
	for (i = 0; i < gOptions.numPlayers; i++)
	{
		if (IsPlayerAlive(i))
		{
			TActor *p = CArrayGet(&gActors, gPlayerIds[i]);
			Vec2i pPos = Vec2iFull2Real(p->Pos);
			int distance = CHEBYSHEV_DISTANCE(
				fullpos.x, fullpos.y, pPos.x, pPos.y);
			if (!closestPlayer || distance < minDistance)
			{
				closestPlayer = p;
				minDistance = distance;
			}
		}
	}
	return closestPlayer;
}

static TActor *AIGetClosestActor(Vec2i from, int (*compFunc)(TActor *))
{
	// Search all the actors and find the closest one that
	// satisfies the condition
	TActor *closest = NULL;
	int minDistance = -1;
	for (int i = 0; i < (int)gActors.size; i++)
	{
		TActor *a = CArrayGet(&gActors, i);
		if (!a->isInUse || a->dead)
		{
			continue;
		}
		// Never target invulnerables or civilians
		if (a->flags & (FLAGS_INVULNERABLE | FLAGS_PENALTY))
		{
			continue;
		}
		if (compFunc(a))
		{
			int distance =
				CHEBYSHEV_DISTANCE(from.x, from.y, a->Pos.x, a->Pos.y);
			if (!closest || distance < minDistance)
			{
				minDistance = distance;
				closest = a;
			}
		}
	}
	return closest;
}

static int IsGood(TActor *a)
{
	return a->pData || (a->flags & FLAGS_GOOD_GUY);
}
static int IsBad(TActor *a)
{
	return !IsGood(a);
}
TActor *AIGetClosestEnemy(Vec2i from, int flags, int isPlayer)
{
	if (!isPlayer && !(flags & FLAGS_GOOD_GUY))
	{
		// we are bad; look for good guys
		return AIGetClosestActor(from, IsGood);
	}
	else
	{
		// we are good; look for bad guys
		return AIGetClosestActor(from, IsBad);
	}
}

static int IsGoodAndVisible(TActor *a)
{
	return IsGood(a) && (a->flags & FLAGS_VISIBLE);
}
static int IsBadAndVisible(TActor *a)
{
	return IsBad(a) && (a->flags & FLAGS_VISIBLE);
}
TActor *AIGetClosestVisibleEnemy(Vec2i from, int flags, int isPlayer)
{
	if (!isPlayer && !(flags & FLAGS_GOOD_GUY))
	{
		// we are bad; look for good guys
		return AIGetClosestActor(from, IsGoodAndVisible);
	}
	else
	{
		// we are good; look for bad guys
		return AIGetClosestActor(from, IsBadAndVisible);
	}
}

Vec2i AIGetClosestPlayerPos(Vec2i pos)
{
	TActor *closestPlayer = AIGetClosestPlayer(pos);
	if (closestPlayer)
	{
		return Vec2iFull2Real(closestPlayer->Pos);
	}
	else
	{
		return pos;
	}
}

int AIReverseDirection(int cmd)
{
	if (cmd & (CMD_LEFT | CMD_RIGHT))
	{
		cmd ^= CMD_LEFT | CMD_RIGHT;
	}
	if (cmd & (CMD_UP | CMD_DOWN))
	{
		cmd ^= CMD_UP | CMD_DOWN;
	}
	return cmd;
}

typedef bool (*IsBlockedFunc)(void *, Vec2i);
static bool AIHasClearLine(
	Vec2i from, Vec2i to, IsBlockedFunc isBlockedFunc);
static bool IsTileWalkable(Map *map, Vec2i pos);
static bool IsPosNoWalk(void *data, Vec2i pos);
static bool IsTileWalkableAroundObjects(Map *map, Vec2i pos);
static bool IsPosNoWalkAroundObjects(void *data, Vec2i pos);
bool AIHasClearPath(
	const Vec2i from, const Vec2i to, const bool ignoreObjects)
{
	IsBlockedFunc f = ignoreObjects ? IsPosNoWalk : IsPosNoWalkAroundObjects;
	return AIHasClearLine(from, to, f);
}
static bool AIHasClearLine(
	Vec2i from, Vec2i to, IsBlockedFunc isBlockedFunc)
{
	// Find all tiles that overlap with the line (from, to)
	// Uses Bresenham that crosses interiors of tiles
	HasClearLineData data;
	data.IsBlocked = isBlockedFunc;
	data.data = &gMap;

	return HasClearLineXiaolinWu(from, to, &data);
}
static bool IsTileWalkableOrOpenable(Map *map, Vec2i pos);
static bool IsTileWalkable(Map *map, Vec2i pos)
{
	if (!IsTileWalkableOrOpenable(map, pos))
	{
		return false;
	}
	// Check if tile has a dangerous (explosive) item on it
	// For AI, we don't want to shoot it, so just walk around
	Tile *t = MapGetTile(map, pos);
	for (int i = 0; i < (int)t->things.size; i++)
	{
		ThingId *tid = CArrayGet(&t->things, i);
		// Only look for explosive objects
		if (tid->Kind != KIND_OBJECT)
		{
			continue;
		}
		TObject *o = CArrayGet(&gObjs, tid->Id);
		if (o->flags & OBJFLAG_DANGEROUS)
		{
			return false;
		}
	}
	return true;
}
static bool IsPosNoWalk(void *data, Vec2i pos)
{
	return !IsTileWalkable(data, Vec2iToTile(pos));
}
static bool IsTileWalkableAroundObjects(Map *map, Vec2i pos)
{
	if (!IsTileWalkableOrOpenable(map, pos))
	{
		return false;
	}
	// Check if tile has any item on it
	Tile *t = MapGetTile(map, pos);
	for (int i = 0; i < (int)t->things.size; i++)
	{
		ThingId *tid = CArrayGet(&t->things, i);
		if (tid->Kind == KIND_OBJECT)
		{
			// Check that the object is not a pickup type
			TObject *o = CArrayGet(&gObjs, tid->Id);
			if (o->Type == OBJ_NONE && !(o->tileItem.flags & TILEITEM_IS_WRECK))
			{
				return false;
			}
		}
		else if (tid->Kind == KIND_CHARACTER)
		{
			switch (gConfig.Game.AllyCollision)
			{
			case ALLYCOLLISION_NORMAL:
				return false;
			case ALLYCOLLISION_REPEL:
				// TODO: implement
				// Need to know collision team of player
				// to know if collision will result in repelling
				break;
			case ALLYCOLLISION_NONE:
				continue;
			default:
				CASSERT(false, "unknown collision type");
				break;
			}
		}
	}
	return true;
}
static bool IsPosNoWalkAroundObjects(void *data, Vec2i pos)
{
	return !IsTileWalkableAroundObjects(data, Vec2iToTile(pos));
}
static bool IsTileWalkableOrOpenable(Map *map, Vec2i pos)
{
	int tileFlags = MapGetTile(map, pos)->flags;
	if (!(tileFlags & MAPTILE_NO_WALK))
	{
		return true;
	}
	if (tileFlags & MAPTILE_OFFSET_PIC)
	{
		// A door; check if we can open it
		int keycard = MapGetDoorKeycardFlag(map, pos);
		if (!keycard)
		{
			// Unlocked door
			return true;
		}
		return !!(keycard & gMission.flags);
	}
	// Otherwise, we cannot walk over this tile
	return false;
}
static bool IsPosNoSee(void *data, Vec2i pos);
bool AIHasClearShot(const Vec2i from, const Vec2i to)
{
	// Perform 4 line tests - above, below, left and right
	// This is to account for possible positions for the muzzle
	Vec2i fromOffset = from;

	const int pad = 2;
	fromOffset.x = from.x - (ACTOR_W + pad) / 2;
	if (Vec2iToTile(fromOffset).x >= 0 &&
		!AIHasClearLine(fromOffset, to, IsPosNoSee))
	{
		return false;
	}
	fromOffset.x = from.x + (ACTOR_W + pad) / 2;
	if (Vec2iToTile(fromOffset).x < gMap.Size.x &&
		!AIHasClearLine(fromOffset, to, IsPosNoSee))
	{
		return false;
	}
	fromOffset.x = from.x;
	fromOffset.y = from.y - (ACTOR_H + pad) / 2;
	if (Vec2iToTile(fromOffset).y >= 0 &&
		!AIHasClearLine(fromOffset, to, IsPosNoSee))
	{
		return false;
	}
	fromOffset.y = from.y + (ACTOR_H + pad) / 2;
	if (Vec2iToTile(fromOffset).y < gMap.Size.y &&
		!AIHasClearLine(fromOffset, to, IsPosNoSee))
	{
		return false;
	}
	return true;
}
static bool IsPosNoSee(void *data, Vec2i pos)
{
	return MapGetTile(data, Vec2iToTile(pos))->flags & MAPTILE_NO_SEE;
}

TObject *AIGetObjectRunningInto(TActor *a, int cmd)
{
	// Check the position just in front of the character;
	// check if there's a (non-dangerous) object in front of it
	Vec2i frontPos = Vec2iFull2Real(a->Pos);
	TTileItem *item;
	if (cmd & CMD_LEFT)
	{
		frontPos.x--;
	}
	else if (cmd & CMD_RIGHT)
	{
		frontPos.x++;
	}
	if (cmd & CMD_UP)
	{
		frontPos.y--;
	}
	else if (cmd & CMD_DOWN)
	{
		frontPos.y++;
	}
	item = GetItemOnTileInCollision(
		&a->tileItem,
		frontPos,
		TILEITEM_IMPASSABLE,
		CalcCollisionTeam(1, a),
		gCampaign.Entry.Mode == CAMPAIGN_MODE_DOGFIGHT);
	if (!item || item->kind != KIND_OBJECT)
	{
		return NULL;
	}
	return CArrayGet(&gObjs, item->id);
}


typedef struct
{
	Map *Map;
	TileSelectFunc IsTileOk;
} AStarContext;
static void AddTileNeighbors(
	ASNeighborList neighbors, void *node, void *context)
{
	Vec2i *v = node;
	int y;
	AStarContext *c = context;
	for (y = v->y - 1; y <= v->y + 1; y++)
	{
		int x;
		if (y < 0 || y >= c->Map->Size.y)
		{
			continue;
		}
		for (x = v->x - 1; x <= v->x + 1; x++)
		{
			float cost;
			Vec2i neighbor;
			neighbor.x = x;
			neighbor.y = y;
			if (x < 0 || x >= c->Map->Size.x)
			{
				continue;
			}
			if (x == v->x && y == v->y)
			{
				continue;
			}
			// if we're moving diagonally,
			// need to check the axis-aligned neighbours are also clear
			if (!c->IsTileOk(c->Map, Vec2iNew(x, y)) ||
				!c->IsTileOk(c->Map, Vec2iNew(v->x, y)) ||
				!c->IsTileOk(c->Map, Vec2iNew(x, v->y)))
			{
				continue;
			}
			// Calculate cost of direction
			// Note that there are different horizontal and vertical costs,
			// due to the tiles being non-square
			// Slightly prefer axes instead of diagonals
			if (x != v->x && y != v->y)
			{
				cost = TILE_WIDTH * 1.1f;
			}
			else if (x != v->x)
			{
				cost = TILE_WIDTH;
			}
			else
			{
				cost = TILE_HEIGHT;
			}
			ASNeighborListAdd(neighbors, &neighbor, cost);
		}
	}
}
static float AStarHeuristic(void *fromNode, void *toNode, void *context)
{
	// Simple Euclidean
	Vec2i *v1 = fromNode;
	Vec2i *v2 = toNode;
	UNUSED(context);
	return (float)sqrt(DistanceSquared(
		Vec2iCenterOfTile(*v1), Vec2iCenterOfTile(*v2)));
}
static ASPathNodeSource cPathNodeSource =
{
	sizeof(Vec2i), AddTileNeighbors, AStarHeuristic, NULL, NULL
};

// Use pathfinding to check that there is a path between
// source and destination tiles
bool AIHasPath(const Vec2i from, const Vec2i to, const bool ignoreObjects)
{
	// Quick first test: check there is a clear path
	if (AIHasClearPath(from, to, ignoreObjects))
	{
		return true;
	}
	// Pathfind
	AStarContext ac;
	ac.Map = &gMap;
	ac.IsTileOk = ignoreObjects ? IsTileWalkable : IsTileWalkableAroundObjects;
	Vec2i fromTile = Vec2iToTile(from);
	Vec2i toTile = MapSearchTileAround(ac.Map, Vec2iToTile(to), ac.IsTileOk);
	ASPath path = ASPathCreate(&cPathNodeSource, &ac, &fromTile, &toTile);
	size_t pathCount = ASPathGetCount(path);
	ASPathDestroy(path);
	return pathCount > 1;
}

static int AIGotoDirect(Vec2i a, Vec2i p)
{
	int cmd = 0;

	if (a.x < p.x)		cmd |= CMD_RIGHT;
	else if (a.x > p.x)	cmd |= CMD_LEFT;

	if (a.y < p.y)		cmd |= CMD_DOWN;
	else if (a.y > p.y)	cmd |= CMD_UP;

	return cmd;
}
// Follow the current A* path
static int AStarFollow(
	AIGotoContext *c, Vec2i currentTile, TTileItem *i, Vec2i a)
{
	Vec2i *pathTile = ASPathGetNode(c->Path, c->PathIndex);
	c->IsFollowing = 1;
	// Check if we need to follow the next step in the path
	// Note: need to make sure the actor is fully within the current tile
	// otherwise it may get stuck at corners
	if (Vec2iEqual(currentTile, *pathTile) &&
		IsTileItemInsideTile(i, currentTile))
	{
		c->PathIndex++;
		pathTile = ASPathGetNode(c->Path, c->PathIndex);
		c->IsFollowing = 0;
	}
	// Go directly to the center of the next tile
	return AIGotoDirect(a, Vec2iCenterOfTile(*pathTile));
}
// Check that we are still close to the start of the A* path,
// and the end of the path is close to our goal
static int AStarCloseToPath(
	AIGotoContext *c, Vec2i currentTile, Vec2i goalTile)
{
	Vec2i *pathTile;
	Vec2i *pathEnd;
	if (!c ||
		c->PathIndex >= (int)ASPathGetCount(c->Path) - 1) // at end of path
	{
		return 0;
	}
	// Check if we're too far from the current start of the path
	pathTile = ASPathGetNode(c->Path, c->PathIndex);
	if (CHEBYSHEV_DISTANCE(
		currentTile.x, currentTile.y, pathTile->x, pathTile->y) > 2)
	{
		return 0;
	}
	// Check if we're too far from the end of the path
	pathEnd = ASPathGetNode(c->Path, ASPathGetCount(c->Path) - 1);
	if (CHEBYSHEV_DISTANCE(
		goalTile.x, goalTile.y, pathEnd->x, pathEnd->y) > 0)
	{
		return 0;
	}
	return 1;
}
int AIGoto(TActor *actor, Vec2i p, bool ignoreObjects)
{
	Vec2i a = Vec2iFull2Real(actor->Pos);
	Vec2i currentTile = Vec2iToTile(a);
	Vec2i goalTile = Vec2iToTile(p);
	AIGotoContext *c = &actor->aiContext->Goto;

	// If we are already there, bail
	// This can happen if AI is trying to track the player,
	// but the player has died, for example.
	if (Vec2iEqual(currentTile, goalTile))
	{
		return 0;
	}

	// If we are currently following an A* path,
	// and it is still valid, keep following it until
	// we have reached a new tile
	if (c && c->IsFollowing && AStarCloseToPath(c, currentTile, goalTile))
	{
		return AStarFollow(c, currentTile, &actor->tileItem, a);
	}
	else if (AIHasClearPath(a, p, ignoreObjects))
	{
		// Simple case: if there's a clear line between AI and target,
		// walk straight towards it
		return AIGotoDirect(a, p);
	}
	else
	{
		// We need to recalculate A*

		AStarContext ac;
		ac.Map = &gMap;
		ac.IsTileOk =
			ignoreObjects ? IsTileWalkable : IsTileWalkableAroundObjects;
		// First, if the goal tile is blocked itself,
		// find a nearby tile that can be walked to
		c->Goal = MapSearchTileAround(ac.Map, goalTile, ac.IsTileOk);

		c->PathIndex = 1;	// start navigating to the next path node
		ASPathDestroy(c->Path);
		c->Path = ASPathCreate(
			&cPathNodeSource, &ac, &currentTile, &c->Goal);

		// In case we can't calculate A* for some reason,
		// try simple navigation again
		if (ASPathGetCount(c->Path) <= 1)
		{
			debug(
				D_MAX,
				"Error: can't calculate path from {%d, %d} to {%d, %d}",
				currentTile.x, currentTile.y,
				goalTile.x, goalTile.y);
			return AIGotoDirect(a, p);
		}

		return AStarFollow(c, currentTile, &actor->tileItem, a);
	}
}

int AIHunt(TActor *actor, Vec2i targetPos)
{
	Vec2i fullPos = Vec2iAdd(
		actor->Pos,
		GunGetMuzzleOffset(ActorGetGun(actor)->Gun, actor->direction));
	const int dx = abs(targetPos.x - fullPos.x);
	const int dy = abs(targetPos.y - fullPos.y);

	int cmd = 0;
	if (2 * dx > dy)
	{
		if (fullPos.x < targetPos.x)		cmd |= CMD_RIGHT;
		else if (fullPos.x > targetPos.x)	cmd |= CMD_LEFT;
	}
	if (2 * dy > dx)
	{
		if (fullPos.y < targetPos.y)		cmd |= CMD_DOWN;
		else if (fullPos.y > targetPos.y)	cmd |= CMD_UP;
	}
	// If it's a coward, reverse directions...
	if (actor->flags & FLAGS_RUNS_AWAY)
	{
		cmd = AIReverseDirection(cmd);
	}

	return cmd;
}
int AIHuntClosest(TActor *actor)
{
	Vec2i targetPos = actor->Pos;
	if (!(actor->pData || (actor->flags & FLAGS_GOOD_GUY)))
	{
		targetPos = AIGetClosestPlayerPos(actor->Pos);
	}

	if (actor->flags & FLAGS_VISIBLE)
	{
		TActor *a = AIGetClosestEnemy(
			actor->Pos, actor->flags, !!actor->pData);
		if (a)
		{
			targetPos = a->Pos;
		}
	}
	return AIHunt(actor, targetPos);
}

void AIContextTerminate(void *aiContext)
{
	AIGotoContext *c = aiContext;
	if (c)
	{
		ASPathDestroy(c->Path);
	}
	CFREE(c);
}
