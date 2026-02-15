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

#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

#include "editor.h"
#include "creature.h"
#include "map_drawer.h"
#include "map_display.h"
#include "copybuffer.h"
#include "graphics.h"
#include "settings.h"

#include "doodad_brush.h"
#include "creature_brush.h"
#include "house_exit_brush.h"
#include "house_brush.h"
#include "wall_brush.h"
#include "carpet_brush.h"
#include "raw_brush.h"
#include "table_brush.h"
#include "waypoint_brush.h"
#include "light_drawer.h"

DrawingOptions::DrawingOptions()
{
	SetDefault();
}

void DrawingOptions::SetDefault()
{
	transparent_floors = false;
	transparent_items = false;
	show_ingame_box = false;
	show_lights = false;
	ingame = false;
	dragging = false;

	show_grid = 0;
	show_all_floors = true;
	show_creatures = true;
	show_houses = true;
	show_shade = true;
	show_special_tiles = true;
	show_items = true;

	highlight_items = false;
	show_blocking = false;
	show_tooltips = false;
	show_as_minimap = false;
	show_only_colors = false;
	show_only_modified = false;
	show_preview = false;
	show_hooks = false;
	show_pickupables = false;
	show_moveables = false;
	hide_items_when_zoomed = true;
}

void DrawingOptions::SetIngame()
{
	transparent_floors = false;
	transparent_items = false;
	show_ingame_box = false;
	show_lights = false;
	ingame = true;
	dragging = false;

	show_grid = 0;
	show_all_floors = true;
	show_creatures = true;
	show_houses = false;
	show_shade = false;
	show_special_tiles = false;
	show_items = true;

	highlight_items = false;
	show_blocking = false;
	show_tooltips = false;
	show_as_minimap = false;
	show_only_colors = false;
	show_only_modified = false;
	show_preview = false;
	show_hooks = false;
	show_pickupables = false;
	show_moveables = false;
	hide_items_when_zoomed = false;
}

bool DrawingOptions::isOnlyColors() const noexcept
{
	return show_as_minimap || show_only_colors;
}

bool DrawingOptions::isTileIndicators() const noexcept
{
	if(isOnlyColors())
		return false;
	return show_pickupables || show_moveables || show_houses;
}

bool DrawingOptions::isTooltips() const noexcept
{
	return show_tooltips && !isOnlyColors();
}

bool DrawingOptions::isDrawLight() const noexcept
{
	return show_ingame_box && show_lights;
}

MapDrawer::MapDrawer(MapCanvas* canvas) : canvas(canvas)
{
	light_drawer = std::make_shared<LightDrawer>();
}

MapDrawer::~MapDrawer()
{
	Release();
}

void MapDrawer::SetupVars()
{
	canvas->MouseToMap(&mouse_map_x, &mouse_map_y);
	canvas->GetViewBox(&view_scroll_x, &view_scroll_y, &screensize_x, &screensize_y);

	dragging = canvas->dragging;
	dragging_draw = canvas->dragging_draw;

	zoom = static_cast<float>(canvas->GetZoom());
	tile_size = int(rme::TileSize / zoom); // after zoom
	floor = canvas->GetFloor();

	if(options.show_all_floors){
		if(floor < 8){
			start_z = rme::MapGroundLayer;
		}else{
			start_z = std::min(rme::MapMaxLayer, floor + 2);
		}
	}else{
		start_z = floor;
	}

	end_z = floor;
	superend_z = (floor > rme::MapGroundLayer ? 8 : 0);

	start_x = view_scroll_x / rme::TileSize;
	start_y = view_scroll_y / rme::TileSize;

	if(floor > rme::MapGroundLayer){
		start_x -= 2;
		start_y -= 2;
	}

	end_x = start_x + screensize_x / tile_size + 2;
	end_y = start_y + screensize_y / tile_size + 2;
}

void MapDrawer::SetupGL()
{
	glViewport(0, 0, screensize_x, screensize_y);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, screensize_x * zoom, screensize_y * zoom, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

void MapDrawer::Release()
{
	for(auto it = tooltips.begin(); it != tooltips.end(); ++it) {
		delete *it;
	}
	tooltips.clear();

	if(light_drawer) {
		light_drawer->clear();
	}

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glDisable(GL_BLEND);
}

void MapDrawer::Draw()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	DrawMap();
	DrawDraggingShadow();
	DrawHigherFloors();
	if(options.dragging)
		DrawSelectionBox();
	DrawBrush();
	if(options.show_grid && zoom <= 10.f)
		DrawGrid();
	if(options.show_ingame_box)
		DrawIngameBox();
	if(options.isTooltips())
		DrawTooltips();
}

inline int getFloorAdjustment(int floor)
{
	if(floor > rme::MapGroundLayer) // Underground
		return 0; // No adjustment
	else
		return rme::TileSize * (rme::MapGroundLayer - floor);
}

void MapDrawer::DrawShade(int map_z)
{
	if(map_z == end_z && start_z != end_z) {
		bool only_colors = options.isOnlyColors();
		if(!only_colors)
			glDisable(GL_TEXTURE_2D);

		float x = screensize_x * zoom;
		float y = screensize_y * zoom;
		glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
		glBegin(GL_QUADS);
			glVertex2f(0, y);
			glVertex2f(x, y);
			glVertex2f(x,0);
			glVertex2f(0,0);
		glEnd();

		if(!only_colors)
			glEnable(GL_TEXTURE_2D);
	}
}

void MapDrawer::DrawMap()
{
	// The current house we're drawing
	current_house_id = 0;
	if(Brush *brush = g_editor.GetCurrentBrush()) {
		if(brush->isHouse())
			current_house_id = brush->asHouse()->getHouseID();
		else if(brush->isHouseExit())
			current_house_id = brush->asHouseExit()->getHouseID();
	}

	bool only_colors = options.isOnlyColors();
	bool tile_indicators = options.isTileIndicators();
	bool draw_lights = options.isDrawLight();

	for(int map_z = start_z; map_z >= superend_z; map_z--) {
		if(options.show_shade) {
			DrawShade(map_z);
		}

		if(map_z >= end_z) {
			if(!only_colors)
				glEnable(GL_TEXTURE_2D);

			int minSectorX = start_x / MAP_SECTOR_SIZE;
			int minSectorY = start_y / MAP_SECTOR_SIZE;
			int maxSectorX = end_x   / MAP_SECTOR_SIZE;
			int maxSectorY = end_y   / MAP_SECTOR_SIZE;

			for(int sectorY = minSectorY; sectorY <= maxSectorY; sectorY += 1)
			for(int sectorX = minSectorX; sectorX <= maxSectorX; sectorX += 1){
				MapSector *sector = g_editor.map.getSectorAt(
						sectorX * MAP_SECTOR_SIZE,
						sectorY * MAP_SECTOR_SIZE,
						map_z);
				if(!sector){
					continue;
				}

				for(int offsetY = 0; offsetY < MAP_SECTOR_SIZE; offsetY += 1)
				for(int offsetX = 0; offsetX < MAP_SECTOR_SIZE; offsetX += 1){
					Tile *tile = sector->getTile(offsetX, offsetY);
					if(!tile || tile->empty()){
						continue;
					}

					if(tile->pos.x < start_x || tile->pos.x > end_x
					|| tile->pos.y < start_y || tile->pos.y > end_y){
						continue;
					}

					DrawTile(tile);
					if(draw_lights){
						AddLight(tile);
					}
				}

				if(tile_indicators){
					for(int offsetY = 0; offsetY < MAP_SECTOR_SIZE; offsetY += 1)
					for(int offsetX = 0; offsetX < MAP_SECTOR_SIZE; offsetX += 1){
						Tile *tile = sector->getTile(offsetX, offsetY);
						if(!tile || tile->empty()){
							continue;
						}

						if(tile->pos.x < start_x || tile->pos.x > end_x
						|| tile->pos.y < start_y || tile->pos.y > end_y){
							continue;
						}

						DrawTileIndicators(tile);
					}
				}
			}

			if(!only_colors)
				glDisable(GL_TEXTURE_2D);

			DrawPositionIndicator(map_z);
		}

		// Draws the doodad preview or the paste preview (or import preview)
		DrawSecondaryMap(map_z);

		--start_x;
		--start_y;
		++end_x;
		++end_y;
	}

	if(!only_colors)
		glEnable(GL_TEXTURE_2D);
}

