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

#include "wall_brush.h"
#include "brush_enums.h"
#include "editor.h"
#include "map.h"

uint32_t WallBrush::full_border_types[16];
uint32_t WallBrush::half_border_types[16];

WallBrush::WallBrush() :
	redirect_to(nullptr)
{
	////
}

WallBrush::~WallBrush()
{
	////
}

bool WallBrush::load(pugi::xml_node node)
{
	pugi::xml_attribute attribute;
	if((attribute = node.attribute("lookid"))) {
		look_id = (uint16_t)attribute.as_int();
	}

	for(pugi::xml_node childNode: node.children()){
		std::string_view childTag = childNode.name();
		if(childTag == "wall") {
			const std::string& typeString = childNode.attribute("type").as_string();
			if(typeString.empty()) {
				g_editor.Warning("Could not read type tag of wall node");
				continue;
			}

			uint32_t alignment;
			if(typeString == "vertical") {
				alignment = WALL_VERTICAL;
			} else if(typeString == "horizontal") {
				alignment = WALL_HORIZONTAL;
			} else if(typeString == "corner") {
				alignment = WALL_NORTHWEST_DIAGONAL;
			} else if(typeString == "pole") {
				alignment = WALL_POLE;
			} else if(typeString == "south end") {
				alignment = WALL_SOUTH_END;
			} else if(typeString == "east end") {
				alignment = WALL_EAST_END;
			} else if(typeString == "north end") {
				alignment = WALL_NORTH_END;
			} else if(typeString == "west end") {
				alignment = WALL_WEST_END;
			} else if(typeString == "south T") {
				alignment = WALL_SOUTH_T;
			} else if(typeString == "east T") {
				alignment = WALL_EAST_T;
			} else if(typeString == "west T") {
				alignment = WALL_WEST_T;
			} else if(typeString == "north T") {
				alignment = WALL_NORTH_T;
			} else if(typeString == "northwest diagonal") {
				alignment = WALL_NORTHWEST_DIAGONAL;
			} else if(typeString == "northeast diagonal") {
				alignment = WALL_NORTHEAST_DIAGONAL;
			} else if(typeString == "southwest diagonal") {
				alignment = WALL_SOUTHWEST_DIAGONAL;
			} else if(typeString == "southeast diagonal") {
				alignment = WALL_SOUTHEAST_DIAGONAL;
			} else if(typeString == "intersection") {
				alignment = WALL_INTERSECTION;
			} else if(typeString == "untouchable") {
				alignment = WALL_UNTOUCHABLE;
			} else {
				g_editor.Warning(wxString() << "Unknown wall alignment '" << typeString << "'");
				continue;
			}

			for(pugi::xml_node itemNode: childNode.children()){
				std::string_view itemTag = itemNode.name();
				if(itemTag == "item") {
					uint16_t id = (uint16_t)itemNode.attribute("id").as_int();
					if(id == 0) {
						g_editor.Warning("Could not read id tag of item node");
						break;
					}

					ItemType *type = GetMutableItemType(id);
					if(!type) {
						g_editor.Warning(wxString() << "There is no itemtype with id " << id);
						return false;
					} else if(type->brush && type->brush != this) {
						g_editor.Warning(wxString() << "Itemtype id " << id << " already has a brush");
						return false;
					}

					type->isWall = true;
					type->brush = this;
					type->border_alignment = ::BorderType(alignment);

					WallType wt;
					wt.id = id;

					wall_items[alignment].total_chance += itemNode.attribute("chance").as_int();
					wt.chance = wall_items[alignment].total_chance;

					wall_items[alignment].items.push_back(wt);
				} else if(itemTag == "door") {
					uint16_t id = (uint16_t)itemNode.attribute("id").as_int();
					if(id == 0) {
						g_editor.Warning("Could not read id tag of door node");
						break;
					}

					std::string nodetype = itemNode.attribute("type").as_string();
					if(nodetype.empty()) {
						g_editor.Warning("Could not read type tag of door node");
						continue;
					}

					bool isOpen;
					pugi::xml_attribute openAttribute = itemNode.attribute("open");
					if(openAttribute) {
						isOpen = openAttribute.as_bool();
					} else {
						isOpen = true;
						if(nodetype != "window" && nodetype != "any window" && nodetype != "hatch window") {
							g_editor.Warning("Could not read open tag of door node");
							break;
						}
					}

					ItemType* type = GetMutableItemType(id);
					if(!type) {
						g_editor.Warning(wxString() << "There is no itemtype with id " << id);
						return false;
					} else if(type->brush && type->brush != this) {
						g_editor.Warning(wxString() << "Itemtype id " << id << " already has a brush");
						return false;
					}

					type->isWall = true;
					type->brush = this;
					type->isBrushDoor = true;
					type->wall_hate_me = itemNode.attribute("hate").as_bool();
					type->isOpen = isOpen;
					type->border_alignment = ::BorderType(alignment);

					DoorType dt;
					bool all_windows = false;
					bool all_doors = false;
					if(nodetype == "normal") {
						dt.type = WALL_DOOR_NORMAL;
					} else if(nodetype == "locked") {
						dt.type = WALL_DOOR_LOCKED;
					} else if(nodetype == "quest") {
						dt.type = WALL_DOOR_QUEST;
					} else if(nodetype == "magic") {
						dt.type = WALL_DOOR_MAGIC;
					} else if(nodetype == "archway") {
						dt.type = WALL_ARCHWAY;
					} else if(nodetype == "window") {
						dt.type = WALL_WINDOW;
					} else if(nodetype == "hatch_window" || nodetype == "hatch window") {
						dt.type = WALL_HATCH_WINDOW;
					} else if(nodetype == "any door") {
						all_doors = true;
					} else if(nodetype == "any window") {
						all_windows = true;
					} else if(nodetype == "any") {
						all_windows = true;
						all_doors = true;
					} else {
						g_editor.Warning(wxString() << "Unknown door type '" << nodetype << "'");
						break;
					}

					dt.id = id;
					if(all_windows) {
						dt.type = WALL_WINDOW;       door_items[alignment].push_back(dt);
						dt.type = WALL_HATCH_WINDOW; door_items[alignment].push_back(dt);
					}

					if(all_doors) {
						dt.type = WALL_ARCHWAY;     door_items[alignment].push_back(dt);
						dt.type = WALL_DOOR_NORMAL; door_items[alignment].push_back(dt);
						dt.type = WALL_DOOR_LOCKED; door_items[alignment].push_back(dt);
						dt.type = WALL_DOOR_QUEST;  door_items[alignment].push_back(dt);
						dt.type = WALL_DOOR_MAGIC;  door_items[alignment].push_back(dt);
					}

					if(!all_doors && !all_windows) {
						door_items[alignment].push_back(dt);
					}
				}
			}
		} else if(childTag == "friend") {
			std::string name = childNode.attribute("name").as_string();
			if(name == "all"){
				//friends.push_back(-1);
			}else if(!name.empty()){
				Brush* brush = g_brushes.getBrush(name);
				if(brush) {
					friends.push_back(brush->getID());
				} else {
					g_editor.Warning(wxString() << "Brush '" << name << "' is not defined.");
					continue;
				}

				if(childNode.attribute("redirect").as_bool()) {
					if(!brush->isWall()) {
						g_editor.Warning(wxString() << "Wall brush redirect link: '" << name << "' is not a wall brush.");
					} else if(!redirect_to) {
						redirect_to = brush->asWall();
					} else {
						g_editor.Warning(wxString() << "Wall brush '" << getName() << "' has more than one redirect link.");
					}
				}
			}
		}
	}
	return true;
}

