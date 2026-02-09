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
#include "carpet_brush.h"
#include "creature_brush.h"
#include "doodad_brush.h"
#include "ground_brush.h"
#include "house_brush.h"
#include "house_exit_brush.h"
#include "raw_brush.h"
#include "table_brush.h"
#include "wall_brush.h"
#include "waypoint_brush.h"

#include "settings.h"

#include "sprites.h"

#include "item.h"
#include "creature.h"
#include "map.h"
#include "editor.h"

Brushes g_brushes;

Brushes::Brushes()
{
	////
}

Brushes::~Brushes()
{
	////
}

void Brushes::clear()
{
	for(auto brushEntry : brushes) {
		delete brushEntry.second;
	}
	brushes.clear();

	for(auto borderEntry : borders) {
		delete borderEntry.second;
	}
	borders.clear();
}

void Brushes::init()
{
	addBrush(g_editor.optional_brush = newd OptionalBorderBrush());
	addBrush(g_editor.eraser = newd EraserBrush());
	addBrush(g_editor.normal_door_brush = newd DoorBrush(WALL_DOOR_NORMAL));
	addBrush(g_editor.locked_door_brush = newd DoorBrush(WALL_DOOR_LOCKED));
	addBrush(g_editor.magic_door_brush = newd DoorBrush(WALL_DOOR_MAGIC));
	addBrush(g_editor.quest_door_brush = newd DoorBrush(WALL_DOOR_QUEST));
	addBrush(g_editor.hatch_door_brush = newd DoorBrush(WALL_HATCH_WINDOW));
	addBrush(g_editor.window_door_brush = newd DoorBrush(WALL_WINDOW));
	addBrush(g_editor.house_brush = newd HouseBrush());
	addBrush(g_editor.house_exit_brush = newd HouseExitBrush());
	addBrush(g_editor.waypoint_brush = newd WaypointBrush());

	addBrush(g_editor.refresh_brush = newd FlagBrush(TILE_FLAG_REFRESH));
	addBrush(g_editor.nolog_brush = newd FlagBrush(TILE_FLAG_NOLOGOUT));
	addBrush(g_editor.pz_brush = newd FlagBrush(TILE_FLAG_PROTECTIONZONE));

	GroundBrush::init();
	WallBrush::init();
	TableBrush::init();
	CarpetBrush::init();
}

bool Brushes::unserializeBrush(pugi::xml_node node, wxArrayString& warnings)
{
	pugi::xml_attribute attribute;
	if(!(attribute = node.attribute("name"))) {
		warnings.push_back("Brush node without name.");
		return false;
	}

	const std::string& brushName = attribute.as_string();
	if(brushName == "all" || brushName == "none") {
		warnings.push_back(wxString("Using reserved brushname \"") << wxstr(brushName) << "\".");
		return false;
	}

	Brush* brush = getBrush(brushName);
	if(!brush) {
		if(!(attribute = node.attribute("type"))) {
			warnings.push_back("Couldn't read brush type");
			return false;
		}

		const std::string brushType = attribute.as_string();
		if(brushType == "border" || brushType == "ground") {
			brush = newd GroundBrush();
		} else if(brushType == "wall") {
			brush = newd WallBrush();
		} else if(brushType == "wall decoration") {
			brush = newd WallDecorationBrush();
		} else if(brushType == "carpet") {
			brush = newd CarpetBrush();
		} else if(brushType == "table") {
			brush = newd TableBrush();
		} else if(brushType == "doodad") {
			brush = newd DoodadBrush();
		} else {
			warnings.push_back(wxString("Unknown brush type ") << wxstr(brushType));
			return false;
		}

		ASSERT(brush);
		brush->setName(brushName);
	}

	if(!node.first_child()) {
		brushes.insert(std::make_pair(brush->getName(), brush));
		return true;
	}

	wxArrayString subWarnings;
	brush->load(node, subWarnings);

	if(!subWarnings.empty()) {
		warnings.push_back(wxString("Errors while loading brush \"") << brush->getName() << "\"");
		warnings.insert(warnings.end(), subWarnings.begin(), subWarnings.end());
	}

	if(brush->getName() == "all" || brush->getName() == "none") {
		warnings.push_back(wxString("Using reserved brushname '") << brush->getName() << "'.");
		delete brush;
		return false;
	}

	Brush* otherBrush = getBrush(brush->getName());
	if(otherBrush) {
		if(otherBrush != brush) {
			warnings.push_back(wxString("Duplicate brush name ")
					<< brush->getName() << ". Undefined behaviour may ensue.");
		} else {
			// Don't insert
			return true;
		}
	}

	brushes.insert(std::make_pair(brush->getName(), brush));
	return true;
}

