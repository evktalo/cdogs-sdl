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
#include "editor_brush.h"

#include <assert.h>

#include <cdogs/algorithms.h>
#include <cdogs/map.h>
#include <cdogs/map_build.h>
#include <cdogs/mission_convert.h>


const char *BrushTypeStr(BrushType t)
{
	switch (t)
	{
	case BRUSHTYPE_POINT:
		return "Point";
	case BRUSHTYPE_LINE:
		return "Line";
	case BRUSHTYPE_BOX:
		return "Box";
	case BRUSHTYPE_BOX_FILLED:
		return "Box Filled";
	case BRUSHTYPE_ROOM:
		return "Room";
	case BRUSHTYPE_SELECT:
		return "Select";
	default:
		assert(0 && "unknown brush type");
		return "";
	}
}
BrushType StrBrushType(const char *s)
{
	if (strcmp(s, "Point") == 0)
	{
		return BRUSHTYPE_POINT;
	}
	else if (strcmp(s, "Line") == 0)
	{
		return BRUSHTYPE_LINE;
	}
	else if (strcmp(s, "Box") == 0)
	{
		return BRUSHTYPE_BOX;
	}
	else if (strcmp(s, "Box Filled") == 0)
	{
		return BRUSHTYPE_BOX_FILLED;
	}
	else if (strcmp(s, "Room") == 0)
	{
		return BRUSHTYPE_ROOM;
	}
	else if (strcmp(s, "Select") == 0)
	{
		return BRUSHTYPE_SELECT;
	}
	else
	{
		assert(0 && "unknown brush type");
		return BRUSHTYPE_POINT;
	}
}


void EditorBrushInit(EditorBrush *b)
{
	memset(b, 0, sizeof *b);
	b->Type = BRUSHTYPE_POINT;
	b->MainType = MAP_WALL;
	b->SecondaryType = MAP_FLOOR;
	b->PaintType = b->MainType;
	b->IsActive = 0;
	b->IsPainting = 0;
	b->BrushSize = 1;
	b->LastPos = Vec2iNew(-1, -1);
	b->Pos = Vec2iNew(-1, -1);
	b->GuideImageAlpha = 64;
	CArrayInit(&b->HighlightedTiles, sizeof(Vec2i));
}
void EditorBrushTerminate(EditorBrush *b)
{
	CArrayTerminate(&b->HighlightedTiles);
	SDL_FreeSurface(b->GuideImageSurface);
}

