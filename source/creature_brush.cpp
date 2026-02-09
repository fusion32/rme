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

#include "main.h"

#include "creature_brush.h"
#include "editor.h"
#include "settings.h"
#include "tile.h"
#include "creature.h"

//=============================================================================
// Creature brush

CreatureBrush::CreatureBrush(int raceId_) : Brush(), raceId(raceId_)
{
	// TODO(fusion): Do we want to fail here if there is no creature type with
	// the requested race id?
	if(CreatureType *type = GetMutableCreatureType(raceId)){
		ASSERT(type->brush == NULL);
		type->brush = this;
	}
}

CreatureBrush::~CreatureBrush()
{
	////
}

bool CreatureBrush::canDraw(Map *map, const Position &position) const
{
	bool result = false;
	Tile *tile = map->getTile(position);
	if(tile && tile->creature == NULL && !tile->getFlag(UNPASS) && !tile->getFlag(AVOID)) {
		result = !tile->getTileFlag(TILE_FLAG_PROTECTIONZONE);
	}
	return result;
}

void CreatureBrush::undraw(Map *map, Tile* tile)
{
	delete tile->creature;
	tile->creature = NULL;
}

void CreatureBrush::draw(Map *map, Tile *tile, void *parameter)
{
	ASSERT(tile);
	ASSERT(parameter);
	draw_creature(map, tile);
}

void CreatureBrush::draw_creature(Map *map, Tile *tile)
{
	if(canDraw(map, tile->pos)) {
		undraw(map, tile);

		Creature *creature = newd Creature();
		creature->raceId = raceId;
		creature->spawnRadius = g_editor.GetSpawnRadius();
		creature->spawnAmount = g_editor.GetSpawnAmount();
		creature->spawnInterval = g_editor.GetSpawnInterval();
		creature->selected = false;
		tile->creature = creature;
	}
}
