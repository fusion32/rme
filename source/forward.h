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

#ifndef RME_FORWARD_H_
#define RME_FORWARD_H_

struct Action;
struct ActionGroup;
struct ActionQueue;
struct Creature;
struct CreatureType;
struct Item;
struct ItemType;
struct Map;
struct Outfit;
struct Tile;

class CopyBuffer;
class House;
class Selection;
class Tileset;
class Town;
class Waypoint;
class Waypoints;

class AutoBorder;
class Brush;
class RAWBrush;
class DoodadBrush;
class TerrainBrush;
class GroundBrush;
class WallBrush;
class WallDecorationBrush;
class TableBrush;
class CarpetBrush;
class DoorBrush;
class OptionalBorderBrush;
class CreatureBrush;
class HouseBrush;
class HouseExitBrush;
class WaypointBrush;
class FlagBrush;
class EraserBrush;

// TODO(fusion): Remove these??
#include <vector>
#include <unordered_set>
typedef std::vector<uint32_t> HouseExitList;
typedef std::unordered_set<Tile*> TileSet;
typedef std::vector<Brush*> BrushVector;

#endif //RME_FORWARD_H_
