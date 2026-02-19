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

#ifndef RME_SELECTION_H
#define RME_SELECTION_H

#include "forward.h"
#include "position.h"

#include <unordered_set>

class Selection
{
public:
	void getBounds(Position &minPos, Position &maxPos) const;

	// IMPORTANT(fusion): Passing a NULL action here is valid and will result
	// in a separate action being created. It helps reducing boilerplate when
	// selecting/deselecting single objects but shouldn't be overused otherwise.
	void add(Action *action, Tile *tile, Item *item);
	void add(Action *action, Tile *tile, Creature *creature);
	void add(Action *action, Tile *tile);
	void remove(Action *action, Tile *tile, Item *item);
	void remove(Action *action, Tile *tile, Creature *creature);
	void remove(Action *action, Tile *tile);
	void clear(Action *action);

	// The tile will be added to the list of selected tiles, however, the items on the tile won't be selected
	void addInternal(Tile *tile);
	void removeInternal(Tile *tile);

	size_t size() const { return tiles.size(); }
	bool empty() const { return tiles.empty(); }
	auto begin() { return tiles.begin(); }
	auto end() { return tiles.end(); }
	const auto &getTiles() const { return tiles; }
	void updateSelectionCount();

	Tile* getSelectedTile() {
		ASSERT(tiles.size() == 1);
		return *tiles.begin();
	}

private:
	std::unordered_set<Tile*> tiles;
};

#endif
