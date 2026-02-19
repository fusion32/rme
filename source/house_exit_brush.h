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

#ifndef RME_HOUSE_EXIT_BRUSH_H
#define RME_HOUSE_EXIT_BRUSH_H

#include "brush.h"

//=============================================================================
// HouseExitBrush, draw house exit tiles
// This doesn't actually draw anything, and the draw/undraw functions will ASSERT if
// you try to call them, so I strongly advice against it

class HouseExitBrush : public Brush
{
public:
	HouseExitBrush();
	~HouseExitBrush() override;

	bool isHouseExit() const override { return true; }
	HouseExitBrush* asHouseExit() override { return static_cast<HouseExitBrush*>(this); }

	// Not used
	bool load(pugi::xml_node node) override { return true; }

	bool canDraw(Map *map, const Position& position) const override;
	// Will ASSERT
	void draw(Map *map, Tile* tile, void* parameter) override;
	void undraw(Map *map, Tile* tile) override;

	bool canDrag() const override { return false; }
	bool canSmear() const override { return false; }
	bool oneSizeFitsAll() const override { return true; }

	void setHouse(uint16_t houseId_) { houseId = houseId_; }
	uint16_t getHouseID() const { return houseId; }

	int getLookID() const override { return 0; } // We don't have a graphic
	std::string getName() const override { return "House Exit Brush"; } // We don't have a name

protected:
	uint16_t houseId;
};

#endif
