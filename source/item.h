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

#ifndef RME_ITEM_H_
#define RME_ITEM_H_

#include "items.h"

enum LiquidType {
	LIQUID_NONE = 0,
	LIQUID_WATER = 1,
	LIQUID_BLOOD = 2,
	LIQUID_BEER = 3,
	LIQUID_SLIME = 4,
	LIQUID_LEMONADE = 5,
	LIQUID_MILK = 6,
	LIQUID_MANAFLUID = 7,
	LIQUID_INK = 8,
	LIQUID_WATER2 = 9,
	LIQUID_LIFEFLUID = 10,
	LIQUID_OIL = 11,
	LIQUID_SLIME2 = 12,
	LIQUID_URINE = 13,
	LIQUID_COCONUT_MILK = 14,
	LIQUID_WINE = 15,
	LIQUID_MUD = 19,
	LIQUID_FRUIT_JUICE = 21,
	LIQUID_LAVA = 26,
	LIQUID_RUM = 27,
	LIQUID_SWAMP = 28,
	LIQUID_TEA = 35,
	LIQUID_MEAD = 43,

	LIQUID_FIRST = LIQUID_WATER,
	LIQUID_LAST = LIQUID_MEAD
};

IMPLEMENT_INCREMENT_OP(LiquidType)

class Creature;
class Border;
class Tile;
class Container;
class Depot;
class Teleport;
class Door;

struct SpriteLight;

class Item {
public:
	//Factory member to create item of right type based on type
	static Item* Create(uint16_t id, uint16_t subtype = 0xFFFF);
	static Item* Create(pugi::xml_node);
	static Item* Create_OTBM(const IOMap& maphandle, BinaryNode* stream);

public:
	~Item();

	Item* deepCopy() const;

	// Static conversions
	static std::string LiquidID2Name(uint16_t id);
	static uint16_t LiquidName2ID(std::string id);

	uint16_t getID() const { return typeId; }
	bool isValidID() const { return ItemTypeExists(typeId); }
	const ItemType &getItemType() const noexcept { return GetItemType(typeId); }
	const std::string getName() const { return getItemType().name; }
	//const std::string &getDescription() const { return getItemType().description; }
	bool getFlag(ObjectFlag flag) const;
	int getAttribute(ObjectTypeAttribute attr) const;
	int getAttribute(ObjectInstanceAttribute attr) const;

	int getCount() const;
	double getWeight() const;

	void doRotate() {
		if(getFlag(ROTATE)) {
			setID(getAttribute(ROTATETARGET));
		}
	}

	Brush* getBrush() const { return getItemType().brush; }
	GroundBrush* getGroundBrush() const;
	WallBrush* getWallBrush() const;
	DoorBrush* getDoorBrush() const;
	TableBrush* getTableBrush() const;
	CarpetBrush* getCarpetBrush() const;
	Brush* getDoodadBrush() const { return getItemType().doodad_brush; } // This is not necessarily a doodad brush
	RAWBrush* getRAWBrush() const { return getItemType().raw_brush; }
	uint16_t getGroundEquivalent() const { return getItemType().ground_equivalent; }
	uint16_t hasBorderEquivalent() const { return getItemType().has_equivalent; }
	uint32_t getBorderGroup() const { return getItemType().border_group; }
	bool isOptionalBorder() const { return getItemType().isOptionalBorder; }
	bool isWall() const { return getItemType().isWall; }
	bool isDoor() const { return getItemType().isDoor(); }
	bool isOpen() const { return getItemType().isOpen; }
	bool isBrushDoor() const { return getItemType().isBrushDoor; }
	bool isTable() const { return getItemType().isTable; }
	bool isCarpet() const { return getItemType().isCarpet; }

	// Wall alignment (vertical, horizontal, pole, corner)
	BorderType getWallAlignment() const;
	// Border aligment (south, west etc.)
	BorderType getBorderAlignment() const;

	// Drawing related
	int getFrame() const;
	uint8_t getMiniMapColor() const;
	wxPoint getDrawOffset() const;
	SpriteLight getLight() const;

	// Selection
	bool isSelected() const { return selected; }
	void select() {selected = true; }
	void deselect() {selected = false; }
	void toggleSelection() {selected =! selected; }

protected:
	//Item *container; //unused
	Item *next;
	Item *content;
	int typeId;
	bool selected;
	int attributes[4];

	// TODO(fusion): We still need some way to relate integers with strings for
	// TEXTSTRING and EDITOR attributes. We could use a running index and a hash
	// table or use something similar to what the game server does. The simplest
	// would be a hash table, idk.
	// static int stringAttributeCounter = 0;
	// static std::unordered_map<int, std::string> stringAttributes;
};

typedef std::vector<Item*> ItemVector;

Item* transformItem(Item* old_item, uint16_t new_id, Tile* parent = nullptr);

#endif