bool Brushes::unserializeBorder(pugi::xml_node node, wxArrayString& warnings)
{
	pugi::xml_attribute attribute = node.attribute("id");
	if(!attribute) {
		warnings.push_back("Couldn't read border id node");
		return false;
	}

	uint32_t id = attribute.as_uint();
	if(borders[id]) {
		warnings.push_back("Border ID " + std::to_string(id) + " already exists");
		return false;
	}

	AutoBorder* border = newd AutoBorder(id);
	border->load(node, warnings);
	borders[id] = border;
	return true;
}

void Brushes::addBrush(Brush *brush)
{
	brushes.insert(std::make_pair(brush->getName(), brush));
}

Brush* Brushes::getBrush(const std::string& name) const
{
	auto it = brushes.find(name);
	if(it != brushes.end()) {
		return it->second;
	}
	return nullptr;
}

// Brush
uint32_t Brush::id_counter = 0;
Brush::Brush() :
	id(++id_counter), visible(false)
{
	////
}

Brush::~Brush()
{
	////
}

// TerrainBrush
TerrainBrush::TerrainBrush() :
	look_id(0), hate_friends(false)
{
	////
}

TerrainBrush::~TerrainBrush()
{
	////
}

bool TerrainBrush::friendOf(TerrainBrush* other)
{
	uint32_t borderID = other->getID();
	for(uint32_t friendId : friends) {
		if(friendId == borderID) {
			//printf("%s is friend of %s\n", getName().c_str(), other->getName().c_str());
			return !hate_friends;
		} else if(friendId == 0xFFFFFFFF) {
			//printf("%s is generic friend of %s\n", getName().c_str(), other->getName().c_str());
			return !hate_friends;
		}
	}
	//printf("%s is enemy of %s\n", getName().c_str(), other->getName().c_str());
	return hate_friends;
}

//=============================================================================
// Flag brush
// draws pz etc.

FlagBrush::FlagBrush(uint32_t _flag) : flag(_flag)
{
	////
}

FlagBrush::~FlagBrush()
{
	////
}

std::string FlagBrush::getName() const
{
	switch(flag) {
		case TILE_FLAG_REFRESH:        return "Refresh brush (0x04)";
		case TILE_FLAG_NOLOGOUT:       return "No logout brush (0x04)";
		case TILE_FLAG_PROTECTIONZONE: return "Protection zone brush (0x04)";
	}
	return "Unknown flag brush";
}

int FlagBrush::getLookID() const
{
	switch(flag) {
		//case TILE_FLAG_REFRESH:        return EDITOR_SPRITE_REFRESH_TOOL;
		case TILE_FLAG_NOLOGOUT:       return EDITOR_SPRITE_NOLOG_TOOL;
		case TILE_FLAG_PROTECTIONZONE: return EDITOR_SPRITE_PZ_TOOL;
	}
	return 0;
}

bool FlagBrush::canDraw(Map *map, const Position& position) const
{
	Tile* tile = map->getTile(position);
	return tile && tile->getFlag(BANK);
}

void FlagBrush::undraw(Map *map, Tile* tile)
{
	tile->clearTileFlag(flag);
}

void FlagBrush::draw(Map *map, Tile* tile, void* parameter)
{
	if(tile->getFlag(BANK)) {
		tile->setTileFlag(flag);
	}
}

//=============================================================================
// Door brush

DoorBrush::DoorBrush(DoorType _doortype) : doortype(_doortype)
{
	////
}

DoorBrush::~DoorBrush()
{
	////
}

std::string DoorBrush::getName() const
{
	switch(doortype) {
		case WALL_DOOR_NORMAL: return "Normal door brush";
		case WALL_DOOR_LOCKED: return "Locked door brush";
		case WALL_DOOR_MAGIC: return "Magic door brush";
		case WALL_DOOR_QUEST: return "Quest door brush";
		case WALL_WINDOW: return "Window brush";
		case WALL_HATCH_WINDOW: return "Hatch window brush";
		default: return "Unknown door brush";
	}
}