void MapDrawer::DrawSecondaryMap(int map_z)
{
	if(options.ingame)
		return;

	Map *secondary_map = g_editor.secondary_map;
	if(!secondary_map) return;

	Position normal_pos;
	Position to_pos(mouse_map_x, mouse_map_y, floor);

	if(canvas->isPasting()) {
		normal_pos = g_editor.copybuffer.getPosition();
	} else {
		Brush* brush = g_editor.GetCurrentBrush();
		if(brush && brush->isDoodad()) {
			normal_pos = Position(0x8000, 0x8000, 0x8);
		}
	}

	glEnable(GL_TEXTURE_2D);

	for(int map_x = start_x; map_x <= end_x; map_x++) {
		for(int map_y = start_y; map_y <= end_y; map_y++) {
			Position final_pos(map_x, map_y, map_z);
			Position pos = normal_pos + final_pos - to_pos;
			if(pos.z < 0 || pos.z >= rme::MapLayers) {
				continue;
			}

			Tile* tile = secondary_map->getTile(pos);
			if(!tile) continue;

			int draw_x, draw_y;
			getDrawPosition(final_pos, draw_x, draw_y);

			// Draw ground
			float r = 0.6f, g = 0.6f, b = 0.6f;
			if(const Item *ground = tile->getFirstItem(BANK)) {
				if(options.show_blocking && tile->getFlag(UNPASS)){
					r *= 1.0f; g *= 0.67f; b *= 0.67f;
				}

				if(options.show_special_tiles && tile->getTileFlag(TILE_FLAG_REFRESH)){
					r *= 0.67f; g *= 1.0f; b *= 0.67f;
				}

				if(options.show_special_tiles && tile->getTileFlag(TILE_FLAG_NOLOGOUT)){
					r *= 1.0f; g *= 1.0f; b *= 0.5f;
				}

				if(options.show_houses && tile->houseId != 0){
					r *= 0.5f;
					g *= (tile->houseId == current_house_id ? 1.0f : 0.5f);
					b *= 1.0f;
				}else if(options.show_special_tiles && tile->getTileFlag(TILE_FLAG_PROTECTIONZONE)){
					r *= 0.5f; g *= 1.0f; b *= 0.5f;
				}

				BlitItem(draw_x, draw_y, tile, ground, true, r, g, b, 0.6f);
			}

			bool hidden = options.hide_items_when_zoomed && zoom > 10.f;

			// Draw items
			if(!hidden) {
				for(const Item *item = tile->items; item != NULL; item = item->next){
					// NOTE(fusion): Already handled above.
					if(item->getFlag(BANK)){
						continue;
					}

					if(item->getFlag(CLIP)) {
						BlitItem(draw_x, draw_y, tile, item, true, r, g, b, 0.6f);
					} else {
						BlitItem(draw_x, draw_y, tile, item, true, 0.6f, 0.6f, 0.6f, 0.6f);
					}
				}
			}

			// Draw creature
			if(!hidden && options.show_creatures && tile->creature) {
				BlitCreature(draw_x, draw_y, tile->creature);
			}
		}
	}

	glDisable(GL_TEXTURE_2D);
}

void MapDrawer::DrawIngameBox()
{
	int center_x = start_x + int(screensize_x * zoom / 64);
	int center_y = start_y + int(screensize_y * zoom / 64);

	int offset_y = 2;
	int box_start_map_x = center_x;
	int box_start_map_y = center_y + offset_y;
	int box_end_map_x = center_x + rme::ClientMapWidth;
	int box_end_map_y = center_y + rme::ClientMapHeight + offset_y;

	int box_start_x = box_start_map_x * rme::TileSize - view_scroll_x;
	int box_start_y = box_start_map_y * rme::TileSize - view_scroll_y;
	int box_end_x = box_end_map_x * rme::TileSize - view_scroll_x;
	int box_end_y = box_end_map_y * rme::TileSize - view_scroll_y;

	if(options.isDrawLight()) {
		light_drawer->draw(box_start_map_x, box_start_map_y, view_scroll_x, view_scroll_y);
	}

	static wxColor side_color(0, 0, 0, 200);

	glDisable(GL_TEXTURE_2D);

	// left side
	if(box_start_map_x >= start_x) {
		drawFilledRect(0, 0, box_start_x, screensize_y * zoom, side_color);
	}

	// right side
	if(box_end_map_x < end_x) {
		drawFilledRect(box_end_x, 0, screensize_x * zoom, screensize_y * zoom, side_color);
	}

	// top side
	if(box_start_map_y >= start_y) {
		drawFilledRect(box_start_x, 0, box_end_x-box_start_x, box_start_y, side_color);
	}

	// bottom side
	if(box_end_map_y < end_y) {
		drawFilledRect(box_start_x, box_end_y, box_end_x-box_start_x, screensize_y * zoom, side_color);
	}

	// hidden tiles
	drawRect(box_start_x, box_start_y, box_end_x-box_start_x, box_end_y-box_start_y, *wxRED);

	// visible tiles
	box_start_x += rme::TileSize;
	box_start_y += rme::TileSize;
	box_end_x -= 2 * rme::TileSize;
	box_end_y -= 2 * rme::TileSize;
	drawRect(box_start_x, box_start_y, box_end_x-box_start_x, box_end_y-box_start_y, *wxGREEN);

	// player position
	box_start_x += ((rme::ClientMapWidth/2)-2) * rme::TileSize;
	box_start_y += ((rme::ClientMapHeight/2)-2) * rme::TileSize;
	box_end_x = box_start_x + rme::TileSize;
	box_end_y = box_start_y + rme::TileSize;
	drawRect(box_start_x, box_start_y, box_end_x-box_start_x, box_end_y-box_start_y, *wxGREEN);

	glEnable(GL_TEXTURE_2D);
}

void MapDrawer::DrawGrid()
{
	glDisable(GL_TEXTURE_2D);
	glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
	glBegin(GL_LINES);

	for(int y = start_y; y < end_y; ++y) {
		int py = y * rme::TileSize - view_scroll_y;
		glVertex2f(start_x * rme::TileSize - view_scroll_x, py);
		glVertex2f(end_x * rme::TileSize - view_scroll_x, py);
	}

	for(int x = start_x; x < end_x; ++x) {
		int px = x * rme::TileSize - view_scroll_x;
		glVertex2f(px, start_y * rme::TileSize - view_scroll_y);
		glVertex2f(px, end_y * rme::TileSize - view_scroll_y);
	}

	glEnd();
	glEnable(GL_TEXTURE_2D);
}

void MapDrawer::DrawDraggingShadow()
{
	if(!dragging || options.ingame)
		return;

	glEnable(GL_TEXTURE_2D);

	for(Tile* tile : g_editor.getSelection()) {
		int move_z = canvas->drag_start_z - floor;
		int move_x = canvas->drag_start_x - mouse_map_x;
		int move_y = canvas->drag_start_y - mouse_map_y;

		if(move_x == 0 && move_y == 0 && move_z == 0)
			continue;

		Position position = tile->pos;
		int pos_z = position.z - move_z;
		if(pos_z < 0 || pos_z >= rme::MapLayers) {
			continue;
		}

		int pos_x = position.x - move_x;
		int pos_y = position.y - move_y;

		// On screen and dragging?
		if(pos_x+2 > start_x && pos_x < end_x && pos_y+2 > start_y && pos_y < end_y) {
			Position pos(pos_x, pos_y, pos_z);
			int draw_x, draw_y;
			getDrawPosition(pos, draw_x, draw_y);

			Tile* dest_tile = g_editor.map.getTile(pos);
			for(Item *item = tile->items; item != NULL; item = item->next){
				if(!item->isSelected()){
					continue;
				}

				if(dest_tile)
					BlitItem(draw_x, draw_y, dest_tile, item, true, 0.6f, 0.6f, 0.6f, 0.6f);
				else
					BlitItem(draw_x, draw_y, pos, item, true, 0.6f, 0.6f, 0.6f, 0.6f);
			}

			if(options.show_creatures && tile->creature && tile->creature->isSelected())
				BlitCreature(draw_x, draw_y, tile->creature);
		}
	}

	glDisable(GL_TEXTURE_2D);
}

