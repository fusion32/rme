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

#ifndef RME_DOODAD_BRUSH_H
#define RME_DOODAD_BRUSH_H

#include "brush.h"

//=============================================================================
// Doodadbrush, add doodads!

class DoodadBrush : public Brush
{
public:
	DoodadBrush();
	~DoodadBrush() override;

	bool isDoodad() const override { return true; }
	DoodadBrush* asDoodad() override { return static_cast<DoodadBrush*>(this); }

protected:
	struct SimpleBlock;
	struct CompositeTile;
	struct CompositeBlock;
	struct AlternativeBlock;

public:
	bool loadAlternative(pugi::xml_node node, AlternativeBlock* which = nullptr);
	bool load(pugi::xml_node node) override;

	bool canDraw(Map *map, const Position& position) const override { return true; }
	void draw(Map *map, Tile* tile, void* parameter) override;
	const std::vector<CompositeTile> &getComposite(int variation) const;
	void undraw(Map *map, Tile* tile) override;

	bool isEmpty(int variation) const;

	int getThickness() const { return thickness; }
	int getThicknessCeiling() const { return thickness_ceiling; }

	int getCompositeChance(int variation) const;
	int getSingleChance(int variation) const;
	int getTotalChance(int variation) const;

	bool hasSingleObjects(int variation) const;
	bool hasCompositeObjects(int variation) const;

	bool placeOnBlocking() const { return on_blocking; }
	bool placeOnDuplicate() const { return on_duplicate; }
	bool doNewBorders() const { return do_new_borders; }
	bool ownsItem(const Item* item) const;

	bool canSmear() const override { return draggable; }
	bool canDrag() const override { return false; }
	bool oneSizeFitsAll() const override { return one_size; }
	int getLookID() const override { return look_id; }
	int getMaxVariation() const override { return alternatives.size(); }
	std::string getName() const override { return name; }
	void setName(const std::string& newName) override { name = newName; }

protected:
	std::string name;
	uint16_t look_id;

	int thickness;
	int thickness_ceiling;

	bool draggable;
	bool on_blocking;
	bool one_size;
	bool do_new_borders;
	bool on_duplicate;

	struct SingleBlock {
		int chance;
		Item* item;
	};

	struct CompositeTile {
		Position offset;
		Item *first;
	};

	struct CompositeBlock {
		int chance;
		std::vector<CompositeTile> items;
	};

	struct AlternativeBlock {
		AlternativeBlock();
		~AlternativeBlock();
		bool ownsItem(uint16_t id) const;
		std::vector<SingleBlock> single_items;
		std::vector<CompositeBlock> composite_items;

		int composite_chance; // Total chance of a composite
		int single_chance; // Total chance of a single object
	};

	std::vector<AlternativeBlock*> alternatives;
};

#endif
