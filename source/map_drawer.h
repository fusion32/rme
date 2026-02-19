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

#ifndef RME_MAP_DRAWER_H_
#define RME_MAP_DRAWER_H_

class GameSprite;

struct MapTooltip
{
	enum TextLength {
		MAX_CHARS_PER_LINE = 40,
		MAX_CHARS = 255,
	};

	MapTooltip(int x, int y, std::string text, float red, float green, float blue) :
		x(x), y(y), text(text), red(red), green(green), blue(blue) {
		ellipsis = (text.length() - 3) > MAX_CHARS;
	}

	void checkLineEnding() {
		if(text.at(text.size() - 1) == '\n')
			text.resize(text.size() - 1);
	}

	int x, y;
	std::string text;
	float red, green, blue;
	bool ellipsis;
};

// Storage during drawing, for option caching
class DrawingOptions
{
public:
	DrawingOptions();

	void SetIngame();
	void SetDefault();

	bool isOnlyColors() const noexcept;
	bool isTileIndicators() const noexcept;
	bool isTooltips() const noexcept;
	bool isDrawLight() const noexcept;

	bool transparent_floors;
	bool transparent_items;
	bool show_ingame_box;
	bool show_lights;
	bool ingame;
	bool dragging;

	int grid_size;
	bool show_all_floors;
	bool show_creatures;
	bool show_houses;
	bool show_shade;
	bool show_special_tiles;
	bool show_items;

	bool highlight_items;
	bool show_blocking;
	bool show_tooltips;
	bool show_as_minimap;
	bool show_only_colors;
	bool show_only_modified;
	bool show_preview;
	bool show_hooks;
	bool show_pickupables;
	bool show_moveables;
	bool hide_items_when_zoomed;
};

class MapCanvas;
class LightDrawer;

class MapDrawer
{
	MapCanvas* canvas;
	DrawingOptions options;
	std::shared_ptr<LightDrawer> light_drawer;

	float zoom;

	uint32_t current_house_id;

	int mouse_map_x, mouse_map_y;
	int start_x, start_y, start_z;
	int end_x, end_y, end_z, superend_z;
	int view_scroll_x, view_scroll_y;
	int screensize_x, screensize_y;
	int tile_size;
	int floor;

protected:
	std::vector<MapTooltip*> tooltips;
	std::ostringstream tooltip;

	wxStopWatch pos_indicator_timer;
	Position pos_indicator;

public:
	MapDrawer(MapCanvas* canvas);
	~MapDrawer();

	bool dragging;
	bool dragging_draw;

	void SetupVars();
	void SetupGL();
	void Release();

	void Draw();
	void DrawShade(int mapz);
	void DrawMap();
	void DrawSecondaryMap(int mapz);
	void DrawDraggingShadow();
	void DrawHigherFloors();
	void DrawSelectionBox();
	void DrawBrush();
	void DrawIngameBox();
	void DrawGrid(int grid_size);
	void DrawTooltips();

	void TakeScreenshot(uint8_t* screenshot_buffer);

	void ShowPositionIndicator(const Position& position);
	long GetPositionIndicatorTime() const {
		const long time = pos_indicator_timer.Time();
		if(time < rme::PositionIndicatorDuration) {
			return time;
		}
		return 0;
	}

	DrawingOptions& getOptions() noexcept { return options; }

protected:
	void BlitItem(int& screenx, int& screeny, const Tile* tile, const Item* item, bool ephemeral = false, float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f);
	void BlitItem(int& screenx, int& screeny, const Position& pos, const Item* item, bool ephemeral = false, float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f);
	void BlitSpriteType(int screenx, int screeny, uint32_t spriteid, float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f);
	void BlitSpriteType(int screenx, int screeny, GameSprite* spr, float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f);
	void BlitCreature(int screenx, int screeny, const Creature* c, float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f);
	void BlitCreature(int screenx, int screeny, const Outfit& outfit, Direction dir, float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f);
	void DrawTile(Tile *tile);
	void DrawBrushIndicator(int x, int y, Brush* brush, float red = 1.0f, float green = 1.0f, float blue = 1.0f);
	void DrawHookIndicator(int x, int y, const ItemType& type);
	void DrawTileIndicators(Tile *tile);
	void DrawIndicator(int x, int y, int indicator, float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f);
	void DrawPositionIndicator(int z);
	void WriteItemTooltip(const Item* item, std::ostringstream& stream);
	void WriteMarkTooltip(const wxString &name, std::ostringstream& stream);
	void MakeTooltip(int screenx, int screeny, const std::string& text, float red = 1.0f, float green = 1.0f, float blue = 1.0f);
	void AddLight(Tile *tile);

	enum BrushColor {
		COLOR_BRUSH,
		COLOR_HOUSE_BRUSH,
		COLOR_FLAG_BRUSH,
		COLOR_SPAWN_BRUSH,
		COLOR_ERASER,
		COLOR_VALID,
		COLOR_INVALID,
		COLOR_BLANK,
	};

	void getColor(Brush* brush, const Position& position, float &red, float &green, float &blue);
	void glBlitTexture(int x, int y, int textureId, float red, float green, float blue, float alpha, bool adjustZoom = false);
	void glBlitSquare(int x, int y, float red, float green, float blue, float alpha);
	void glBlitSquare(int x, int y, const wxColor& color);
	void glColor(const wxColor& color);
	void glColor(BrushColor color);
	void glColorCheck(Brush* brush, const Position& pos);
	void drawRect(int x, int y, int w, int h, const wxColor& color, int width = 1);
	void drawFilledRect(int x, int y, int w, int h, const wxColor& color);

private:
	void getDrawPosition(const Position& position, int &x, int &y);
};

#endif