static void EditorBrushHighlightPoint(void *data, Vec2i p)
{
	EditorBrush *b = data;
	Vec2i v;
	for (v.y = 0; v.y < b->BrushSize; v.y++)
	{
		for (v.x = 0; v.x < b->BrushSize; v.x++)
		{
			Vec2i pos = Vec2iAdd(p, v);
			CArrayPushBack(&b->HighlightedTiles, &pos);
		}
	}
}
void EditorBrushSetHighlightedTiles(EditorBrush *b)
{
	int useSimpleHighlight = 1;
	switch (b->Type)
	{
	case BRUSHTYPE_POINT:
		useSimpleHighlight = 1;
		break;
	case BRUSHTYPE_LINE:
		if (b->IsPainting)
		{
			useSimpleHighlight = 0;
			// highlight a line
			CArrayClear(&b->HighlightedTiles);
			AlgoLineDrawData data;
			data.Draw = EditorBrushHighlightPoint;
			data.data = b;
			BresenhamLineDraw(b->LastPos, b->Pos, &data);
		}
		break;
	case BRUSHTYPE_BOX:	// fallthrough
	case BRUSHTYPE_ROOM:	// fallthrough
	case BRUSHTYPE_SET_EXIT:
		if (b->IsPainting)
		{
			Vec2i v;
			Vec2i d = Vec2iNew(
				b->Pos.x > b->LastPos.x ? 1 : -1,
				b->Pos.y > b->LastPos.y ? 1 : -1);
			useSimpleHighlight = 0;
			CArrayClear(&b->HighlightedTiles);
			for (v.y = b->LastPos.y; v.y != b->Pos.y + d.y; v.y += d.y)
			{
				for (v.x = b->LastPos.x; v.x != b->Pos.x + d.x; v.x += d.x)
				{
					if (v.x == b->LastPos.x || v.x == b->Pos.x ||
						v.y == b->LastPos.y || v.y == b->Pos.y)
					{
						EditorBrushHighlightPoint(b, v);
					}
				}
			}
		}
		break;
	case BRUSHTYPE_BOX_FILLED:
		if (b->IsPainting)
		{
			Vec2i v;
			Vec2i d = Vec2iNew(
				b->Pos.x > b->LastPos.x ? 1 : -1,
				b->Pos.y > b->LastPos.y ? 1 : -1);
			useSimpleHighlight = 0;
			CArrayClear(&b->HighlightedTiles);
			for (v.y = b->LastPos.y; v.y != b->Pos.y + d.y; v.y += d.y)
			{
				for (v.x = b->LastPos.x; v.x != b->Pos.x + d.x; v.x += d.x)
				{
					EditorBrushHighlightPoint(b, v);
				}
			}
		}
		break;
	case BRUSHTYPE_SELECT:
		if (b->IsPainting)
		{
			if (b->IsMoving)
			{
				Vec2i v;
				Vec2i offset = Vec2iNew(
					b->Pos.x - b->DragPos.x, b->Pos.y - b->DragPos.y);
				useSimpleHighlight = 0;
				CArrayClear(&b->HighlightedTiles);
				for (v.y = 0; v.y < b->SelectionSize.y; v.y++)
				{
					for (v.x = 0; v.x < b->SelectionSize.x; v.x++)
					{
						Vec2i vOffset = Vec2iAdd(
							Vec2iAdd(v, b->SelectionStart), offset);
						EditorBrushHighlightPoint(b, vOffset);
					}
				}
			}
			else
			{
				// resize the selection
				Vec2i v;
				Vec2i d = Vec2iNew(
					b->Pos.x > b->LastPos.x ? 1 : -1,
					b->Pos.y > b->LastPos.y ? 1 : -1);
				useSimpleHighlight = 0;
				CArrayClear(&b->HighlightedTiles);
				for (v.y = b->LastPos.y; v.y != b->Pos.y + d.y; v.y += d.y)
				{
					for (v.x = b->LastPos.x; v.x != b->Pos.x + d.x; v.x += d.x)
					{
						EditorBrushHighlightPoint(b, v);
					}
				}
			}
		}
		else
		{
			// If there's a valid selection, draw that instead
			if (b->SelectionSize.x > 0 && b->SelectionSize.y > 0)
			{
				Vec2i v;
				useSimpleHighlight = 0;
				CArrayClear(&b->HighlightedTiles);
				for (v.y = 0; v.y < b->SelectionSize.y; v.y++)
				{
					for (v.x = 0; v.x < b->SelectionSize.x; v.x++)
					{
						Vec2i vOffset = Vec2iAdd(v, b->SelectionStart);
						EditorBrushHighlightPoint(b, vOffset);
					}
				}
			}
		}
	default:
		// do nothing
		break;
	}
	if (useSimpleHighlight)
	{
		// Simple highlight at brush tip based on brush size
		CArrayClear(&b->HighlightedTiles);
		EditorBrushHighlightPoint(b, b->Pos);
	}
}

static void SetTile(Mission *m, Vec2i pos, unsigned short tile)
{
	if (MissionTrySetTile(m, pos, tile))
	{
		MapSetTile(&gMap, pos, tile, m);
	}
}

