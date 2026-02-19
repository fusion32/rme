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

#ifndef RME_BRUSH_H_
#define RME_BRUSH_H_
#include "main.h"

#include "forward.h"
#include "position.h"
#include "brush_enums.h"

//=============================================================================
// Brushes, holds all brushes

typedef std::multimap<std::string, Brush*> BrushMap;

class Brushes
{
public:
	Brushes();
	~Brushes();

	void init();
	void clear();

	Brush* getBrush(const std::string& name) const;

	void addBrush(Brush* brush);

	bool unserializeBorder(pugi::xml_node node);
	bool unserializeBrush(pugi::xml_node node);

	const BrushMap& getMap() const noexcept { return brushes; }

protected:
	typedef std::map<uint32_t, AutoBorder*> BorderMap;
	BrushMap brushes;
	BorderMap borders;

	friend class AutoBorder;
	friend class GroundBrush;
};

extern Brushes g_brushes;

//=============================================================================
// Common brush interface

class Brush
{
public:
	Brush();
	virtual ~Brush();

	virtual bool load(pugi::xml_node node) {
		return true;
	}

	virtual void draw(Map *map, Tile* tile, void* parameter = nullptr) = 0;
	virtual void undraw(Map *map, Tile* tile) = 0;
	virtual bool canDraw(Map *map, const Position& position) const = 0;

	//
	uint32_t getID() const noexcept { return id; }

	virtual std::string getName() const = 0;
	virtual void setName(const std::string& newName) {
		ASSERT(!"Brush::setName attempted on nameless brush!");
	}

	virtual int getLookID() const = 0;

	virtual bool needBorders() const { return false; }

	virtual bool canDrag() const { return false; }
	virtual bool canSmear() const { return true; }

	virtual bool oneSizeFitsAll() const { return false; }

	virtual int32_t getMaxVariation() const { return 0; }

	virtual bool isRaw() const { return false; }
	virtual bool isDoodad() const { return false; }
	virtual bool isTerrain() const { return false; }
	virtual bool isGround() const { return false; }
	virtual bool isWall() const { return false; }
	virtual bool isWallDecoration() const { return false; }
	virtual bool isTable() const { return false; }
	virtual bool isCarpet() const { return false; }
	virtual bool isDoor() const { return false; }
	virtual bool isOptionalBorder() const { return false; }
	virtual bool isCreature() const { return false; }
	virtual bool isHouse() const { return false; }
	virtual bool isHouseExit() const { return false; }
	virtual bool isWaypoint() const { return false; }
	virtual bool isFlag() const { return false; }
	virtual bool isEraser() const { return false; }

	virtual RAWBrush* asRaw() { return nullptr; }
	virtual DoodadBrush* asDoodad() { return nullptr; }
	virtual TerrainBrush* asTerrain() { return nullptr; }
	virtual GroundBrush* asGround() { return nullptr; }
	virtual WallBrush* asWall() { return nullptr; }
	virtual WallDecorationBrush* asWallDecoration() { return nullptr; }
	virtual TableBrush* asTable() { return nullptr; }
	virtual CarpetBrush* asCarpet() { return nullptr; }
	virtual DoorBrush* asDoor() { return nullptr; }
	virtual OptionalBorderBrush* asOptionalBorder() { return nullptr; }
	virtual CreatureBrush* asCreature() { return nullptr; }
	virtual HouseBrush* asHouse() { return nullptr; }
	virtual HouseExitBrush* asHouseExit() { return nullptr; }
	virtual WaypointBrush* asWaypoint() { return nullptr; }
	virtual FlagBrush* asFlag() { return nullptr; }
	virtual EraserBrush* asEraser() { return nullptr; }

	bool visibleInPalette() const { return visible; }
	void flagAsVisible() { visible = true; }

protected:
	static uint32_t id_counter;
	uint32_t id;
	bool visible; // Visible in any palette?
};

//=============================================================================
// Terrain brush interface

