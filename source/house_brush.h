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

#ifndef RME_HOUSE_BRUSH_H
#define RME_HOUSE_BRUSH_H

#include "brush.h"

//=============================================================================
// HouseBrush, draw house tiles
// This brush is created on demand and NOT loaded, as such, the load() method is empty
// Should be deleted by the owning palette

// Forward declaration
class HouseBrush : public Brush
{
public:
	HouseBrush();
	~HouseBrush() override;

	bool isHouse() const override { return true; }
	HouseBrush* asHouse() override { return static_cast<HouseBrush*>(this); }

	// Not used
	bool load(pugi::xml_node node) override { return true; }

	bool canDraw(Map *map, const Position& position) const override { return true; }
	void draw(Map *map, Tile* tile, void* parameter) override;
	void undraw(Map *map, Tile* tile) override;

	bool canDrag() const override { return true; }

	void setHouse(uint16_t houseId_) { houseId = houseId_; }
	uint16_t getHouseID() const { return houseId; }

	int getLookID() const override { return 0; } // We don't have a graphic
	std::string getName() const override { return "House Brush"; }

protected:
	uint16_t houseId;
};

#endif
