//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "house_exit_brush.h"
#include "house.h"
#include "map.h"

//=============================================================================
// House Exit Brush

HouseExitBrush::HouseExitBrush() : Brush(), houseId(0)
{
	// no-op
}

HouseExitBrush::~HouseExitBrush()
{
	// no-op
}

bool HouseExitBrush::canDraw(Map *map, const Position& position) const
{
	Tile* tile = map->getTile(position);
	if(!tile || !tile->getFlag(BANK)) {
		return false;
	}
	if(tile->houseId != 0 || tile->getFlag(UNPASS)) {
		return false;
	}
	return true;
}

void HouseExitBrush::undraw(Map *map, Tile* tile)
{
	// Never called
	ASSERT(false);
}

void HouseExitBrush::draw(Map *map, Tile* tile, void* parameter)
{
	// Never called
	ASSERT(false);
}