void MapDrawer::DrawHigherFloors()
{
	if(!options.transparent_floors || floor == 0 || floor == 8)
		return;

	glEnable(GL_TEXTURE_2D);

	int map_z = floor - 1;
	for(int map_x = start_x; map_x <= end_x; map_x++) {
		for(int map_y = start_y; map_y <= end_y; map_y++) {
			Tile* tile = g_editor.map.getTile(map_x, map_y, map_z);
			if(!tile) continue;

			int draw_x, draw_y;
			getDrawPosition(tile->pos, draw_x, draw_y);

			if(const Item *ground = tile->getFirstItem(BANK)) {
				if(tile->getTileFlag(TILE_FLAG_PROTECTIONZONE)) {
					BlitItem(draw_x, draw_y, tile, ground, false, 0.5f, 1.0f, 0.5f, 0.4f);
				} else {
					BlitItem(draw_x, draw_y, tile, ground, false, 1.0f, 1.0f, 1.0f, 0.4f);
				}
			}

			bool hidden = options.hide_items_when_zoomed && zoom > 10.f;
			if(!hidden) {
				for(const Item *item = tile->items; item != NULL; item = item->next){
					// NOTE(fusion): Already handled above.
					if(item->getFlag(BANK)){
						continue;
					}

					BlitItem(draw_x, draw_y, tile, item, false, 1.0f, 1.0f, 1.0f, 0.4f);
				}
			}
		}
	}

	glDisable(GL_TEXTURE_2D);
}

void MapDrawer::DrawSelectionBox()
{
	if (options.ingame) {
		return;
	}

	// Draw bounding box

	int last_click_rx = canvas->last_click_abs_x - view_scroll_x;
	int last_click_ry = canvas->last_click_abs_y - view_scroll_y;
	double cursor_rx = canvas->cursor_x * zoom;
	double cursor_ry = canvas->cursor_y * zoom;

	static double lines[4][4];

	lines[0][0] = last_click_rx;
	lines[0][1] = last_click_ry;
	lines[0][2] = cursor_rx;
	lines[0][3] = last_click_ry;

	lines[1][0] = cursor_rx;
	lines[1][1] = last_click_ry;
	lines[1][2] = cursor_rx;
	lines[1][3] = cursor_ry;

	lines[2][0] = cursor_rx;
	lines[2][1] = cursor_ry;
	lines[2][2] = last_click_rx;
	lines[2][3] = cursor_ry;

	lines[3][0] = last_click_rx;
	lines[3][1] = cursor_ry;
	lines[3][2] = last_click_rx;
	lines[3][3] = last_click_ry;

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LINE_STIPPLE);
	glLineStipple(2, 0xAAAA);
	glLineWidth(1.0);
	glColor4f(1.0,1.0,1.0,1.0);
	glBegin(GL_LINES);
	for(int i = 0; i < 4; i++) {
		glVertex2f(lines[i][0], lines[i][1]);
		glVertex2f(lines[i][2], lines[i][3]);
	}
	glEnd();
	glDisable(GL_LINE_STIPPLE);
	glEnable(GL_TEXTURE_2D);
}