void WallBrush::undraw(Map *map, Tile* tile)
{
	tile->removeWalls(this);
}

void WallBrush::draw(Map *map, Tile* tile, void* parameter)
{
	ASSERT(tile);
	bool b = (parameter && *reinterpret_cast<bool*>(parameter));
	if(b) {
		// Find a matching wall item on this tile, and shift the id
		Item *wall = tile->getFirstItem(
			[this](const Item *item){
				return item->getWallBrush() == this;
			});

		if(wall != NULL){
			BorderType alignment = wall->getWallAlignment();
			uint16_t newId = 0;
			WallBrush* try_brush = this;
			while(newId == 0 && try_brush != NULL) {
				for(int i = 0; i < 16; i += 1){
					const WallNode &wn = try_brush->wall_items[(alignment + i) % 16];
					if(wn.total_chance > 0){
						int chance = random(1, wn.total_chance);
						for(const WallType &wt: wn.items){
							if(chance <= wt.chance) {
								newId = wt.id;
								break;
							}
						}
					}else if(!wn.items.empty()){
						newId = wn.items.front().id;
					}

					if(newId != 0) {
						break;
					}
				}

				try_brush = try_brush->redirect_to;
				if(try_brush == this) {
					break;
				}
			}

			if(newId != 0) {
				wall->transform(newId);
			}

			return;
		}
	}

	tile->removeWalls(this);

	// Just find a valid item and place it, the bordering algorithm will change it to the proper shape.
	uint16_t newId = 0;
	WallBrush *try_brush = this;
	while(newId == 0 && try_brush != NULL) {
		for(int i = 0; i < 16; ++i) {
			const WallNode &wn = try_brush->wall_items[i];
			if(wn.total_chance > 0){
				int chance = random(1, wn.total_chance);
				for(const WallType &wt: wn.items){
					if(chance <= wt.chance) {
						newId = wt.id;
						break;
					}
				}
			}else if(!wn.items.empty()){
				newId = wn.items.front().id;
			}

			if(newId != 0) {
				break;
			}
		}

		try_brush = try_brush->redirect_to;
		if(try_brush == this) {
			break;
		}
	}

	if(newId != 0){
		tile->addItem(Item::Create(newId));
	}
}

