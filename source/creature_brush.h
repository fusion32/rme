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
	virtual ~CreatureBrush();

	bool isCreature() const { return true; }
	CreatureBrush* asCreature() { return static_cast<CreatureBrush*>(this); }

	virtual bool canDraw(Map *map, const Position& position) const;
	virtual void draw(Map *map, Tile* tile, void* parameter);
	virtual void undraw(Map *map, Tile* tile);

	const CreatureType &getType() const { return GetCreatureType(raceId); }
	Outfit getOutfit(void) const { return getType().outfit; }

	virtual int getLookID() const { return 0; }
	virtual std::string getName() const { return getType().name; }
	virtual bool canDrag() const { return false; }
	virtual bool canSmear() const { return true; }
	virtual bool oneSizeFitsAll() const { return true; }

protected:
	const int raceId;
};

#endif