void MapDrawer::DrawBrush()
{
	// TODO(fusion): Some brushes will not be drawn at certain zoom levels, probably
	// due to some issue with precision, or blending, or both, I'm not exactly sure.

	if(options.ingame || !g_editor.IsDrawingMode() || !g_editor.GetCurrentBrush()) {
		return;
	}

	Brush* brush = g_editor.GetCurrentBrush();

	BrushColor brushColor = COLOR_BLANK;
	if(brush->isTerrain() || brush->isTable() || brush->isCarpet())
		brushColor = COLOR_BRUSH;
	else if(brush->isCreature())
		brushColor = COLOR_SPAWN_BRUSH;
	else if(brush->isHouse())
		brushColor = COLOR_HOUSE_BRUSH;
	else if(brush->isFlag())
		brushColor = COLOR_FLAG_BRUSH;
	else if(brush->isEraser())
		brushColor = COLOR_ERASER;

	int adjustment = getFloorAdjustment(floor);

	if(dragging_draw) {
		ASSERT(brush->canDrag());

		if(brush->isWall()) {
			int last_click_start_map_x = std::min(canvas->last_click_map_x, mouse_map_x);
			int last_click_start_map_y = std::min(canvas->last_click_map_y, mouse_map_y);
			int last_click_end_map_x = std::max(canvas->last_click_map_x, mouse_map_x)+1;
			int last_click_end_map_y = std::max(canvas->last_click_map_y, mouse_map_y)+1;

			int last_click_start_sx = last_click_start_map_x * rme::TileSize - view_scroll_x - adjustment;
			int last_click_start_sy = last_click_start_map_y * rme::TileSize - view_scroll_y - adjustment;
			int last_click_end_sx = last_click_end_map_x * rme::TileSize - view_scroll_x - adjustment;
			int last_click_end_sy = last_click_end_map_y * rme::TileSize - view_scroll_y - adjustment;

			int delta_x = last_click_end_sx - last_click_start_sx;
			int delta_y = last_click_end_sy - last_click_start_sy;

			glColor(brushColor);
			glBegin(GL_QUADS);
				{
					glVertex2f(last_click_start_sx, last_click_start_sy + rme::TileSize);
					glVertex2f(last_click_end_sx, last_click_start_sy + rme::TileSize);
					glVertex2f(last_click_end_sx, last_click_start_sy);
					glVertex2f(last_click_start_sx, last_click_start_sy);
				}

				if(delta_y > rme::TileSize) {
					glVertex2f(last_click_start_sx, last_click_end_sy - rme::TileSize);
					glVertex2f(last_click_start_sx + rme::TileSize, last_click_end_sy - rme::TileSize);
					glVertex2f(last_click_start_sx + rme::TileSize, last_click_start_sy + rme::TileSize);
					glVertex2f(last_click_start_sx, last_click_start_sy + rme::TileSize);
				}

				if(delta_x > rme::TileSize && delta_y > rme::TileSize) {
					glVertex2f(last_click_end_sx - rme::TileSize, last_click_start_sy + rme::TileSize);
					glVertex2f(last_click_end_sx, last_click_start_sy + rme::TileSize);
					glVertex2f(last_click_end_sx, last_click_end_sy - rme::TileSize);
					glVertex2f(last_click_end_sx - rme::TileSize, last_click_end_sy - rme::TileSize);
				}

				if(delta_y > rme::TileSize) {
					glVertex2f(last_click_start_sx, last_click_end_sy - rme::TileSize);
					glVertex2f(last_click_end_sx, last_click_end_sy - rme::TileSize);
					glVertex2f(last_click_end_sx, last_click_end_sy);
					glVertex2f(last_click_start_sx, last_click_end_sy);
				}
			glEnd();
		} else {
			if(brush->isRaw())
				glEnable(GL_TEXTURE_2D);

			if(g_editor.GetBrushShape() == BRUSHSHAPE_SQUARE) {
				if(brush->isRaw() || brush->isOptionalBorder()) {
					int start_x, end_x;
					int start_y, end_y;

					if(mouse_map_x < canvas->last_click_map_x) {
						start_x = mouse_map_x;
						end_x = canvas->last_click_map_x;
					} else {
						start_x = canvas->last_click_map_x;
						end_x = mouse_map_x;
					}
					if(mouse_map_y < canvas->last_click_map_y) {
						start_y = mouse_map_y;
						end_y = canvas->last_click_map_y;
					} else {
						start_y = canvas->last_click_map_y;
						end_y = mouse_map_y;
					}

					RAWBrush* raw_brush = nullptr;
					if(brush->isRaw())
						raw_brush = brush->asRaw();

					for(int y = start_y; y <= end_y; y++) {
						int cy = y * rme::TileSize - view_scroll_y - adjustment;
						for(int x = start_x; x <= end_x; x++) {
							int cx = x * rme::TileSize - view_scroll_x - adjustment;
							if(brush->isOptionalBorder())
								glColorCheck(brush, Position(x, y, floor));
							else
								BlitSpriteType(cx, cy, raw_brush->getItemType()->sprite, 0.6f, 0.6f, 0.6f, 0.6f);
						}
					}
				} else {
					int last_click_start_map_x = std::min(canvas->last_click_map_x, mouse_map_x);
					int last_click_start_map_y = std::min(canvas->last_click_map_y, mouse_map_y);
					int last_click_end_map_x   = std::max(canvas->last_click_map_x, mouse_map_x)+1;
					int last_click_end_map_y   = std::max(canvas->last_click_map_y, mouse_map_y)+1;

					int last_click_start_sx = last_click_start_map_x * rme::TileSize - view_scroll_x - adjustment;
					int last_click_start_sy = last_click_start_map_y * rme::TileSize - view_scroll_y - adjustment;
					int last_click_end_sx = last_click_end_map_x * rme::TileSize - view_scroll_x - adjustment;
					int last_click_end_sy = last_click_end_map_y * rme::TileSize - view_scroll_y - adjustment;

					glColor(brushColor);
					glBegin(GL_QUADS);
						glVertex2f(last_click_start_sx, last_click_start_sy);
						glVertex2f(last_click_end_sx, last_click_start_sy);
						glVertex2f(last_click_end_sx, last_click_end_sy);
						glVertex2f(last_click_start_sx, last_click_end_sy);
					glEnd();
				}
			} else if(g_editor.GetBrushShape() == BRUSHSHAPE_CIRCLE) {
				// Calculate drawing offsets
				int start_x, end_x;
				int start_y, end_y;
				int width = std::max(
					std::abs(std::max(mouse_map_y, canvas->last_click_map_y) - std::min(mouse_map_y, canvas->last_click_map_y)),
					std::abs(std::max(mouse_map_x, canvas->last_click_map_x) - std::min(mouse_map_x, canvas->last_click_map_x))
					);

				if(mouse_map_x < canvas->last_click_map_x) {
					start_x = canvas->last_click_map_x - width;
					end_x = canvas->last_click_map_x;
				} else {
					start_x = canvas->last_click_map_x;
					end_x = canvas->last_click_map_x + width;
				}

				if(mouse_map_y < canvas->last_click_map_y) {
					start_y = canvas->last_click_map_y - width;
					end_y = canvas->last_click_map_y;
				} else {
					start_y = canvas->last_click_map_y;
					end_y = canvas->last_click_map_y + width;
				}

				int center_x = start_x + (end_x - start_x) / 2;
				int center_y = start_y + (end_y - start_y) / 2;
				float radii = width / 2.0f + 0.005f;

				RAWBrush* raw_brush = nullptr;
				if(brush->isRaw())
					raw_brush = brush->asRaw();

				for(int y = start_y-1; y <= end_y+1; y++) {
					int cy = y * rme::TileSize - view_scroll_y - adjustment;
					float dy = center_y - y;
					for(int x = start_x-1; x <= end_x+1; x++) {
						int cx = x * rme::TileSize - view_scroll_x - adjustment;

						float dx = center_x - x;
						//printf("%f;%f\n", dx, dy);
						float distance = sqrt(dx*dx + dy*dy);
						if(distance < radii) {
							if(brush->isRaw()) {
								BlitSpriteType(cx, cy, raw_brush->getItemType()->sprite, 0.6f, 0.6f, 0.6f, 0.6f);
							} else {
								glColor(brushColor);
								glBegin(GL_QUADS);
									glVertex2f(cx, cy + rme::TileSize);
									glVertex2f(cx + rme::TileSize, cy + rme::TileSize);
									glVertex2f(cx + rme::TileSize, cy);
									glVertex2f(cx,   cy);
								glEnd();
							}
						}
					}
				}
			}

			if(brush->isRaw())
				glDisable(GL_TEXTURE_2D);
		}
	} else {
		if(brush->isWall()) {
			int start_map_x = mouse_map_x - g_editor.GetBrushSize();
			int start_map_y = mouse_map_y - g_editor.GetBrushSize();
			int end_map_x   = mouse_map_x + g_editor.GetBrushSize() + 1;
			int end_map_y   = mouse_map_y + g_editor.GetBrushSize() + 1;

			int start_sx = start_map_x * rme::TileSize - view_scroll_x - adjustment;
			int start_sy = start_map_y * rme::TileSize - view_scroll_y - adjustment;
			int end_sx = end_map_x * rme::TileSize - view_scroll_x - adjustment;
			int end_sy = end_map_y * rme::TileSize - view_scroll_y - adjustment;

			int delta_x = end_sx - start_sx;
			int delta_y = end_sy - start_sy;

			glColor(brushColor);
			glBegin(GL_QUADS);
				{
					glVertex2f(start_sx, start_sy + rme::TileSize);
					glVertex2f(end_sx, start_sy + rme::TileSize);
					glVertex2f(end_sx, start_sy);
					glVertex2f(start_sx, start_sy);
				}

				if(delta_y > rme::TileSize) {
					glVertex2f(start_sx, end_sy - rme::TileSize);
					glVertex2f(start_sx + rme::TileSize, end_sy - rme::TileSize);
					glVertex2f(start_sx + rme::TileSize, start_sy + rme::TileSize);
					glVertex2f(start_sx, start_sy + rme::TileSize);
				}

				if(delta_x > rme::TileSize && delta_y > rme::TileSize) {
					glVertex2f(end_sx - rme::TileSize, start_sy + rme::TileSize);
					glVertex2f(end_sx, start_sy + rme::TileSize);
					glVertex2f(end_sx, end_sy - rme::TileSize);
					glVertex2f(end_sx - rme::TileSize, end_sy - rme::TileSize);
				}

				if(delta_y > rme::TileSize) {
					glVertex2f(start_sx, end_sy - rme::TileSize);
					glVertex2f(end_sx, end_sy - rme::TileSize);
					glVertex2f(end_sx, end_sy);
					glVertex2f(start_sx, end_sy);
				}
			glEnd();
		} else if(brush->isDoor()) {
			int cx = (mouse_map_x) * rme::TileSize - view_scroll_x - adjustment;
			int cy = (mouse_map_y) * rme::TileSize - view_scroll_y - adjustment;

			glColorCheck(brush, Position(mouse_map_x, mouse_map_y, floor));
			glBegin(GL_QUADS);
				glVertex2f(cx, cy + rme::TileSize);
				glVertex2f(cx + rme::TileSize, cy + rme::TileSize);
				glVertex2f(cx + rme::TileSize, cy);
				glVertex2f(cx, cy);
			glEnd();
		} else if(brush->isCreature()) {
			// TODO(fusion): We might want to add some settings to control whether this is shown or not?
			{ // draw spawn radius
				int spawnRadius = g_settings.getInteger(Config::SPAWN_RADIUS);
				int spawn_start_x = (mouse_map_x - spawnRadius)     * rme::TileSize - view_scroll_x - adjustment;
				int spawn_start_y = (mouse_map_y - spawnRadius)     * rme::TileSize - view_scroll_y - adjustment;
				int spawn_end_x   = (mouse_map_x + spawnRadius + 1) * rme::TileSize - view_scroll_x - adjustment;
				int spawn_end_y   = (mouse_map_y + spawnRadius + 1) * rme::TileSize - view_scroll_y - adjustment;
				glColor(brushColor);
				glBegin(GL_QUADS);
					glVertex2f(spawn_start_x, spawn_start_y);
					glVertex2f(spawn_end_x,   spawn_start_y);
					glVertex2f(spawn_end_x,   spawn_end_y);
					glVertex2f(spawn_start_x, spawn_end_y);
				glEnd();
			}

			{ // draw creature
				glEnable(GL_TEXTURE_2D);
				int cx = (mouse_map_x) * rme::TileSize - view_scroll_x - adjustment;
				int cy = (mouse_map_y) * rme::TileSize - view_scroll_y - adjustment;
				CreatureBrush* creature_brush = brush->asCreature();
				if(creature_brush->canDraw(&g_editor.map, Position(mouse_map_x, mouse_map_y, floor))){
					BlitCreature(cx, cy, creature_brush->getOutfit(), SOUTH, 1.0f, 1.0f, 1.0f, 0.6f);
				}else{
					BlitCreature(cx, cy, creature_brush->getOutfit(), SOUTH, 1.0f, 0.25f, 0.25f, 0.6f);
				}
				glDisable(GL_TEXTURE_2D);
			}
		} else if(!brush->isDoodad()) {
			RAWBrush* raw_brush = nullptr;
			if(brush->isRaw()) { // Textured brush
				glEnable(GL_TEXTURE_2D);
				raw_brush = brush->asRaw();
			} else {
				glDisable(GL_TEXTURE_2D);
			}

			for(int y = -g_editor.GetBrushSize()-1; y <= g_editor.GetBrushSize()+1; y++) {
				int cy = (mouse_map_y + y) * rme::TileSize - view_scroll_y - adjustment;
				for(int x = -g_editor.GetBrushSize()-1; x <= g_editor.GetBrushSize()+1; x++) {
					int cx = (mouse_map_x + x) * rme::TileSize - view_scroll_x - adjustment;
					if(g_editor.GetBrushShape() == BRUSHSHAPE_SQUARE) {
						if(x >= -g_editor.GetBrushSize() && x <= g_editor.GetBrushSize() && y >= -g_editor.GetBrushSize() && y <= g_editor.GetBrushSize()) {
							if(brush->isRaw()){
								BlitSpriteType(cx, cy, raw_brush->getItemType()->sprite, 0.6f, 0.6f, 0.6f, 0.6f);
							}else if(brush->isWaypoint()){
								float r, g, b;
								getColor(brush, Position(mouse_map_x + x, mouse_map_y + y, floor), r, g, b);
								DrawBrushIndicator(cx, cy, brush, r, g, b);
							}else{
								if(brush->isHouseExit() || brush->isOptionalBorder())
									glColorCheck(brush, Position(mouse_map_x + x, mouse_map_y + y, floor));
								else
									glColor(brushColor);

								glBegin(GL_QUADS);
									glVertex2f(cx, cy + rme::TileSize);
									glVertex2f(cx + rme::TileSize, cy + rme::TileSize);
									glVertex2f(cx + rme::TileSize, cy);
									glVertex2f(cx, cy);
								glEnd();
							}
						}
					} else if(g_editor.GetBrushShape() == BRUSHSHAPE_CIRCLE) {
						double distance = sqrt(double(x*x) + double(y*y));
						if(distance < g_editor.GetBrushSize()+0.005) {
							if(brush->isRaw()){
								BlitSpriteType(cx, cy, raw_brush->getItemType()->sprite, 0.6f, 0.6f, 0.6f, 0.6f);
							}else if(brush->isWaypoint()){
								float r, g, b;
								getColor(brush, Position(mouse_map_x + x, mouse_map_y + y, floor), r, g, b);
								DrawBrushIndicator(cx, cy, brush, r, g, b);
							}else{
								if(brush->isHouseExit() || brush->isOptionalBorder())
									glColorCheck(brush, Position(mouse_map_x + x, mouse_map_y + y, floor));
								else
									glColor(brushColor);

								glBegin(GL_QUADS);
									glVertex2f(cx, cy + rme::TileSize);
									glVertex2f(cx + rme::TileSize, cy + rme::TileSize);
									glVertex2f(cx + rme::TileSize, cy);
									glVertex2f(cx, cy);
								glEnd();
							}
						}
					}
				}
			}

			if(brush->isRaw()) { // Textured brush
				glDisable(GL_TEXTURE_2D);
			} else {
				glEnable(GL_TEXTURE_2D);
			}
		}
	}
}

