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

#ifndef RME_MAP_H_
#define RME_MAP_H_

#include "tile.h"
#include "town.h"
#include "house.h"
#include "spawn.h"
#include "waypoints.h"

#define MAP_SECTOR_SIZE 32
#define MAP_SECTOR_MASK (MAP_SECTOR_SIZE - 1)
STATIC_ASSERT(MAP_SECTOR_SIZE >= 16);
STATIC_ASSERT(ISPOW2(MAP_SECTOR_SIZE));

inline uint32_t GetMapSectorId(int x, int y, int z){
	ASSERT(x >= 0 && x <= UINT16_MAX
		&& y >= 0 && y <= UINT16_MAX
		&& z >= 0 && z <= UINT8_MAX);

	// IMPORTANT(fusion): Each position has 40 bits of information so if we trim
	// 4 bits from both the x and y coordinates, we can obtain an unique identifier
	// for each sector.
	return ((uint32_t)z << 24)
		| (((uint32_t)y & ~MAP_SECTOR_MASK) << 8)
		| (((uint32_t)x & ~MAP_SECTOR_MASK) >> 4);
}

struct MapSector{
	void setTilePositions(int baseX, int baseY, int baseZ){
		for(int offsetY = 0; offsetY < MAP_SECTOR_SIZE; offsetY += 1)
		for(int offsetX = 0; offsetX < MAP_SECTOR_SIZE; offsetX += 1){
			Position tilePos = {
				baseX + offsetX,
				baseY + offsetY,
				baseZ,
			};

			tiles[offsetY * MAP_SECTOR_SIZE + offsetX].pos = tilePos;
		}
	}

	Tile tiles[MAP_SECTOR_SIZE * MAP_SECTOR_SIZE];
};

struct Map {
	wxString mapDir = {};
	wxString saveDir = {};

	int minSectorX = 0;
	int minSectorY = 0;
	int minSectorZ = 0;
	int maxSectorX = 0;
	int maxSectorY = 0;
	int maxSectorZ = 0;

	// IMPORTANT(fusion): `std::unordered_map` is node based so sector and tile
	// pointers should remain stable and valid while their sectors are still in
	// this map.
	std::unordered_map<uint32_t, MapSector> sectors;
	std::unordered_set<uint32_t> dirtySectors;

	//std::vector<Town> towns;
	//std::vector<House> houses;
	//std::vector<Waypoint> waypoints;

	bool load(const wxString &projectDir, wxString &outError, wxArrayString &outWarnings);
	bool save(void);
	void clear(void);

	Tile *getTile(int x, int y, int z);
	Tile *getOrCreateTile(int x, int y, int z);
	Tile *getTile(Position pos) { return getTile(pos.x, pos.y, pos.z); }
	Tile *getOrCreateTile(Position pos) { return getOrCreateTile(pos.x, pos.y, pos.z); }
	bool isEmpty(void) const { return sectors.empty(); }
	bool isDirty(void) const { return !dirtySectors.empty(); }

	int getTileCount(void) const {
		return (int)sectors.size() * MAP_SECTOR_SIZE * MAP_SECTOR_SIZE;
	}

	int getWidth(void) const {
		return (maxSectorX - minSectorX + 1) * MAP_SECTOR_SIZE;
	}

	int getHeight(void) const {
		return (maxSectorY - minSectorY + 1) * MAP_SECTOR_SIZE;
	}

	Position getMinPosition(void) const {
		return Position{
			minSectorX * MAP_SECTOR_SIZE,
			minSectorY * MAP_SECTOR_SIZE,
			minSectorZ,
		};
	}

	Position getMaxPosition(void) const {
		return Position{
			maxSectorX * MAP_SECTOR_SIZE + (MAP_SECTOR_SIZE - 1),
			maxSectorY * MAP_SECTOR_SIZE + (MAP_SECTOR_SIZE - 1),
			maxSectorZ,
		};
	}


	void cleanInvalidTiles(bool showDialog = false);
	bool exportMinimap(const FileName &filename,
						int floor = rme::MapGroundLayer,
						bool showDialog = false);

	template<typename F>
	void forEachTile(F &&f, bool selectedOnly = false){
		int numProcessed = 0;
		int numTiles = getTileCount();
		for(auto &[sectorId, sector]: sectors){
			for(Tile &tile: sector.tiles){
				f(&tile, ((double)numProcessed / (double)numTiles));
			}
		}
	}

	template<typename F>
	void forEachItem(F &&f, bool selectedOnly = false){
		int numProcessed = 0;
		int numTiles = getTileCount();
		std::queue<Item*> containers;
		for(auto &[sectorId, sector]: sectors){
			for(Tile &tile: sector.tiles){
				for(Item *item = tile.items; item != NULL; item = item->next){
					if(selectedOnly && !item->isSelected()){
						continue;
					}

					f(&tile, item, ((double)numProcessed / (double)numTiles));
					if(item->getFlag(CONTAINER) || item->getFlag(CHEST)) {
						containers.push(item);
						while(!containers.empty()){
							Item *container = containers.front();
							containers.pop();
							for(Item *inner = container->content; inner != NULL; inner = inner->next){
								f(&tile, inner, ((double)numProcessed / (double)numTiles));
								if(inner->getFlag(CONTAINER) || inner->getFlag(CHEST)){
									containers.push(inner);
								}
							}
						}
					}
				}
				numProcessed += 1;
			}
		}
	}

	template<typename Pred>
	int clearTiles(Pred &&predicate){
		int numCleared = 0;
		int numProcessed = 0;
		int numTiles = getTileCount();
		for(auto &[sectorId, sector]: sectors){
			for(Tile &tile: sector.tiles){
				if(predicate(&tile, ((double)numProcessed / (double)numTiles))){
					tile.clear();
					numCleared += 1;
				}
				numProcessed += 1;
			}
		}
		return numCleared;
	}

	template<typename Pred>
	int removeItems(Pred &&predicate){
		int numRemoved = 0;
		int numProcessed = 0;
		int numTiles = getTileCount();
		for(auto &[sectorId, sector]: sectors){
			for(Tile &tile: sector.tiles){
				double progress = ((double)numProcessed / (double)numTiles);
				numRemoved += tile.removeItems(
					[&predicate, progress](const Item *item){
						return predicate(item, progress);
					});
				numProcessed += 1;
			}
		}
		return numRemoved;
	}
};

#endif //RME_MAP_H_
