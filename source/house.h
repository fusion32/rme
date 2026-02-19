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

#ifndef RME_HOUSE_H_
#define RME_HOUSE_H_

#include "forward.h"
#include "position.h"
#include <vector>

// TODO(fusion): This will probably all change.
struct House {
	int rent = 0;
	uint16_t houseId = 0;
	bool guildHall = false;
	uint32_t townId = 0;
	std::string name = {};
	Position exit = {};
	std::vector<Position> tiles = {};

	size_t size(Map *map) const;
	void clean(Map *map);
	void addTile(Map *map, Position pos);
	void removeTile(Map *map, Position pos);
	void setExit(Map *map, Position pos);
	std::string getDescription(void);
};

class Houses {
	uint16_t maxHouseId = 0;
	std::vector<House*> houses = {};

	Houses(void) = default;
	~Houses(void);
	uint16_t getEmptyID(void);
	void addHouse(Map *map, House *house);
	void removeHouse(Map *map, House *house);
	House *getHouse(uint16_t houseId);
	const House *getHouse(uint16_t houseId) const;
};

#endif