void MapDrawer::BlitItem(int& draw_x, int& draw_y, const Tile* tile, const Item* item, bool ephemeral, float red, float green, float blue, float alpha)
{
	const ItemType &type = GetItemType(item->getID());
	if(type.typeId == 0) {
		glDisable(GL_TEXTURE_2D);
		glBlitSquare(draw_x, draw_y, *wxRED);
		glEnable(GL_TEXTURE_2D);
		return;
	}

	if(!options.ingame && !ephemeral && item->isSelected()) {
		red *= 0.5f; green *= 0.5f; blue *= 0.5f;
	}

	// TODO(fusion): There is probably a better way to handle this?
	if(!options.ingame && type.getLookId() == 469){
		glDisable(GL_TEXTURE_2D);
		glBlitSquare(draw_x, draw_y, red, green, 0, alpha * 0.67f);
		glEnable(GL_TEXTURE_2D);
		return;
	}else if(!options.ingame && type.getLookId() == 470){
		glDisable(GL_TEXTURE_2D);
		glBlitSquare(draw_x, draw_y, red, 0, 0, alpha * 0.67f);
		glEnable(GL_TEXTURE_2D);
		return;
	}

	if(!ephemeral && type.getFlag(TAKE) && !options.show_items)
		return;

	GameSprite* sprite = type.sprite;
	if(!sprite)
		return;

	int screenx = draw_x - sprite->getDrawOffset().x;
	int screeny = draw_y - sprite->getDrawOffset().y;

	Position pos = tile->pos;

	// Set the newd drawing height accordingly
	draw_x -= sprite->getDrawHeight();
	draw_y -= sprite->getDrawHeight();

	int pattern_x = 0;
	int pattern_y = 0;
	int pattern_z = pos.z % sprite->pattern_z;
	if (type.getFlag(LIQUIDPOOL) || type.getFlag(LIQUIDCONTAINER)){
		int liquidType = type.getFlag(LIQUIDPOOL)
				? item->getAttribute(POOLLIQUIDTYPE)
				: item->getAttribute(CONTAINERLIQUIDTYPE);
		int liquidColor = GetLiquidColor(liquidType);
		pattern_x = (liquidColor % (int)sprite->pattern_x);
		pattern_y = (liquidColor / (int)sprite->pattern_x);
	} else if(type.getFlag(HANG)) {
		if(tile->getFlag(HOOKSOUTH)) {
			pattern_x = 1;
		} else if(tile->getFlag(HOOKEAST)) {
			pattern_x = 2;
		}
	} else if(type.getFlag(CUMULATIVE) && sprite->pattern_x == 4 && sprite->pattern_y == 2) {
		int count = item->getAttribute(AMOUNT);
		if(count <= 0) {
			pattern_x = 0;
			pattern_y = 0;
		} else if(count < 5) {
			pattern_x = count - 1;
			pattern_y = 0;
		} else if(count < 10) {
			pattern_x = 0;
			pattern_y = 1;
		} else if(count < 25) {
			pattern_x = 1;
			pattern_y = 1;
		} else if(count < 50) {
			pattern_x = 2;
			pattern_y = 1;
		} else {
			pattern_x = 3;
			pattern_y = 1;
		}
	} else {
		pattern_x = pos.x % sprite->pattern_x;
		pattern_y = pos.y % sprite->pattern_y;
	}

	if(!ephemeral && options.transparent_items
			&& !type.getFlag(LIQUIDPOOL)
			&& (!type.getFlag(BANK) || sprite->width > 1 || sprite->height > 1)
			&& (!type.getFlag(CLIP) || sprite->width > 1 || sprite->height > 1))
	{
		alpha *= 0.5f;
	}

	int frame = item->getFrame();
	for(int cx = 0; cx != sprite->width; cx++) {
		for(int cy = 0; cy != sprite->height; cy++) {
			for(int cf = 0; cf != sprite->layers; cf++) {
				int texnum = sprite->getHardwareID(cx,cy,cf,
					pattern_x,
					pattern_y,
					pattern_z,
					frame
				);
				glBlitTexture(screenx - cx * rme::TileSize, screeny - cy * rme::TileSize, texnum, red, green, blue, alpha);
			}
		}
	}

	if(options.show_hooks && (type.getFlag(HOOKSOUTH) || type.getFlag(HOOKEAST)))
		DrawHookIndicator(draw_x, draw_y, type);
}

void MapDrawer::BlitItem(int& draw_x, int& draw_y, const Position& pos, const Item* item, bool ephemeral, float red, float green, float blue, float alpha)
{
	const ItemType &type = GetItemType(item->getID());
	if(type.typeId == 0)
		return;

	if(!options.ingame && !ephemeral && item->isSelected()) {
		red *= 0.5f; blue *= 0.5f; green *= 0.5f;
	}

	// TODO(fusion): There is probably a better way to handle this?
	if(!options.ingame && type.getLookId() == 469){
		glDisable(GL_TEXTURE_2D);
		glBlitSquare(draw_x, draw_y, red, green, 0, alpha * 0.67f);
		glEnable(GL_TEXTURE_2D);
		return;
	}else if(!options.ingame && type.getLookId() == 470){
		glDisable(GL_TEXTURE_2D);
		glBlitSquare(draw_x, draw_y, red, 0, 0, alpha * 0.67f);
		glEnable(GL_TEXTURE_2D);
		return;
	}

	if(!ephemeral && type.getFlag(TAKE) && options.show_items)
		return;

	GameSprite* sprite = type.sprite;
	if(!sprite)
		return;

	int screenx = draw_x - sprite->getDrawOffset().x;
	int screeny = draw_y - sprite->getDrawOffset().y;

	// Set the newd drawing height accordingly
	draw_x -= sprite->getDrawHeight();
	draw_y -= sprite->getDrawHeight();


	int pattern_x = 0;
	int pattern_y = 0;
	int pattern_z = pos.z % sprite->pattern_z;
	if (type.getFlag(LIQUIDPOOL) || type.getFlag(LIQUIDCONTAINER)){
		int liquidType = type.getFlag(LIQUIDPOOL)
				? item->getAttribute(POOLLIQUIDTYPE)
				: item->getAttribute(CONTAINERLIQUIDTYPE);
		int liquidColor = GetLiquidColor(liquidType);
		pattern_x = (liquidColor % (int)sprite->pattern_x);
		pattern_y = (liquidColor / (int)sprite->pattern_x);
	} else if(type.getFlag(CUMULATIVE) && sprite->pattern_x == 4 && sprite->pattern_y == 2) {
		int count = item->getAttribute(AMOUNT);
		if(count <= 0) {
			pattern_x = 0;
			pattern_y = 0;
		} else if(count < 5) {
			pattern_x = count - 1;
			pattern_y = 0;
		} else if(count < 10) {
			pattern_x = 0;
			pattern_y = 1;
		} else if(count < 25) {
			pattern_x = 1;
			pattern_y = 1;
		} else if(count < 50) {
			pattern_x = 2;
			pattern_y = 1;
		} else {
			pattern_x = 3;
			pattern_y = 1;
		}
	} else {
		pattern_x = pos.x % sprite->pattern_x;
		pattern_y = pos.y % sprite->pattern_y;
	}

	if(!ephemeral && options.transparent_items
			&& !type.getFlag(LIQUIDPOOL)
			&& (!type.getFlag(BANK) || sprite->width > 1 || sprite->height > 1)
			&& (!type.getFlag(CLIP) || sprite->width > 1 || sprite->height > 1))
	{
		alpha *= 0.5f;
	}

	int frame = item->getFrame();
	for(int cx = 0; cx != sprite->width; ++cx) {
		for(int cy = 0; cy != sprite->height; ++cy) {
			for(int cf = 0; cf != sprite->layers; ++cf) {
				int texnum = sprite->getHardwareID(cx,cy,cf,
					pattern_x,
					pattern_y,
					pattern_z,
					frame
				);
				glBlitTexture(screenx - cx * rme::TileSize, screeny - cy * rme::TileSize, texnum, red, green, blue, alpha);
			}
		}
	}

	if(options.show_hooks && (type.getFlag(HOOKSOUTH) || type.getFlag(HOOKEAST)) && zoom <= 3.0)
		DrawHookIndicator(draw_x, draw_y, type);
}

