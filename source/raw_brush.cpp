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

#include "raw_brush.h"
#include "settings.h"
#include "tile.h"

//=============================================================================
// RAW brush

RAWBrush::RAWBrush(uint16_t itemid) :
	Brush()
{
	itemtype = GetMutableItemType(itemid);
}

RAWBrush::~RAWBrush()
{
	////
}

int RAWBrush::getLookID() const
{
	return (itemtype ? itemtype->typeId : 0);
}

uint16_t RAWBrush::getItemID() const
{
	return (itemtype ? itemtype->typeId : 0);
}

std::string RAWBrush::getName() const
{
	if(!itemtype)
		return "RAWBrush";

	if(itemtype->getFlag(HOOKSOUTH))
		return i2s(itemtype->typeId) + " - " + itemtype->name + " (Hook South)";
	else if(itemtype->getFlag(HOOKEAST))
		return i2s(itemtype->typeId) + " - " + itemtype->name + " (Hook East)";

	return i2s(itemtype->typeId) + " - " + itemtype->name;
}

void RAWBrush::undraw(Map *map, Tile* tile)
{
	ASSERT(itemtype); // ?
	tile->removeItems(
		[typeId = itemtype->typeId](const Item *item){
			return item->getID() == typeId;
		});
}

void RAWBrush::draw(Map *map, Tile* tile, void* parameter)
{
	if(!itemtype) return;

	// TODO(fusion): I think this is now already handled by Tile::addItem? It is
	// also a good idea to keep the tile ordering and constraints similar the the
	// game servers'.
	bool b = parameter && *reinterpret_cast<bool*>(parameter);
	if((g_settings.getInteger(Config::RAW_LIKE_SIMONE) && !b) && itemtype->getFlag(BOTTOM)){
		tile->removeItems([](const Item *item){ return item->getFlag(BOTTOM); });
	}

	tile->addItem(Item::Create(itemtype->typeId));
}