int DoorBrush::getLookID() const
{
	switch(doortype) {
		case WALL_DOOR_NORMAL: return EDITOR_SPRITE_DOOR_NORMAL;
		case WALL_DOOR_LOCKED: return EDITOR_SPRITE_DOOR_LOCKED;
		case WALL_DOOR_MAGIC: return EDITOR_SPRITE_DOOR_MAGIC;
		case WALL_DOOR_QUEST: return EDITOR_SPRITE_DOOR_QUEST;
		case WALL_WINDOW: return EDITOR_SPRITE_WINDOW_NORMAL;
		case WALL_HATCH_WINDOW: return EDITOR_SPRITE_WINDOW_HATCH;
		default: return EDITOR_SPRITE_DOOR_NORMAL;
	}
}

void DoorBrush::switchDoor(Item* item)
{
	ASSERT(item);
	ASSERT(item->isBrushDoor());

	WallBrush* wb = item->getWallBrush();
	if(!wb) return;

	bool new_open = !item->isOpen();
	BorderType wall_alignment = item->getWallAlignment();
	DoorType doortype = WALL_UNDEFINED;

	for(std::vector<WallBrush::DoorType>::iterator iter = wb->door_items[wall_alignment].begin(); iter != wb->door_items[wall_alignment].end(); ++iter) {
		WallBrush::DoorType& dt = *iter;
		if(dt.id == item->getID()) {
			doortype = dt.type;
			break;
		}
	}
	if(doortype == WALL_UNDEFINED) return;

	for(std::vector<WallBrush::DoorType>::iterator iter = wb->door_items[wall_alignment].begin(); iter != wb->door_items[wall_alignment].end(); ++iter) {
		WallBrush::DoorType& dt = *iter;
		if(dt.type == doortype) {
			ASSERT(dt.id);
			const ItemType &type = GetItemType(dt.id);
			ASSERT(type.typeId != 0);

			if(type.isOpen == new_open) {
				item->transform(dt.id);
				return;
			}
		}
	}
}

bool DoorBrush::canDraw(Map *map, const Position& position) const
{
	Tile* tile = map->getTile(position);
	if(!tile) {
		return false;
	}

	Item* item = tile->getWall();
	if(!item) {
		return false;
	}

	WallBrush* wb = item->getWallBrush();
	if(!wb) {
		return false;
	}

	BorderType wall_alignment = item->getWallAlignment();

	uint16_t discarded_id = 0; // The id of a discarded match
	bool close_match = false;

	bool open = false;
	if(item->isBrushDoor()) {
		open = item->isOpen();
	}

	WallBrush* test_brush = wb;
	do {
		for(std::vector<WallBrush::DoorType>::iterator iter = test_brush->door_items[wall_alignment].begin();
				iter != test_brush->door_items[wall_alignment].end();
				++iter) {
			WallBrush::DoorType& dt = *iter;
			if(dt.type == doortype) {
				ASSERT(dt.id);
				const ItemType &type = GetItemType(dt.id);
				ASSERT(type.typeId != 0);

				if(type.isOpen == open) {
					return true;
				} else if(!close_match) {
					discarded_id = dt.id;
					close_match = true;
				}
				if(!close_match && discarded_id == 0) {
					discarded_id = dt.id;
				}
			}
		}
		test_brush = test_brush->redirect_to;
	} while(test_brush != wb && test_brush != nullptr);
	// If we've found no perfect match, use a close-to perfect
	if(discarded_id) {
		return true;
	}
	return false;
}

void DoorBrush::undraw(Map *map, Tile* tile)
{
	for(Item *item = tile->items; item != NULL; item = item->next){
		if(item->isBrushDoor()) {
			item->getWallBrush()->draw(map, tile, nullptr);
			if(g_settings.getInteger(Config::USE_AUTOMAGIC)) {
				tile->wallize(map);
			}
			return;
		}
	}
}