void MapDrawer::BlitSpriteType(int screenx, int screeny, uint32_t spriteid, float red, float green, float blue, float alpha)
{
	const ItemType &type = GetItemType(spriteid);
	if(type.typeId == 0)
		return;

	GameSprite* sprite = type.sprite;
	if(!sprite)
		return;

	screenx -= sprite->getDrawOffset().x;
	screeny -= sprite->getDrawOffset().y;

	int frame = 0;
	for(int cx = 0; cx != sprite->width; ++cx) {
		for(int cy = 0; cy != sprite->height; ++cy) {
			for(int cf = 0; cf != sprite->layers; ++cf) {
				int texnum = sprite->getHardwareID(cx,cy,cf,0,0,0, frame);
				glBlitTexture(screenx - cx * rme::TileSize, screeny - cy * rme::TileSize, texnum, red, green, blue, alpha);
			}
		}
	}
}

void MapDrawer::BlitSpriteType(int screenx, int screeny, GameSprite* sprite, float red, float green, float blue, float alpha)
{
	if(!sprite) return;

	screenx -= sprite->getDrawOffset().x;
	screeny -= sprite->getDrawOffset().y;

	int frame = 0;
	for(int cx = 0; cx != sprite->width; ++cx) {
		for(int cy = 0; cy != sprite->height; ++cy) {
			for(int cf = 0; cf != sprite->layers; ++cf) {
				int texnum = sprite->getHardwareID(cx,cy,cf,0,0,0, frame);
				glBlitTexture(screenx - cx * rme::TileSize, screeny - cy * rme::TileSize, texnum, red, green, blue, alpha);
			}
		}
	}
}

void MapDrawer::BlitCreature(int screenx, int screeny, const Outfit& outfit, Direction dir, float red, float green, float blue, float alpha)
{
	if(outfit.lookItem != 0) {
		const ItemType& type = GetItemType(outfit.lookItem);
		BlitSpriteType(screenx, screeny, type.sprite, red, green, blue, alpha);
	} else {
		GameSprite* sprite = g_editor.gfx.getCreatureSprite(outfit.lookType);
		if(!sprite || outfit.lookType == 0) {
			return;
		}

		// mount and addon drawing thanks to otc code
		int pattern_z = 0;
		if(outfit.lookMount != 0) {
			if(GameSprite* mountSpr = g_editor.gfx.getCreatureSprite(outfit.lookMount)) {
				for(int cx = 0; cx != mountSpr->width; ++cx) {
					for(int cy = 0; cy != mountSpr->height; ++cy) {
						int texnum = mountSpr->getHardwareID(cx, cy, 0, (int)dir, 0, 0, 0);
						glBlitTexture(screenx - cx * rme::TileSize, screeny - cy * rme::TileSize, texnum, red, green, blue, alpha);
					}
				}
				pattern_z = std::min<int>(1, sprite->pattern_z - 1);
			}
		}

		int frame = 0;

		// pattern_y => creature addon
		for(int pattern_y = 0; pattern_y < sprite->pattern_y; pattern_y++) {

			// continue if we dont have this addon
			if(pattern_y > 0 && !(outfit.lookAddon & (1 << (pattern_y - 1))))
				continue;

			for(int cx = 0; cx != sprite->width; ++cx) {
				for(int cy = 0; cy != sprite->height; ++cy) {
					int texnum = sprite->getHardwareID(cx, cy, (int)dir, pattern_y, pattern_z, outfit, frame);
					glBlitTexture(screenx - cx * rme::TileSize, screeny - cy * rme::TileSize, texnum, red, green, blue, alpha);
				}
			}
		}
	}
}

void MapDrawer::BlitCreature(int screenx, int screeny, const Creature* creature, float red, float green, float blue, float alpha)
{
	if(!options.ingame && creature->isSelected()) {
		red *= 0.5f; blue *= 0.5f; green *= 0.5f;
	}
	BlitCreature(screenx, screeny, creature->getOutfit(), SOUTH, red, green, blue, alpha);
}

void MapDrawer::WriteItemTooltip(const Item* item, std::ostringstream& stream)
{
	if(!item || !ItemTypeExists(item->getID())) return;

	// TODO(fusion): Relevant srv flags/attributes instead?
#if TODO
	if(...){ // check all flags
		if(stream.tellp() > 0){
			stream << "\n";
		}

		// check individual flags and add attributes here
	}
#endif
}

void MapDrawer::WriteWaypointTooltip(const wxString &name, std::ostringstream& stream)
{
	if(stream.tellp() > 0)
		stream << "\n";
	stream << "wp: " << name << "\n";
}

void MapDrawer::DrawTile(Tile *tile)
{
	ASSERT(tile);
	if(options.show_only_modified && !tile->getTileFlag(TILE_FLAG_DIRTY)){
		return;
	}

	// TODO(fusion): Pass `animate` as a parameter to BlitItem.
	//bool animate = options.show_preview && zoom <= 2.0;
	bool only_colors = options.isOnlyColors();
	bool show_tooltips = options.isTooltips();
	Position pos = tile->pos;

	int draw_x, draw_y;
	getDrawPosition(pos, draw_x, draw_y);

	float r = 1.0f, g = 1.0f, b = 1.0f;
	if(only_colors || tile->getFlag(BANK)) {
		if(!options.show_as_minimap) {
			bool showspecial = options.show_only_colors || options.show_special_tiles;

			if(options.show_blocking && tile->getFlag(UNPASS) && tile->size() > 0) {
				r *= 1.00f;
				g *= 0.67f;
				b *= 0.67f;
			}

			{
				int topIndex;
				Item *topItem = tile->getTopItem(&topIndex);
				if(options.highlight_items && topItem && !topItem->getFlag(CLIP)){
					float factor;
					switch(topIndex){
						case 0:  factor = 0.75f; break;
						case 1:  factor = 0.60f; break;
						case 2:  factor = 0.48f; break;
						case 3:  factor = 0.40f; break;
						default: factor = 0.33f; break;
					}

					r *= factor;
					g *= factor;
					b *= 1.0f;
				}
			}

			if(showspecial && tile->getTileFlag(TILE_FLAG_REFRESH)){
				r *= 0.85f;
				g *= 0.90f;
				b *= 0.10f;
			}

			if(showspecial && tile->getTileFlag(TILE_FLAG_NOLOGOUT)){
				r *= 1.00f;
				g *= 1.00f;
				b *= 0.50f;
			}

			if(options.show_houses && tile->houseId != 0){
				r *= 0.50f;
				g *= (tile->houseId == current_house_id ? 1.00f : 0.50f);
				b *= 1.00f;
			}else if(showspecial && tile->getTileFlag(TILE_FLAG_PROTECTIONZONE)){
				r *= 0.50f;
				g *= 1.00f;
				b *= 0.50f;
			}
		}

		if(only_colors){
			glDisable(GL_TEXTURE_2D);
			if(options.show_as_minimap){
				wxColor color = colorFromEightBit(tile->getMiniMapColor());
				glBlitSquare(draw_x, draw_y, color);
			}else if(r < 1.0f || g < 1.0f || b < 1.0f){
				glBlitSquare(draw_x, draw_y, r, g, b, 0.5f);
			}
			glEnable(GL_TEXTURE_2D);
		}else if(const Item *ground = tile->getFirstItem(BANK)){
			if(show_tooltips && pos.z == floor)
				WriteItemTooltip(ground, tooltip);
			BlitItem(draw_x, draw_y, tile, ground, false, r, g, b);
		}
	}

	bool hidden = only_colors || (options.hide_items_when_zoomed && zoom > 10.f);

	if(!hidden) {
		for(const Item *item = tile->items; item != NULL; item = item->next){
			// NOTE(fusion): Already handled above.
			if(item->getFlag(BANK)){
				continue;
			}

			if(show_tooltips && pos.z == floor)
				WriteItemTooltip(item, tooltip);

			if(item->getFlag(CLIP)) {
				BlitItem(draw_x, draw_y, tile, item, false, r, g, b);
			} else {
				BlitItem(draw_x, draw_y, tile, item);
			}
		}
	}

	if(!hidden && options.show_creatures && tile->creature) {
		BlitCreature(draw_x, draw_y, tile->creature);
	}

	if(show_tooltips) {
		MakeTooltip(draw_x, draw_y, tooltip.str());
		tooltip.str("");
	}
}

