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

#ifndef RME_CREATURE_H_
#define RME_CREATURE_H_

enum Direction {
	NORTH = 0,
	EAST  = 1,
	SOUTH = 2,
	WEST  = 3,
};

struct Outfit {
	int lookType = 0;
	int lookItem = 0;
	int lookMount = 0;
	int lookAddon = 0;
	int lookHead = 0;
	int lookBody = 0;
	int lookLegs = 0;
	int lookFeet = 0;

	uint32_t getColorHash() const {
		return lookHead << 24 | lookBody << 16 | lookLegs << 8 | lookFeet;
	}
};

class CreatureBrush;
struct Creature {
	int raceId = 0;
	int spawnRadius = 0;
	int spawnAmount = 0;
	int spawnInterval = 0;
	bool selected = false;

	bool isSelected(void) const { return selected; }
	void select(void) { selected = true; }
	void deselect(void) { selected = false; }

	Creature *deepCopy(void) const;
	const std::string &getName(void) const;
	Outfit getOutfit(void) const;
	CreatureBrush *getBrush(void) const;
};

struct CreatureType {
	int raceId = 0;
	Outfit outfit = {};
	CreatureBrush *brush = NULL;
	bool in_other_tileset = false;
	std::string name = {};
};

int GetMinRaceId(void);
int GetMaxRaceId(void);
const CreatureType &GetCreatureType(int raceId);
CreatureType *GetMutableCreatureType(int raceId);
bool LoadCreatureTypes(const wxString &projectDir, wxString &outError, wxArrayString &outWarnings);
void ClearCreatureTypes(void);

#endif //RME_CREATURE_H_
