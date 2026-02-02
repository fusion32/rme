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

#include "brush.h"

#include "tile.h"
#include "creature.h"
#include "house.h"
#include "basemap.h"
#include "spawn.h"
#include "ground_brush.h"
#include "wall_brush.h"
#include "carpet_brush.h"
#include "table_brush.h"

Tile::Tile(TileLocation& loc) :
	location(&loc),
	items(nullptr),
	creature(nullptr),
	spawn(nullptr),
	house_id(0),
	flags(0),
	minimapColor(INVALID_MINIMAP_COLOR)
{
	////
}

Tile::Tile(int x, int y, int z) :
	location(nullptr),
	items(nullptr),
	creature(nullptr),
	spawn(nullptr),
	house_id(0),
	flags(0),
	minimapColor(INVALID_MINIMAP_COLOR)
{
	////
}

Tile::~Tile()
{
	while(Item *it = items){
		items = it->next;
		it->next = NULL;
		delete it;
	}
	delete creature;
	delete spawn;
}

Tile* Tile::deepCopy(BaseMap& map) const
{
	Tile* copy = map.allocator.allocateTile(location);
	copy->flags = flags;
	copy->house_id = house_id;
	if(spawn) copy->spawn = spawn->deepCopy();
	if(creature) copy->creature = creature->deepCopy();
	// Spawncount & exits are not transferred on copy!

	{
		Item **tail = &copy->items;
		for(Item *it = items; it != NULL; it = it->next){
			*tail = it->deepCopy();
			tail = &(*tail)->next;
		}
	}

	return copy;
}

int Tile::countItems(void) const
{
	int count = 0;
	for(Item *it = items; it != NULL; it = it->next){
		count += 1;
	}
	return count;
}

uint32_t Tile::memsize() const
{
	// TODO(fusion): This is missing some stuff...
	return sizeof(Tile) + countItems() * sizeof(Item);
}

int Tile::size() const
{
	int sz = countItems();
	if(creature) ++sz;
	if(spawn) ++sz;
	if(location) {
		if(location->getHouseExits()) ++sz;
		if(location->getSpawnCount()) ++sz;
		if(location->getWaypointCount()) ++ sz;
	}
	return sz;
}

bool Tile::empty() const
{
	int count = 0;
	if(items) count += 1;
	if(creature) count += 1;
	if(spawn) count += 1;
	if(location) {
		if(location->getHouseExits()) count += 1;
		if(location->getSpawnCount()) count += 1;
		if(location->getWaypointCount()) count += 1;
	}
	return count > 0;
}

void Tile::merge(Tile* other)
{
	if(!other) return;

	flags |= other->flags;

	if(other->house_id) {
		house_id = other->house_id;
	}

	if(other->creature) {
		delete creature;
		creature = other->creature;
		other->creature = nullptr;
	}

	if(other->spawn) {
		delete spawn;
		spawn = other->spawn;
		other->spawn = nullptr;
	}

	if(other->creature) {
		delete creature;
		creature = other->creature;
		other->creature = nullptr;
	}

	while(Item *first = other->items){
		other->items = first->next;
		first->next = NULL;
		addItem(first);
	}
}

void Tile::select()
{
	if(size() == 0) return;
	if(spawn) spawn->select();
	if(creature) creature->select();
	for(Item *it = items; it != NULL; it = it->next){
		it->select();
	}
}

void Tile::deselect()
{
	if(spawn) spawn->deselect();
	if(creature) creature->deselect();
	for(Item *it = items; it != NULL; it = it->next){
		it->deselect();
	}
}

void Tile::selectGround()
{
	for(Item *it = items; it != NULL; it = it->next){
		if(!it->getFlag(BANK) && !it->getFlag(CLIP)){
			break;
		}

		it->select();
	}
}

void Tile::deselectGround()
{
	for(Item *it = items; it != NULL; it = it->next){
		if(!it->getFlag(BANK) && !it->getFlag(CLIP)){
			break;
		}

		it->deselect();
	}
}

Item *Tile::popSelectedItems()
{
	Item *result = NULL;
	Item **tail = &result;
	Item **it   = &items;
	while(*it != NULL){
		if((*it)->isSelected()){
			// NOTE(fusion): Insert into the result list.
			*tail = (*it);
			tail = &(*tail)->next;

			// NOTE(fusion): Remove from the tile list. Don't need to advance.
			*it = (*it)->next;
		}else{
			// NOTE(fusion): Advance tile list.
			it = &(*it)->next;
		}
	}
	return result;
}

