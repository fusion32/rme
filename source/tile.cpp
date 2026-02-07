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
#include "ground_brush.h"
#include "wall_brush.h"
#include "carpet_brush.h"
#include "table_brush.h"

void Tile::clear(void)
{
	while(Item *it = items){
		items = it->next;
		it->next = NULL;
		delete it;
	}

	delete creature;
}

void Tile::swap(Tile &other){
	std::swap(pos, other.pos);
	std::swap(flags, other.flags);
	std::swap(houseId, other.houseId);
	std::swap(minimapColor, other.minimapColor);
	std::swap(creature, other.creature);
	std::swap(items, other.items);
}

void Tile::deepCopy(const Tile &other)
{
	clear();

	pos = other.pos;
	flags = other.flags;
	houseId = other.houseId;
	minimapColor = other.minimapColor;

	if(other.creature){
		creature = other.creature->deepCopy();
	}

	if(other.items){
		Item **tail = &items;
		for(const Item *it = other.items; it != NULL; it = it->next){
			*tail = it->deepCopy();
			tail = &(*tail)->next;
		}
	}
}

Tile *Tile::deepCopy(void) const
{
	Tile *copy = newd Tile();
	copy->deepCopy(*this);
	return copy;
}

void Tile::mergeCopy(const Tile &other)
{
	flags |= other.flags;

	if(other.houseId) {
		houseId = other.houseId;
	}

	if(other.creature) {
		delete creature;
		creature = other.creature->deepCopy();
	}

	for(const Item *item = other.items; item != NULL; item = item->next){
		addItem(item->deepCopy());
	}
}

void Tile::merge(Tile &&other)
{
	flags |= other.flags;

	if(other.houseId) {
		houseId = other.houseId;
	}

	if(other.creature) {
		delete creature;
		creature = other.creature;
		other.creature = NULL;
	}

	while(Item *first = other.items){
		other.items = first->next;
		first->next = NULL;
		addItem(first);
	}
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
	int result = countItems();
	if(creature){
		result += 1;
	}
	return result;
}

bool Tile::empty() const
{
	return items == NULL && creature == NULL;
}

void Tile::select()
{
	if(creature) creature->select();
	for(Item *it = items; it != NULL; it = it->next){
		it->select();
	}
}

void Tile::deselect()
{
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

Item *Tile::getItemAt(int index) const
{
	Item *result = items;
	while(result && index > 0){
		result = result->next;
		index -= 1;
	}
	return result;
}

Item *Tile::getTopItem(int *outIndex /*= NULL*/) const
{
	int index = -1;
	Item *result = NULL;
	for(Item *it = items; it != NULL; it = it->next){
		result = it;
		index += 1;
	}
	if(outIndex){
		*outIndex = index;
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

uint16_t Tile::getGroundSpeed(void) const noexcept
{
	if(items && items->getFlag(BANK)){
		return items->getAttribute(WAYPOINTS);
	}
	return 0;
}

uint8_t Tile::getMiniMapColor(void) const
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

void Tile::borderize(Map *parent)
{
	GroundBrush::doBorders(parent, this);
}

void Tile::wallize(Map *parent)
{
	WallBrush::doWalls(parent, this);
}

void Tile::tableize(Map *parent)
{
	TableBrush::doTables(parent, this);
}

void Tile::carpetize(Map *parent)
{
	CarpetBrush::doCarpets(parent, this);
}