class TerrainBrush : public Brush
{
public:
	TerrainBrush();
	~TerrainBrush() override;

	bool isTerrain() const override { return true; }
	TerrainBrush* asTerrain() override { return static_cast<TerrainBrush*>(this); }

	bool canDraw(Map *map, const Position& position) const override { return true; }

	std::string getName() const override { return name; }
	void setName(const std::string& newName) override { name = newName; }

	int32_t getLookID() const override { return look_id; }

	bool needBorders() const override { return true; }
	bool canDrag() const override { return true; }

	bool friendOf(TerrainBrush* other);
	int32_t getZ() const { return 0; }

protected:
	std::vector<uint32_t> friends;
	std::string name;
	uint16_t look_id;
	bool hate_friends;
};

//=============================================================================
// FlagBrush, draw PZ etc.

class FlagBrush : public Brush
{
public:
	FlagBrush(uint32_t _flag);
	~FlagBrush() override;

	bool isFlag() const override { return true; }
	FlagBrush* asFlag() override { return static_cast<FlagBrush*>(this); }

	bool canDraw(Map *map, const Position& position) const override;
	void draw(Map *map, Tile* tile, void* parameter) override;
	void undraw(Map *map, Tile* tile) override;

	bool canDrag() const override { return true; }
	int getLookID() const override;
	std::string getName() const override;

protected:
	uint32_t flag;
};

//=============================================================================
// Doorbrush, add doors, windows etc.

class DoorBrush : public Brush
{
public:
	DoorBrush(DoorType _doortype);
	~DoorBrush() override;

	bool isDoor() const override { return true; }
	DoorBrush* asDoor() override { return static_cast<DoorBrush*>(this); }

	static void switchDoor(Item* door);

	bool canDraw(Map *map, const Position& position) const override;
	void draw(Map *map, Tile* tile, void* parameter) override;
	void undraw(Map *map, Tile* tile) override;

	int getLookID() const override;
	std::string getName() const override;
	bool oneSizeFitsAll() const override { return true; }

protected:
	DoorType doortype;
};

//=============================================================================
// OptionalBorderBrush, add gravel 'round mountains.

class OptionalBorderBrush : public Brush
{
public:
	OptionalBorderBrush();
	~OptionalBorderBrush() override;

	bool isOptionalBorder() const override { return true; }
	OptionalBorderBrush* asOptionalBorder() override { return static_cast<OptionalBorderBrush*>(this); }

	bool canDraw(Map *map, const Position& position) const override;
	void draw(Map *map, Tile* tile, void* parameter) override;
	void undraw(Map *map, Tile* tile) override;

	bool canDrag() const override { return true; }
	int getLookID() const override;
	std::string getName() const override;
};

//=============================================================================
// Rampbrush, add ramps to mountains.

class EraserBrush : public Brush
{
public:
	EraserBrush();
	~EraserBrush() override;

	bool isEraser() const override { return true; }
	EraserBrush* asEraser() override { return static_cast<EraserBrush*>(this); }

	bool canDraw(Map *map, const Position& position) const override;
	void draw(Map *map, Tile* tile, void* parameter) override;
	void undraw(Map *map, Tile* tile) override;

	bool needBorders() const override { return true; }
	bool canDrag() const override { return true; }
	int getLookID() const override;
	std::string getName() const override;
};

//=============================================================================
// Autoborder, used by GroundBrush, should be transparent to users

class AutoBorder
{
public:
	AutoBorder(int _id) : id(_id), group(0), ground(false) {
		for(int i = 0; i < 13; i++) tiles[i] = 0;
	}
	~AutoBorder() {}

	static int edgeNameToID(std::string_view edgename);
	bool load(pugi::xml_node node, GroundBrush* owner = nullptr, uint16_t ground_equivalent = 0);

	uint32_t tiles[13];
	uint32_t id;
	uint16_t group;
	bool ground;
};

#endif