void Tile::addItem(Item *item)
{
	if(!item) return;

	ASSERT(item->next == NULL);
	int stackPriority = item->getStackPriority();
	bool append = (stackPriority == STACK_PRIORITY_CREATURE
			|| stackPriority == STACK_PRIORITY_LOW);
	// TODO(fusion): Review this?
	bool replace = (stackPriority == STACK_PRIORITY_BANK
			|| stackPriority == STACK_PRIORITY_CLIP
			|| stackPriority == STACK_PRIORITY_BOTTOM
			|| stackPriority == STACK_PRIORITY_TOP);

	Item **it = &items;
	while(*it != NULL){
		if(append  && (*it)->getStackPriority() >  stackPriority) break;
		if(!append && (*it)->getStackPriority() >= stackPriority) break;
		it = &(*it)->next;
	}

	if(replace && *it != NULL && (*it)->getStackPriority() == stackPriority){
		item->next = (*it)->next;
		delete (*it);
		*it = item;
	}else{
		item->next = *it;
		*it = item;
	}

	if(item->isSelected()){
		statflags |= TILESTATE_SELECTED;
	}
}

int Tile::addItems(Item *first)
{
	int count = 0;
	while(Item *item = first){
		first = item->next;
		item->next = NULL;
		addItem(item);
		count += 1;
	}
	return count;
}

int Tile::getIndexOf(Item *item) const
{
	if(item){
		int index = 0;
		for(Item *it = items; it != NULL; it = it->next){
			if(it == item){
				return index;
			}
			index += 1;
		}
	}
	return wxNOT_FOUND;
}

Item* Tile::getItemAt(int index) const
{
	Item *result = items;
	while(result && index > 0){
		result = result->next;
		index -= 1;
	}
	return result;
}

Item* Tile::getTopItem() const
{
	Item *result = NULL;
	for(Item *it = items; it != NULL; it = it->next){
		result = it;
	}
	return result;
}

bool Tile::getFlag(ObjectFlag flag) const
{
	for(Item *it = items; it != NULL; it = it->next){
		if(it->getFlag(flag)){
			return true;
		}
	}
	return false;
}

uint16_t Tile::getGroundSpeed() const noexcept
{
	if(items && items->getFlag(BANK)){
		return items->getAttribute(WAYPOINTS);
	}
	return 0;
}

uint8_t Tile::getMiniMapColor() const
{
	if(minimapColor != INVALID_MINIMAP_COLOR)
		return minimapColor;

	uint8_t result = 0;
	for(Item *it = items; it != NULL; it = it->next){
		uint8_t color = it->getMiniMapColor();
		if(color != 0){
			result = color;
		}
	}
	return result;
}

GroundBrush* Tile::getGroundBrush() const
{
	if(items && items->getFlag(BANK)){
		return items->getGroundBrush();
	}else{
		return nullptr;
	}
}

void Tile::borderize(BaseMap* parent)
{
	GroundBrush::doBorders(parent, this);
}

void Tile::wallize(BaseMap* parent)
{
	WallBrush::doWalls(parent, this);
}

void Tile::tableize(BaseMap* parent)
{
	TableBrush::doTables(parent, this);
}

void Tile::carpetize(BaseMap* parent)
{
	CarpetBrush::doCarpets(parent, this);
}

void Tile::setHouse(House* house)
{
	house_id = (house ? house->id : 0);
}

bool Tile::isHouseExit() const {
	const HouseExitList* exits = location->getHouseExits();
	return exits && !exits->empty();
}

void Tile::addHouseExit(House* house)
{
	if(!house) return;

	HouseExitList* exits = location->createHouseExits();
	exits->push_back(house->id);
}

void Tile::removeHouseExit(House* house)
{
	if(!house) return;

	HouseExitList* exits = location->getHouseExits();
	if(!exits || exits->empty()) return;

	auto it = std::find(exits->begin(), exits->end(), house->id);
	if(it != exits->end())
		exits->erase(it);
}

bool Tile::hasHouseExit(uint32_t houseId) const
{
	const HouseExitList* exits = getHouseExits();
	if(!exits || exits->empty())
		return false;

	auto it = std::find(exits->begin(), exits->end(), houseId);
	return it != exits->end();
}

