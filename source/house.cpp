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

#include "house.h"
#include "tile.h"
#include "map.h"

// House
//==============================================================================
void House::clean(Map *map)
{
	for(Position pos: tiles){
		if(Tile *tile = map->getTile(pos)){
			tile->houseId = 0;
		}
	}

#if TODO
	Tile* tile = map->getTile(exit);
	if(tile)
		tile->removeHouseExit(this);
#endif
}

size_t House::size(Map *map) const
{
	size_t count = 0;
	for(Position pos: tiles){
		Tile* tile = map->getTile(pos);
		if(tile && !tile->getFlag(UNPASS)){
			count += 1;
		}
	}
	return count;
}

void House::addTile(Map *map, Position pos)
{
	if(Tile *tile = map->getTile(pos)){
		tile->houseId = houseId;
		tiles.push_back(pos);
	}
}

void House::removeTile(Map *map, Position pos)
{
	if(Tile *tile = map->getTile(pos)){
		for(auto it = tiles.begin(); it != tiles.end(); ++it){
			if(*it == tile->pos){
				ASSERT(tile->houseId == houseId);
				tiles.erase(it);
				tile->houseId = 0;
				break;
			}
		}
	}
}

std::string House::getDescription()
{
	std::ostringstream os;
	os << name;
	os << " (ID:" << houseId << "; Rent: " << rent << ")";
	return os.str();
}

void House::setExit(Map *map, Position pos)
{
#if TODO
	// This might fail when the user decides to put an exit at 0,0,0, let's just hope noone does (Noone SHOULD, so there is no problem? Hm?)
	if(pos == exit || !pos.isValid())
		return;

	if(exit.isValid()) {
		Tile* oldexit = targetmap->getTile(exit);
		if(oldexit)
			oldexit->removeHouseExit(this);
	}

	Tile* newexit = targetmap->getTile(pos);
	if(!newexit) {
		newexit = targetmap->allocator(targetmap->createTileL(pos));
		targetmap->setTile(pos, newexit);
	}

	newexit->addHouseExit(this);
	exit = pos;
#endif
}

// Houses
//==============================================================================
Houses::~Houses()
{
	for(House *house: houses){
		delete house;
	}
	houses.clear();
}

uint16_t Houses::getEmptyID(void)
{
	maxHouseId += 1;
	return maxHouseId;
}

void Houses::addHouse(Map *map, House *house)
{
	uint16_t houseId = house->houseId;

	if(houseId > maxHouseId){
		maxHouseId = house->houseId;
	}

	if(houseId >= houses.size()){
		houses.resize(houseId + 1);
	}

	ASSERT(houses[houseId] == NULL);
	houses[houseId] = house;
}

void Houses::removeHouse(Map *map, House *house)
{
	uint16_t houseId = house->houseId;
	if(houseId < houses.size() && houses[houseId] == house){
		house->clean(map);
		houses[houseId] = NULL;
		delete house;
	}
}

House* Houses::getHouse(uint16_t houseId)
{
	return houseId < houses.size() ? houses[houseId] : NULL;
}

const House *Houses::getHouse(uint16_t houseId) const
{
	return houseId < houses.size() ? houses[houseId] : NULL;
}