typedef struct
{
	EditorBrush *brush;
	Mission *mission;
} EditorBrushPaintTilesAtData;
static void EditorBrushPaintTilesAt(void *data, Vec2i pos)
{
	Vec2i v;
	EditorBrushPaintTilesAtData *paintData = data;
	EditorBrush *b = paintData->brush;
	Mission *m = paintData->mission;
	for (v.y = 0; v.y < b->BrushSize; v.y++)
	{
		for (v.x = 0; v.x < b->BrushSize; v.x++)
		{
			SetTile(m, Vec2iAdd(pos, v), b->PaintType);
		}
	}
}
static void EditorBrushPaintLine(EditorBrush *b, Mission *m)
{
	// Draw tiles between the last point and the current point
	EditorBrushPaintTilesAtData paintData;
	paintData.brush = b;
	paintData.mission = m;
	if (b->IsPainting)
	{
		AlgoLineDrawData data;
		data.Draw = EditorBrushPaintTilesAt;
		data.data = &paintData;
		BresenhamLineDraw(b->LastPos, b->Pos, &data);
	}
	EditorBrushPaintTilesAt(&paintData, b->Pos);
	b->IsPainting = 1;
	b->LastPos = b->Pos;
}
typedef struct
{
	Mission *m;
	unsigned short fromType;
	unsigned short toType;
} PaintFloodFillData;
static void MissionFillTile(void *data, Vec2i v);
static bool MissionIsTileSame(void *data, Vec2i v);
EditorResult EditorBrushStartPainting(EditorBrush *b, Mission *m, int isMain)
{
	if (!b->IsPainting)
	{
		b->LastPos = b->Pos;
	}
	b->PaintType = isMain ? b->MainType : b->SecondaryType;
	switch (b->Type)
	{
	case BRUSHTYPE_POINT:
		b->IsPainting = 1;
		EditorBrushPaintLine(b, m);
		return EDITOR_RESULT_CHANGED;
	case BRUSHTYPE_LINE:	// fallthrough
	case BRUSHTYPE_BOX:	// fallthrough
	case BRUSHTYPE_BOX_FILLED:	// fallthrough
	case BRUSHTYPE_ROOM:	// fallthrough
	case BRUSHTYPE_SET_EXIT:
		// don't paint until the end
		break;
	case BRUSHTYPE_SELECT:
		// Perform state changes if we've started painting
		if (!b->IsPainting)
		{
			if (!b->IsMoving)
			{
				// check if the click was inside the selection
				// If so, start moving
				if (b->Pos.x >= b->SelectionStart.x &&
					b->Pos.y >= b->SelectionStart.y &&
					b->Pos.x < b->SelectionStart.x + b->SelectionSize.x &&
					b->Pos.y < b->SelectionStart.y + b->SelectionSize.y)
				{
					b->IsMoving = 1;
					b->DragPos = b->Pos;
				}
			}
		}
		break;
	case BRUSHTYPE_FILL:
		// Use flood fill to change all the tiles of the same type to
		// another type
		// Don't paint if target already same type
		if ((MissionGetTile(m, b->Pos) & MAP_MASKACCESS) != b->PaintType)
		{
			FloodFillData data;
			data.Fill = MissionFillTile;
			data.IsSame = MissionIsTileSame;
			PaintFloodFillData pData;
			pData.m = m;
			pData.fromType = MissionGetTile(m, b->Pos) & MAP_MASKACCESS;
			pData.toType = b->PaintType;
			data.data = &pData;
			if (FloodFill(b->Pos, &data))
			{
				return EDITOR_RESULT_CHANGED;
			}
		}
		return EDITOR_RESULT_NONE;
	case BRUSHTYPE_SET_PLAYER_START:
		if (MissionGetTile(m, b->Pos) == MAP_ROOM ||
			MissionGetTile(m, b->Pos) == MAP_FLOOR)
		{
			m->u.Static.Start = b->Pos;
			return EDITOR_RESULT_CHANGED;
		}
		break;
	case BRUSHTYPE_ADD_ITEM:
		if (isMain)
		{
			if (MissionStaticTryAddItem(m, b->ItemIndex, b->Pos))
			{
				return EDITOR_RESULT_CHANGED_AND_RELOAD;
			}
		}
		else
		{
			if (MissionStaticTryRemoveItemAt(m, b->Pos))
			{
				return EDITOR_RESULT_CHANGED_AND_RELOAD;
			}
		}
		break;
	case BRUSHTYPE_ADD_WRECK:
		if (isMain)
		{
			if (MissionStaticTryAddWreck(m, b->ItemIndex, b->Pos))
			{
				return EDITOR_RESULT_CHANGED_AND_RELOAD;
			}
		}
		else
		{
			if (MissionStaticTryRemoveWreckAt(m, b->Pos))
			{
				return EDITOR_RESULT_CHANGED_AND_RELOAD;
			}
		}
		break;
	case BRUSHTYPE_ADD_CHARACTER:
		if (isMain)
		{
			if (MissionStaticTryAddCharacter(m, b->ItemIndex, b->Pos))
			{
				return EDITOR_RESULT_CHANGED_AND_RELOAD;
			}
		}
		else
		{
			if (MissionStaticTryRemoveCharacterAt(m, b->Pos))
			{
				return EDITOR_RESULT_CHANGED_AND_RELOAD;
			}
		}
		break;
	case BRUSHTYPE_ADD_OBJECTIVE:
		if (isMain)
		{
			if (MissionStaticTryAddObjective(
				m, b->ItemIndex, b->Index2, b->Pos))
			{
				return EDITOR_RESULT_CHANGED_AND_RELOAD;
			}
		}
		else
		{
			if (MissionStaticTryRemoveObjectiveAt(m, b->Pos))
			{
				return EDITOR_RESULT_CHANGED_AND_RELOAD;
			}
		}
		break;
	case BRUSHTYPE_ADD_KEY:
		if (isMain)
		{
			if (MissionStaticTryAddKey(m, b->ItemIndex, b->Pos))
			{
				return EDITOR_RESULT_CHANGED_AND_RELOAD;
			}
		}
		else
		{
			if (MissionStaticTryRemoveKeyAt(m, b->Pos))
			{
				return EDITOR_RESULT_CHANGED_AND_RELOAD;
			}
		}
		break;
	case BRUSHTYPE_SET_KEY:
		if (isMain)
		{
			if (MissionStaticTrySetKey(m, b->ItemIndex, b->Pos))
			{
				return EDITOR_RESULT_CHANGED_AND_RELOAD;
			}
		}
		else
		{
			if (MissionStaticTryUnsetKeyAt(m, b->Pos))
			{
				return EDITOR_RESULT_CHANGED_AND_RELOAD;
			}
		}
		break;
	default:
		assert(0 && "unknown brush type");
		break;
	}
	b->IsPainting = 1;
	return EDITOR_RESULT_NONE;
}
static void MissionFillTile(void *data, Vec2i v)
{
	PaintFloodFillData *pData = data;
	SetTile(pData->m, v, pData->toType);
}
static bool MissionIsTileSame(void *data, Vec2i v)
{
	PaintFloodFillData *pData = data;
	return (MissionGetTile(pData->m, v) & MAP_MASKACCESS) == pData->fromType;
}
static void EditorBrushPaintBox(
	EditorBrush *b, Mission *m,
	unsigned short lineType, unsigned short fillType)
{
	// Draw the fill first, then the line
	// This will create the expected results when brush size is
	// greater than one, without having the box drawing routine made
	// aware of brush sizes
	Vec2i v;
	Vec2i d = Vec2iNew(
		b->Pos.x > b->LastPos.x ? 1 : -1,
		b->Pos.y > b->LastPos.y ? 1 : -1);
	EditorBrushPaintTilesAtData paintData;
	paintData.brush = b;
	paintData.mission = m;
	// Draw fill
	if (fillType != MAP_UNSET)
	{
		b->PaintType = fillType;
		for (v.y = b->LastPos.y; v.y != b->Pos.y + d.y; v.y += d.y)
		{
			for (v.x = b->LastPos.x; v.x != b->Pos.x + d.x; v.x += d.x)
			{
				if (v.x != b->LastPos.x && v.x != b->Pos.x &&
					v.y != b->LastPos.y && v.y != b->Pos.y)
				{
					EditorBrushPaintTilesAt(&paintData, v);
				}
			}
		}
	}
	// Draw edge
	if (lineType != MAP_UNSET)
	{
		b->PaintType = lineType;
		for (v.y = b->LastPos.y; v.y != b->Pos.y + d.y; v.y += d.y)
		{
			for (v.x = b->LastPos.x; v.x != b->Pos.x + d.x; v.x += d.x)
			{
				if (v.x == b->LastPos.x || v.x == b->Pos.x ||
					v.y == b->LastPos.y || v.y == b->Pos.y)
				{
					EditorBrushPaintTilesAt(&paintData, v);
				}
			}
		}
	}
}
EditorResult EditorBrushStopPainting(EditorBrush *b, Mission *m)
{
	EditorResult result = EDITOR_RESULT_NONE;
	if (b->IsPainting)
	{
		switch (b->Type)
		{
		case BRUSHTYPE_LINE:
			EditorBrushPaintLine(b, m);
			result = EDITOR_RESULT_CHANGED;
			break;
		case BRUSHTYPE_BOX:
			EditorBrushPaintBox(b, m, b->PaintType, MAP_UNSET);
			result = EDITOR_RESULT_CHANGED;
			break;
		case BRUSHTYPE_BOX_FILLED:
			EditorBrushPaintBox(b, m, b->PaintType, b->PaintType);
			result = EDITOR_RESULT_CHANGED;
			break;
		case BRUSHTYPE_ROOM:
			EditorBrushPaintBox(b, m, MAP_WALL, MAP_ROOM);
			result = EDITOR_RESULT_CHANGED;
			break;
		case BRUSHTYPE_SELECT:
			if (b->IsMoving)
			{
				// Move the tiles from the source to the target
				// Need to copy all the tiles to a temp buffer first in case
				// we are moving to an overlapped position
				CArray movedTiles;
				Vec2i v;
				int i;
				int delta;
				CArrayInit(&movedTiles, sizeof(unsigned short));
				// Copy tiles to temp from selection, setting them to MAP_FLOOR
				// in the process
				for (v.y = 0; v.y < b->SelectionSize.y; v.y++)
				{
					for (v.x = 0; v.x < b->SelectionSize.x; v.x++)
					{
						Vec2i vOffset = Vec2iAdd(v, b->SelectionStart);
						int idx = vOffset.y * m->Size.x + vOffset.x;
						unsigned short *tile = CArrayGet(
							&m->u.Static.Tiles, idx);
						CArrayPushBack(&movedTiles, tile);
						*tile = MAP_FLOOR;
					}
				}
				// Move the selection to the new position
				b->SelectionStart.x += b->Pos.x - b->DragPos.x;
				b->SelectionStart.y += b->Pos.y - b->DragPos.y;
				// Copy tiles to the new area, for parts of the new area that
				// are valid
				i = 0;
				for (v.y = 0; v.y < b->SelectionSize.y; v.y++)
				{
					for (v.x = 0; v.x < b->SelectionSize.x; v.x++)
					{
						Vec2i vOffset = Vec2iAdd(v, b->SelectionStart);
						if (vOffset.x >= 0 && vOffset.x < m->Size.x &&
							vOffset.y >= 0 && vOffset.y < m->Size.y)
						{
							int idx = vOffset.y * m->Size.x + vOffset.x;
							unsigned short *tileFrom =
								CArrayGet(&movedTiles, i);
							unsigned short *tileTo = CArrayGet(
								&m->u.Static.Tiles, idx);
							*tileTo = *tileFrom;
							result = EDITOR_RESULT_CHANGED;
						}
						i++;
					}
				}
				// Update the selection to fit within map boundaries
				delta = -b->SelectionStart.x;
				if (delta > 0)
				{
					b->SelectionStart.x += delta;
					b->SelectionSize.x -= delta;
				}
				delta = -b->SelectionStart.y;
				if (delta > 0)
				{
					b->SelectionStart.y += delta;
					b->SelectionSize.y -= delta;
				}
				delta = b->SelectionStart.x + b->SelectionSize.x - m->Size.x;
				if (delta > 0)
				{
					b->SelectionSize.x -= delta;
				}
				delta = b->SelectionStart.y + b->SelectionSize.y - m->Size.y;
				if (delta > 0)
				{
					b->SelectionSize.y -= delta;
				}
				// Check if the selection is still valid; if not, invalidate it
				if (b->SelectionSize.x < 0 || b->SelectionSize.y < 0)
				{
					b->SelectionSize = Vec2iZero();
				}

				b->IsMoving = 0;
			}
			else
			{
				// Record the selection size
				b->SelectionStart = Vec2iMin(b->LastPos, b->Pos);
				b->SelectionSize.x = abs(b->LastPos.x - b->Pos.x) + 1;
				b->SelectionSize.y = abs(b->LastPos.y - b->Pos.y) + 1;
				// Disallow 1x1 selection sizes
				if (b->SelectionSize.x <= 1 && b->SelectionSize.y <= 1)
				{
					b->SelectionSize = Vec2iZero();
				}
			}
			break;
		case BRUSHTYPE_SET_EXIT:
			{
				Vec2i exitStart = Vec2iMin(b->LastPos, b->Pos);
				Vec2i exitEnd = Vec2iMax(b->LastPos, b->Pos);
				// Clamp within map boundaries
				exitStart = Vec2iClamp(
					exitStart, Vec2iZero(), Vec2iMinus(m->Size, Vec2iUnit()));
				exitEnd = Vec2iClamp(
					exitEnd, Vec2iZero(), Vec2iMinus(m->Size, Vec2iUnit()));
				// Check that size is big enough
				Vec2i size =
					Vec2iAdd(Vec2iMinus(exitEnd, exitStart), Vec2iUnit());
				if (size.x >= 3 && size.y >= 3)
				{
					// Check that exit area has changed
					if (!Vec2iEqual(exitStart, m->u.Static.Exit.Start) ||
						!Vec2iEqual(exitEnd, m->u.Static.Exit.End))
					{
						m->u.Static.Exit.Start = exitStart;
						m->u.Static.Exit.End = exitEnd;
						result = EDITOR_RESULT_CHANGED_AND_RELOAD;
					}
				}
			}
			break;
		default:
			// do nothing
			break;
		}
	}
	b->IsPainting = 0;
	CArrayClear(&b->HighlightedTiles);
	return result;
}