bool hasMatchingWallBrushAtTile(Map *map, WallBrush* wall_brush, int x, int y, int z)
{
	Tile* t = map->getTile(x, y, z);
	if(!t) return false;
	for(Item *item = t->items; item != NULL; item = item->next){
		if(item->isWall()) {
			WallBrush* wb = item->getWallBrush();
			if(wb == wall_brush) {
				return !GetItemType(item->getID()).wall_hate_me;
			} else if(wall_brush->friendOf(wb) || wb->friendOf(wall_brush)) {
				return !GetItemType(item->getID()).wall_hate_me;
			}
		}
	}

	return false;
}

void WallBrush::doWalls(Map *map, Tile* tile)
{
	ASSERT(tile);

	// For quicker reference
	int x = tile->pos.x;
	int y = tile->pos.y;
	int z = tile->pos.z;

	// TODO(fusion): On a reasonable setting there should be a single wall on a
	// tile, but it remains to be confirmed whether that is the case. What I know
	// for a fact is that whatever was here was just unmanageable.
	Item *wall = tile->getFirstItem(
		[](const Item *item){
			WallBrush *wb = item->getWallBrush();
			return wb && !wb->isWallDecoration();
		});

	if(wall != NULL && wall->getWallAlignment() != WALL_UNTOUCHABLE){
		WallBrush *wb = wall->getWallBrush();

		bool neighbours[4] = {};
		neighbours[0] = hasMatchingWallBrushAtTile(map, wb, x,     y - 1, z);
		neighbours[1] = hasMatchingWallBrushAtTile(map, wb, x - 1, y,     z);
		neighbours[2] = hasMatchingWallBrushAtTile(map, wb, x + 1, y,     z);
		neighbours[3] = hasMatchingWallBrushAtTile(map, wb, x,     y + 1, z);

		int tileConfig = 0;
		for(int i = 0; i < 4; i += 1) {
			if(neighbours[i]) {
				tileConfig |= (1 << i);
			}
		}

		// NOTE(fusion): It seems that there are two types of wall brushes. One
		// for regular walls and another for wall-like objects like ant trails,
		// store counters, fishing nets, and vine trails. These "full" border
		// types are used by these wall-like objects but I'm not exactly sure
		// how they compose and some of them seem to yield weird results.

		BorderType borderTypes[] = {
			(BorderType)full_border_types[tileConfig],
			(BorderType)half_border_types[tileConfig],
		};

		for(BorderType bt: borderTypes){
			bool optimal = (wall->getWallAlignment() == bt);

			if(!optimal){
				uint16_t newId = 0;
				WallBrush *curBrush = wb;
				while(newId == 0 && curBrush != NULL){
					WallNode &wn = curBrush->wall_items[int(bt)];
					if(wn.total_chance > 0){
						int chance = random(1, wn.total_chance);
						for(const WallType &wt: wn.items){
							if(chance <= wt.chance) {
								newId = wt.id;
								break;
							}
						}
					}else if(!wn.items.empty()){
						newId = wn.items.front().id;
					}

					curBrush = curBrush->redirect_to;
					if(curBrush == wb){ // prevent infinite loop
						break;
					}
				}

				if(newId != 0){
					wall->transform(newId);
					optimal = true;
				}
			}

			// NOTE(fusion): The original function would check for decoration alignment
			// regardless of whether the wall alignment has changed or not so I'm keeping
			// that behaviour here.
			for(Item *decor = tile->items; decor != NULL; decor = decor->next){
				WallBrush *decorBrush = decor->getWallBrush();
				if(!decorBrush || !decorBrush->isWallDecoration()){
					continue;
				}

				if(decor->getWallAlignment() == bt){
					continue;
				}

				uint16_t newId = 0;
				WallNode &wn = decorBrush->wall_items[int(bt)];
				if(wn.total_chance > 0){
					int chance = random(1, wn.total_chance);
					for(const WallType &wt: wn.items){
						if(chance <= wt.chance){
							newId = wt.id;
							break;
						}
					}
				}else if(!wn.items.empty()){
					newId = wn.items.front().id;
				}

				if(newId != 0){
					decor->transform(newId);
				}
			}

			if(optimal){
				break;
			}
		}
	}
}

