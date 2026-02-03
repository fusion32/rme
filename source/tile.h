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

#ifndef RME_TILE_H
#define RME_TILE_H

#include "position.h"
#include "item.h"
#include "map_region.h"
#include "wall_brush.h"
#include <unordered_set>

enum {
	TILE_FLAG_REFRESH        = 0x01,
	TILE_FLAG_NOLOGOUT       = 0x02,
	TILE_FLAG_PROTECTIONZONE = 0x04,
};

enum {
	INVALID_MINIMAP_COLOR = 0xFF
};

class Tile
{
public:
	TileLocation *location;
	Item *items;
	Creature *creature;
	Spawn *spawn;
	uint32_t house_id; // House id for this tile (pointer not safe)
	uint8_t flags;
	uint8_t minimapColor;

	// ALWAYS use this constructor if the Tile is EVER going to be placed on a map
	Tile(TileLocation& location);
	// Use this when the tile is only used internally by the editor (like in certain brushes)
	Tile(int x, int y, int z);

	~Tile();

	// non-copyable, non-comparable (?)
	Tile(const Tile &tile) = delete;
	Tile &operator=(const Tile &other) = delete;
	Tile &operator==(const Tile &other) = delete;

	Tile* deepCopy(BaseMap& map) const;
	uint32_t memsize(void) const;
	int countItems(void) const;
	int size(void) const;
	bool empty(void) const;

	void merge(Tile* other);
	void select();
	void deselect();
	void selectGround();
	void deselectGround();
	Item *popSelectedItems();

	void addItem(Item *item);
	int addItems(Item *first);
	int getIndexOf(Item *item) const;
	Item* getItemAt(int index) const;
	Item* getTopItem() const;
	bool getFlag(ObjectFlag flag) const;
	uint16_t getGroundSpeed() const noexcept;
	uint8_t getMiniMapColor() const;

	GroundBrush* getGroundBrush() const;
	void borderize(BaseMap* parent);
	void wallize(BaseMap* parent);
	void tableize(BaseMap* parent);
	void carpetize(BaseMap* parent);

	template<typename Pred>
	Item *getFirstItem(Pred &&predicate){
		for(Item *it = items; it != NULL; it = it->next){
			if(predicate(it)){
				return it;
			}
		}
		return NULL;
	}

	template<typename Pred>
	Item *getLastItem(Pred &&predicate){
		Item *result = NULL;
		for(Item *it = items; it != NULL; it = it->next){
			if(predicate(it)){
				result = it;
			}
		}
		return result;
	}

	Item *getFirstItem(ObjectFlag flag){
		return getFirstItem([flag](const Item *item) { return item->getFlag(flag); });
	}

	Item *getLastItem(ObjectFlag flag){
		return getLastItem([flag](const Item *item) { return item->getFlag(flag); });
	}

	Item *getWall(void){
		return getFirstItem([](const Item *item) { return item->isWall(); });
	}

	Item *getTable(void){
		return getFirstItem([](const Item *item) { return item->isTable(); });
	}

	Item *getCarpet(void){
		return getFirstItem([](const Item *item) { return item->isCarpet(); });
	}

	Item *getFirstSelectedItem(void){
		return getFirstItem([](const Item *item) { return item->isSelected(); });
	}

	Item *getLastSelectedItem(void){
		return getLastItem([](const Item *item) { return item->isSelected(); });
	}

	bool isSelected(void){
		return getFirstSelectedItem() != NULL;
	}

	// TODO(fusion): Not exactly sure when we want to NOT delete removed items here.
	template<typename Pred>
	void removeItems(Pred &&predicate, bool del = true){
		Item **it = &items;
		while(*it != NULL){
			if(predicate(*it)){
				Item *next = (*it)->next;
				(*it)->next = NULL;
				if(del){
					delete *it;
				}
				*it = next;
			}else{
				it = &(*it)->next;
			}
		}
	}

	void removeBorders(void){
		removeItems([](const Item *item) { return item->getFlag(CLIP); }, true);
	}

	void removeWalls(bool del = true){
		removeItems([](const Item *item) { return item->isWall(); }, del);
	}

	void removeWalls(WallBrush *brush){
		if(brush){
			removeItems([brush](const Item *item){ return item->isWall() && brush->hasWall(item); });
		}
	}

#if 0
	// TODO(fusion): We may want to do this?
	template<typename F>
	void forEachItem(F &&f){
		Item *item = items;
		while(item != NULL){
			Item *next = item->next;
			f(item);
			item = next;
		}
	}
#endif

	// The location of the tile
	// Stores state that remains between the tile being moved (like house exits)
	void setLocation(TileLocation* where) { location = where; }
	TileLocation* getLocation() { return location; }
	const TileLocation* getLocation() const { return location; }

	// Position of the tile
	const Position& getPosition() const noexcept { return location->getPosition(); }
	int getX() const noexcept { return location->getX(); }
	int getY() const noexcept { return location->getY(); }
	int getZ() const noexcept { return location->getZ(); }

	bool getTileFlag(uint8_t flag) const { return (flags & flag) != 0; }
	void setTileFlag(uint8_t flag) { flags |= flag; }
	void clearTileFlag(uint8_t flag) { flags &= ~flag; }

	bool isHouseTile() const noexcept { return house_id != 0; }
	uint32_t getHouseID() const noexcept  { return house_id; }
	HouseExitList* getHouseExits() { return location->getHouseExits(); }
	const HouseExitList* getHouseExits() const { return location->getHouseExits(); }
	void setHouse(House *house);
	bool isHouseExit() const;
	void addHouseExit(House* house);
	void removeHouseExit(House* house);
	bool hasHouseExit(uint32_t houseId) const;

	// TODO(fusion): This could be a non-persistent tile flag but I'm not sure it
	// is used for anything particularly useful.
	bool isModified(void) const { return false; }
	void modify(void) {}
	void unmodify(void) {}

	// TODO(fusion): Same thing.
	bool hasOptionalBorder(void) const { return false; }
	void setOptionalBorder(bool dummy) {}
};

typedef std::unordered_set<Tile*> TileSet;

#endif
