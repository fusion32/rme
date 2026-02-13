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
#include "graphics.h"

struct Item {
	//Item *container = NULL; //unused
	Item *next = NULL;
	Item *content = NULL;
	int typeId = 0;
	bool selected = false;
	int attributes[4] = {};

	static Item* Create(int typeId_, int value = 0);

	Item(int typeId_, int value = 0);
	~Item(void);

	void transform(int newTypeId, int value = 0);
	Item *deepCopy(void) const;

	uint16_t getID() const { return typeId; }
	bool isValidID() const { return ItemTypeExists(typeId); }
	const ItemType &getItemType() const noexcept { return GetItemType(typeId); }
	const std::string &getName() const { return getItemType().name; }
	const std::string &getDescription() const { return getItemType().description; }
	bool getFlag(ObjectFlag flag) const;
	int getAttribute(ObjectTypeAttribute attr) const;
	int getAttributeOffset(ObjectInstanceAttribute attr) const;
	int getAttribute(ObjectInstanceAttribute attr) const;
	const char *getTextAttribute(ObjectInstanceAttribute attr) const;
	int getStackPriority(void) const;
	int getLookID(void) const;
	void setAttribute(ObjectInstanceAttribute attr, int value);
	void setTextAttribute(ObjectInstanceAttribute attr, const char *text);
	int countItems(void) const;

	int getFrame() const;
	uint8_t getMiniMapColor() const;
	wxPoint getDrawOffset() const;
	SpriteLight getLight() const;

	// Wall alignment (vertical, horizontal, pole, corner)
	BorderType getWallAlignment() const;
	// Border aligment (south, west etc.)
	BorderType getBorderAlignment() const;

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
	bool isOpen() const { return getItemType().isOpen; }
	bool isBrushDoor() const { return getItemType().isBrushDoor; }
	bool isTable() const { return getItemType().isTable; }
	bool isCarpet() const { return getItemType().isCarpet; }

	bool isSelected() const { return selected; }
	void select() {selected = true; }
	void deselect() {selected = false; }
	void toggleSelection() {selected = !selected; }
};


#endif