void DoorBrush::draw(Map *map, Tile* tile, void* parameter)
{
	// TODO(fusion): What the actual f*** is going on here?
	WallBrush *wb = NULL;
	Item *item = tile->items;
	while(item != NULL){
		if(item->isWall() && (wb = item->getWallBrush())){
			break;
		}
		item = item->next;
	}

	if(!wb){
		return;
	}

	bool open = parameter && *reinterpret_cast<bool*>(parameter);
	if(item->isBrushDoor()) {
		open = item->isOpen();
	}

	BorderType wall_alignment = item->getWallAlignment();
	WallBrush* test_brush = wb;
	uint16_t discarded_id = 0;
	bool close_match = false;
	bool perfect_match = false;
	do {
		for(const WallBrush::DoorType &dt: test_brush->door_items[wall_alignment]){
			if(dt.type == doortype) {
				ASSERT(dt.id);
				const ItemType &type = GetItemType(dt.id);
				ASSERT(type.typeId != 0);

				if(type.isOpen == open) {
					item->transform(dt.id);
					perfect_match = true;
					break;
				} else if(!close_match) {
					discarded_id = dt.id;
					close_match = true;
				}
				if(!close_match && discarded_id == 0) {
					discarded_id = dt.id;
				}
			}
		}
		test_brush = test_brush->redirect_to;
		if(perfect_match) {
			break;
		}
	} while(test_brush != wb && test_brush != nullptr);

	// If we've found no perfect match, use a close-to perfect
	if(!perfect_match && discarded_id) {
		item->transform(discarded_id);
	}

	// TODO(fusion): Similar to HouseBrush::draw.

	// We need to consider decorations!
	while(item->isWall() && (wb = item->getWallBrush()) && wb->isWallDecoration()) {
		for(const WallBrush::DoorType &dt: wb->door_items[wall_alignment]){
			if(dt.type == doortype) {
				ASSERT(dt.id);
				const ItemType &type = GetItemType(dt.id);
				ASSERT(type.typeId != 0);

				if(type.isOpen == open) {
					item->transform(dt.id);
					perfect_match = true;
					break;
				} else if(!close_match) {
					discarded_id = dt.id;
					close_match = true;
				}
				if(!close_match && discarded_id == 0) {
					discarded_id = dt.id;
				}
			}
		}
		// If we've found no perfect match, use a close-to perfect
		if(!perfect_match && discarded_id) {
			item->transform(discarded_id);
		}
	}
}

//=============================================================================
// Gravel brush

OptionalBorderBrush::OptionalBorderBrush()
{
	////
}

OptionalBorderBrush::~OptionalBorderBrush()
{
	////
}

std::string OptionalBorderBrush::getName() const
{
	return "Optional Border Tool";
}

int OptionalBorderBrush::getLookID() const
{
	return EDITOR_SPRITE_OPTIONAL_BORDER_TOOL;
}

bool OptionalBorderBrush::canDraw(Map *map, const Position& position) const
{
	Tile* tile = map->getTile(position);

	// You can't do gravel on a mountain tile
	if(tile) {
		if(GroundBrush* bb = tile->getGroundBrush()) {
			if(bb->hasOptionalBorder()) {
				return false;
			}
		}
	}

	uint32_t x = position.x;
	uint32_t y = position.y;
	uint32_t z = position.z;

	tile = map->getTile(x - 1, y - 1, z);
	if(tile) if(GroundBrush* bb = tile->getGroundBrush()) if(bb->hasOptionalBorder()) return true;
	tile = map->getTile(x    , y - 1, z);
	if(tile) if(GroundBrush* bb = tile->getGroundBrush()) if(bb->hasOptionalBorder()) return true;
	tile = map->getTile(x + 1, y - 1, z);
	if(tile) if(GroundBrush* bb = tile->getGroundBrush()) if(bb->hasOptionalBorder()) return true;
	tile = map->getTile(x - 1, y   , z);
	if(tile) if(GroundBrush* bb = tile->getGroundBrush()) if(bb->hasOptionalBorder()) return true;
	tile = map->getTile(x + 1, y   , z);
	if(tile) if(GroundBrush* bb = tile->getGroundBrush()) if(bb->hasOptionalBorder()) return true;
	tile = map->getTile(x - 1, y + 1, z);
	if(tile) if(GroundBrush* bb = tile->getGroundBrush()) if(bb->hasOptionalBorder()) return true;
	tile = map->getTile(x    , y + 1, z);
	if(tile) if(GroundBrush* bb = tile->getGroundBrush()) if(bb->hasOptionalBorder()) return true;
	tile = map->getTile(x + 1, y + 1, z);
	if(tile) if(GroundBrush* bb = tile->getGroundBrush()) if(bb->hasOptionalBorder()) return true;

	return false;
}

void OptionalBorderBrush::undraw(Map *map, Tile* tile)
{
	tile->setOptionalBorder(false); // The bordering algorithm will handle this automagicaly
}

void OptionalBorderBrush::draw(Map *map, Tile* tile, void* parameter)
{
	tile->setOptionalBorder(true); // The bordering algorithm will handle this automagicaly
}