void MapDrawer::DrawBrushIndicator(int x, int y, Brush* brush, float red, float green, float blue)
{
	x += (rme::TileSize / 2);
	y += (rme::TileSize / 2);

	// 7----0----1
	// |         |
	// 6--5  3--2
	//     \/
	//     4
	static int vertexes[9][2] = {
		{-15, -20},  // 0
		{ 15, -20},  // 1
		{ 15, -5},   // 2
		{ 5,  -5},   // 3
		{ 0,   0},   // 4
		{-5,  -5},   // 5
		{-15, -5},   // 6
		{-15, -20},  // 7
		{-15, -20},  // 0
	};

	// circle
	glBegin(GL_TRIANGLE_FAN);
	glColor4f(0.0f, 0.0f, 0.0f, 0.3f);
	glVertex2i(x, y);
	for(int i = 0; i <= 30; i++) {
		float angle = i * 2.0f * rme::PI / 30;
		glVertex2f(cos(angle) * (rme::TileSize / 2) + x, sin(angle) * (rme::TileSize / 2) + y);
	}
	glEnd();

	// background
	glColor4f(red, green, blue, 0.7f);
	glBegin(GL_POLYGON);
	for(int i = 0; i < 8; ++i)
		glVertex2i(vertexes[i][0] + x, vertexes[i][1] + y);
	glEnd();

	// borders
	glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
	glLineWidth(1.0);
	glBegin(GL_LINES);
	for(int i = 0; i < 8; ++i) {
		glVertex2i(vertexes[i][0] + x, vertexes[i][1] + y);
		glVertex2i(vertexes[i + 1][0] + x, vertexes[i + 1][1] + y);
	}
	glEnd();
}

void MapDrawer::DrawHookIndicator(int x, int y, const ItemType& type)
{
	glDisable(GL_TEXTURE_2D);
	glColor4f(0.0f, 0.0f, 1.0f, 0.80f);
	glBegin(GL_QUADS);
	if(type.getFlag(HOOKSOUTH)) {
		x -= 10;
		y += 10;
		glVertex2f(x, y);
		glVertex2f(x + 10, y);
		glVertex2f(x + 20, y + 10);
		glVertex2f(x + 10, y + 10);
	} else if(type.getFlag(HOOKEAST)) {
		x += 10;
		y -= 10;
		glVertex2f(x, y);
		glVertex2f(x + 10, y + 10);
		glVertex2f(x + 10, y + 20);
		glVertex2f(x, y + 10);
	}
	glEnd();
	glEnable(GL_TEXTURE_2D);
}

void MapDrawer::DrawTileIndicators(Tile *tile)
{
	if(!tile){
		return;
	}

	int x, y;
	getDrawPosition(tile->pos, x, y);
	if(zoom < 10.0 && (options.show_pickupables || options.show_moveables)) {
		float red = 1.0f, green = 1.0f, blue = 1.0f;
		if(tile->houseId != 0) {
			green = 0.0f;
			blue = 0.0f;
		}

		for(const Item *item = tile->items; item != NULL; item = item->next){
			if(item->getFlag(BANK)){
				continue;
			}

			const ItemType &type = item->getItemType();
			bool pickupable = type.getFlag(TAKE);
			bool moveable   = !type.getFlag(UNMOVE);
			if((pickupable && options.show_pickupables) || (moveable && options.show_moveables)) {
				if(pickupable && options.show_pickupables && moveable && options.show_moveables)
					DrawIndicator(x, y, EDITOR_SPRITE_PICKUPABLE_MOVEABLE_ITEM, red, green, blue);
				else if(pickupable && options.show_pickupables)
					DrawIndicator(x, y, EDITOR_SPRITE_PICKUPABLE_ITEM, red, green, blue);
				else if(moveable && options.show_moveables)
					DrawIndicator(x, y, EDITOR_SPRITE_MOVEABLE_ITEM, red, green, blue);
			}
		}
	}

#if TODO
	if(options.show_houses && tile->isHouseExit()) {
		if(tile->hasHouseExit(current_house_id)) {
			DrawIndicator(x, y, EDITOR_SPRITE_HOUSE_EXIT);
		} else {
			DrawIndicator(x, y, EDITOR_SPRITE_HOUSE_EXIT, 0.25f, 0.25f, 1.0f, 0.5f);
		}
	}
#endif
}

void MapDrawer::DrawIndicator(int x, int y, int indicator, float red, float green, float blue, float alpha)
{
	GameSprite* sprite = g_editor.gfx.getEditorSprite(indicator);
	if(sprite == nullptr)
		return;

	int textureId = sprite->getHardwareID(0,0,0,0,0,0,0);
	glBlitTexture(x, y, textureId, red, green, blue, alpha, true);
}

void MapDrawer::DrawPositionIndicator(int z)
{
	if(z != pos_indicator.z
		|| pos_indicator.x < start_x
		|| pos_indicator.x > end_x
		|| pos_indicator.y < start_y
		|| pos_indicator.y > end_y) {
		return;
	}

	const long time = GetPositionIndicatorTime();
	if(time == 0) {
		return;
	}

	int x, y;
	getDrawPosition(pos_indicator, x, y);

	int size = static_cast<int>(rme::TileSize * (0.3f + std::abs(500 - time % 1000) / 1000.f));
	int offset = (rme::TileSize - size) / 2;

	glDisable(GL_TEXTURE_2D);
	drawRect(x + offset + 2, y + offset + 2, size - 4, size - 4, *wxWHITE, 2);
	drawRect(x + offset + 1, y + offset + 1, size - 2, size - 2, *wxBLACK, 2);
	glEnable(GL_TEXTURE_2D);
}

void MapDrawer::DrawTooltips()
{
	if(!options.show_tooltips || tooltips.empty())
		return;

	glDisable(GL_TEXTURE_2D);

	for(MapTooltip* tooltip : tooltips) {
		const char* text = tooltip->text.c_str();
		float line_width = 0.0f;
		float width = 2.0f;
		float height = 14.0f;
		int char_count = 0;
		int line_char_count = 0;

		for(const char* c = text; *c != '\0'; c++) {
			if(*c == '\n' || (line_char_count >= MapTooltip::MAX_CHARS_PER_LINE && *c == ' ')) {
				height += 14.0f;
				line_width = 0.0f;
				line_char_count = 0;
			} else {
				line_width += glutBitmapWidth(GLUT_BITMAP_HELVETICA_12, *c);
			}
			width = std::max<float>(width, line_width);
			char_count++;
			line_char_count++;

			if(tooltip->ellipsis && char_count > (MapTooltip::MAX_CHARS + 3))
				break;
		}

		float scale = zoom < 1.0f ? zoom : 1.0f;

		width = (width + 8.0f) * scale;
		height = (height + 4.0f) * scale;

		float x = tooltip->x + (rme::TileSize / 2.0f);
		float y = tooltip->y + ((rme::TileSize / 2.0f) * scale);
		float center = width / 2.0f;
		float space = (7.0f * scale);
		float startx = x - center;
		float endx = x + center;
		float starty = y - (height + space);
		float endy = y - space;

		// 7----0----1
		// |         |
		// 6--5  3--2
		//     \/
		//     4
		float vertexes[9][2] = {
			{x,         starty}, // 0
			{endx,      starty}, // 1
			{endx,      endy},   // 2
			{x + space, endy},   // 3
			{x,         y},      // 4
			{x - space, endy},   // 5
			{startx,    endy},   // 6
			{startx,    starty}, // 7
			{x,         starty}, // 0
		};

		// background
		glColor4f(tooltip->red, tooltip->green, tooltip->blue, 1.0f);
		glBegin(GL_POLYGON);
		for(int i = 0; i < 8; ++i)
			glVertex2f(vertexes[i][0], vertexes[i][1]);
		glEnd();

		// borders
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
		glLineWidth(1.0);
		glBegin(GL_LINES);
		for(int i = 0; i < 8; ++i) {
			glVertex2f(vertexes[i][0], vertexes[i][1]);
			glVertex2f(vertexes[i + 1][0], vertexes[i + 1][1]);
		}
		glEnd();

		// text
		if(zoom <= 1.0) {
			startx += (3.0f * scale);
			starty += (14.0f * scale);
			glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
			glRasterPos2f(startx, starty);
			char_count = 0;
			line_char_count = 0;
			for(const char* c = text; *c != '\0'; c++) {
				if(*c == '\n' || (line_char_count >= MapTooltip::MAX_CHARS_PER_LINE && *c == ' ')) {
					starty += (14.0f * scale);
					glRasterPos2f(startx, starty);
					line_char_count = 0;
				}
				char_count++;
				line_char_count++;

				if(tooltip->ellipsis && char_count >= MapTooltip::MAX_CHARS) {
					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, '.');
					if(char_count >= (MapTooltip::MAX_CHARS + 2))
						break;
				} else if(!iscntrl(*c)) {
					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
				}
			}
		}
	}

	glEnable(GL_TEXTURE_2D);
}

