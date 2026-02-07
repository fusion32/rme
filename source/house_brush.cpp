//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "house_brush.h"
#include "house.h"
#include "map.h"
#include "settings.h"

//=============================================================================
// House Brush

HouseBrush::HouseBrush() : Brush(), houseId(0)
{
	// no-op
}

HouseBrush::~HouseBrush()
{
	// no-op
}

void HouseBrush::undraw(Map *map, Tile* tile)
{
	// TODO(fusion): Same as HouseBrush::draw.
	if(tile->houseId != 0) {
		tile->clearTileFlag(TILE_FLAG_PROTECTIONZONE);
	}
	tile->houseId = 0;
}

void HouseBrush::draw(Map *map, Tile* tile, void* parameter)
{
	// TODO(fusion): There are no door ids but rather certain doors that have
	// the NAMEDOOR flag and which can have a TEXTSTRING attribute with the
	// permission list. We might want to have support to convert to and from
	// NAMEDOOR versions of the same door (if that's even a thing) but this
	// is a fundamentally different approach.
	ASSERT(houseId != 0);
	tile->houseId = houseId;
	tile->setTileFlag(TILE_FLAG_PROTECTIONZONE);
	if(g_settings.getInteger(Config::HOUSE_BRUSH_REMOVE_ITEMS)) {
		tile->removeItems([](const Item *item){ return !item->getFlag(UNMOVE); });
	}
}