bool WallBrush::hasWall(const Item* item)
{
	ASSERT(item->isWall());
	::BorderType bt = item->getWallAlignment();
	WallBrush* test_wall = this;
	while(test_wall != nullptr) {
		for(const WallType &wt: test_wall->wall_items[int(bt)].items){
			if(wt.id == item->getID()) {
				return true;
			}
		}
		for(const DoorType &dt: test_wall->door_items[int(bt)]){
			if(dt.id == item->getID()) {
				return true;
			}
		}

		test_wall = test_wall->redirect_to;
		if(test_wall == this) return false; // Prevent infinite loop
	}
	return false;
}

::DoorType WallBrush::getDoorTypeFromID(uint16_t id)
{
	for(int index = 0; index < 16; ++index) {
		for(const DoorType &dt: door_items[index]){
			if(dt.id == id) {
				return dt.type;
			}
		}
	}
	return WALL_UNDEFINED;
}

//=============================================================================
// Wall Decoration brush

WallDecorationBrush::WallDecorationBrush()
{
	////
}

WallDecorationBrush::~WallDecorationBrush()
{
	////
}

void WallDecorationBrush::draw(Map *map, Tile* tile, void* parameter)
{
	ASSERT(tile);
	tile->removeWalls(this);

	for(Item *wall = tile->items; wall != NULL; wall = wall->next){
		WallBrush *wb = wall->getWallBrush();
		if(wb && wb->isWallDecoration()){
			continue;
		}

		int newId = 0;
		BorderType bt = wall->getWallAlignment();

		if(wall->isBrushDoor()){
			::DoorType type = wb->getDoorTypeFromID(wall->getID());
			for(const WallBrush::DoorType &dt: door_items[bt]){
				if(dt.type != type){
					continue;
				}

				newId = dt.id;
				if(GetItemType(dt.id).isOpen == wall->isOpen()){
					break;
				}
			}
		}else if(wall_items[bt].total_chance > 0){
			int chance = random(1, wall_items[bt].total_chance);
			for(const WallBrush::WallType &wt: wall_items[bt].items){
				if(chance <= wt.chance){
					newId = wt.id;
					break;
				}
			}
		}else if(!wall_items[bt].items.empty()){
			newId = wall_items[bt].items.front().id;
		}

		// NOTE(fusion): Adding an item to the tile shouldn't be a problem while
		// looping the item list, because all item pointers should be stable. At
		// most we're gonna iterate over newly added items and skip over them after
		// checking that they have a wall decoration brush.
		if(newId != 0){
			tile->addItem(Item::Create(newId));
			break; // there shouldn't be more than a single wall on a tile (?)
		}
	}
}