void MapDrawer::MakeTooltip(int screenx, int screeny, const std::string& text, float red, float green, float blue)
{
	if(text.empty())
		return;

	MapTooltip *tooltip = new MapTooltip(screenx, screeny, text, red, green, blue);
	tooltip->checkLineEnding();
	tooltips.push_back(tooltip);
}

void MapDrawer::AddLight(Tile *tile)
{
	if(!tile){
		return;
	}

	if(!options.isDrawLight()){
		return;
	}

	bool hidden = options.hide_items_when_zoomed && zoom > 10.0f;
	if(!hidden) {
		for(const Item *item = tile->items; item != NULL; item = item->next){
			if(item->getFlag(LIGHT)) {
				light_drawer->addLight(tile->pos.x, tile->pos.y, item->getLight());
			}
		}
	}
}

void MapDrawer::getColor(Brush* brush, const Position& position, float &red, float &green, float &blue)
{
	if(brush->canDraw(&g_editor.map, position)) {
		if(brush->isWaypoint()) {
			red = 0.0f; green = 1.0f; blue = 0.0f;
		} else {
			red = 0.0f; green = 0.0f; blue = 1.0f;
		}
	} else {
		red = 1.0f; green = 0.0f; blue = 0.0f;
	}
}

void MapDrawer::TakeScreenshot(uint8_t* screenshot_buffer)
{
	glFinish(); // Wait for the operation to finish

	glPixelStorei(GL_PACK_ALIGNMENT, 1); // 1 byte alignment

	for(int i = 0; i < screensize_y; ++i)
		glReadPixels(0, screensize_y - i, screensize_x, 1, GL_RGB, GL_UNSIGNED_BYTE, (GLubyte*)(screenshot_buffer) + 3*screensize_x*i);
}

void MapDrawer::ShowPositionIndicator(const Position& position)
{
	pos_indicator = position;
	pos_indicator_timer.Start();
}

void MapDrawer::glBlitTexture(int x, int y, int textureId, float red, float green, float blue, float alpha, bool adjustZoom)
{
	if(textureId <= 0)
		return;

	glBindTexture(GL_TEXTURE_2D, textureId);
	glColor4f(red, green, blue, alpha);
	glBegin(GL_QUADS);

	if(adjustZoom) {
		float size = rme::TileSize;
		if(zoom < 1.0f) {
			float offset = 10 / (10 * zoom);
			size = std::max<float>(16, rme::TileSize * zoom);
			x += offset;
			y += offset;
		} else if(zoom > 1.f) {
			float offset = (10 * zoom);
			size = rme::TileSize + offset;
			x -= offset;
			y -= offset;
		}
		glTexCoord2f(0.f, 0.f); glVertex2f(x, y);
		glTexCoord2f(1.f, 0.f); glVertex2f(x + size, y);
		glTexCoord2f(1.f, 1.f); glVertex2f(x + size, y + size);
		glTexCoord2f(0.f, 1.f); glVertex2f(x, y + size);
	} else {
		glTexCoord2f(0.f, 0.f); glVertex2f(x, y);
		glTexCoord2f(1.f, 0.f); glVertex2f(x + rme::TileSize, y);
		glTexCoord2f(1.f, 1.f); glVertex2f(x + rme::TileSize, y + rme::TileSize);
		glTexCoord2f(0.f, 1.f); glVertex2f(x, y + rme::TileSize);
	}

	glEnd();
}

void MapDrawer::glBlitSquare(int x, int y, float red, float green, float blue, float alpha)
{
	glColor4f(red, green, blue, alpha);
	glBegin(GL_QUADS);
		glVertex2f(x, y);
		glVertex2f(x + rme::TileSize, y);
		glVertex2f(x + rme::TileSize, y + rme::TileSize);
		glVertex2f(x, y + rme::TileSize);
	glEnd();
}

void MapDrawer::glBlitSquare(int x, int y, const wxColor& color)
{
	glColor4ub(color.Red(), color.Green(), color.Blue(), color.Alpha());
	glBegin(GL_QUADS);
		glVertex2f(x, y);
		glVertex2f(x + rme::TileSize, y);
		glVertex2f(x + rme::TileSize, y + rme::TileSize);
		glVertex2f(x, y + rme::TileSize);
	glEnd();
}

void MapDrawer::glColor(const wxColor& color)
{
	glColor4ub(color.Red(), color.Green(), color.Blue(), color.Alpha());
}

void MapDrawer::glColor(MapDrawer::BrushColor color)
{
	switch(color) {
		case COLOR_BRUSH:
			glColor4ub(
				g_settings.getInteger(Config::CURSOR_RED),
				g_settings.getInteger(Config::CURSOR_GREEN),
				g_settings.getInteger(Config::CURSOR_BLUE),
				g_settings.getInteger(Config::CURSOR_ALPHA)
			);
			break;

		case COLOR_FLAG_BRUSH:
		case COLOR_HOUSE_BRUSH:
			glColor4ub(
				g_settings.getInteger(Config::CURSOR_ALT_RED),
				g_settings.getInteger(Config::CURSOR_ALT_GREEN),
				g_settings.getInteger(Config::CURSOR_ALT_BLUE),
				g_settings.getInteger(Config::CURSOR_ALT_ALPHA)
			);
			break;

		case COLOR_SPAWN_BRUSH:
			glColor4f(0.65f, 0.0f, 0.0f, 0.5f);
			break;

		case COLOR_ERASER:
			glColor4f(0.65f, 0.0f, 0.0f, 0.5f);
			break;

		case COLOR_VALID:
			glColor4f(0.0f, 0.65f, 0.0f, 0.5f);
			break;

		case COLOR_INVALID:
			glColor4f(0.65f, 0.0f, 0.0f, 0.5f);
			break;

		default:
			glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
			break;
	}
}

void MapDrawer::glColorCheck(Brush* brush, const Position& pos)
{
	if(brush->canDraw(&g_editor.map, pos))
		glColor(COLOR_VALID);
	else
		glColor(COLOR_INVALID);
}

void MapDrawer::drawRect(int x, int y, int w, int h, const wxColor& color, int width)
{
	glLineWidth(width);
	glColor4ub(color.Red(), color.Green(), color.Blue(), color.Alpha());
	glBegin(GL_LINE_STRIP);
		glVertex2f(x, y);
		glVertex2f(x + w, y);
		glVertex2f(x + w, y + h);
		glVertex2f(x, y + h);
		glVertex2f(x, y);
	glEnd();
}

void MapDrawer::drawFilledRect(int x, int y, int w, int h, const wxColor& color)
{
	glColor4ub(color.Red(), color.Green(), color.Blue(), color.Alpha());
	glBegin(GL_QUADS);
		glVertex2f(x, y);
		glVertex2f(x + w, y);
		glVertex2f(x + w, y + h);
		glVertex2f(x, y + h);
	glEnd();
}

void MapDrawer::getDrawPosition(const Position& position, int& x, int& y)
{
	int offset;
	if(position.z <= rme::MapGroundLayer)
		offset = (rme::MapGroundLayer - position.z) * rme::TileSize;
	else
		offset = rme::TileSize * (floor - position.z);

	x = ((position.x * rme::TileSize) - view_scroll_x) - offset;
	y = ((position.y * rme::TileSize) - view_scroll_y) - offset;
}
