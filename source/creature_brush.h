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

#ifndef RME_CREATURE_BRUSH_H
#define RME_CREATURE_BRUSH_H

#include "brush.h"
#include "creature.h"

//=============================================================================
// CreatureBrush, place creatures

class CreatureBrush : public Brush
{
public:
	CreatureBrush(int raceId_);
	~CreatureBrush() override;

	bool isCreature() const override { return true; }
	CreatureBrush* asCreature() override { return static_cast<CreatureBrush*>(this); }

	bool canDraw(Map *map, const Position& position) const override;
	void draw(Map *map, Tile* tile, void* parameter) override;
	void undraw(Map *map, Tile* tile) override;

	const CreatureType &getType() const { return GetCreatureType(raceId); }
	Outfit getOutfit(void) const { return getType().outfit; }

	int getLookID() const override { return 0; }
	std::string getName() const override { return getType().name; }
	bool canDrag() const override { return false; }
	bool canSmear() const override { return true; }
	bool oneSizeFitsAll() const override { return true; }

protected:
	const int raceId;
};

#endif
