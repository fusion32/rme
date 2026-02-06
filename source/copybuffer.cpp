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

#include "copybuffer.h"
#include "editor.h"
#include "creature.h"
#include "settings.h"
#include "map.h"

CopyBuffer::CopyBuffer() :
	tiles(newd Map())
{
	// no-op
}

int CopyBuffer::GetTileCount()
{
	return tiles ? tiles->getTileCount() : 0;
}

Map *CopyBuffer::getBufferMap()
{
	ASSERT(tiles);
	return tiles;
}

CopyBuffer::~CopyBuffer()
{
	clear();
}

Position CopyBuffer::getPosition() const
{
	ASSERT(tiles);
	return copyPos;
}

void CopyBuffer::clear()
{
	delete tiles;
	tiles = nullptr;
}

void CopyBuffer::copy(int floor)
{
	if(!g_editor.hasSelection()) {
		g_editor.SetStatusText("No tiles to copy.");
		return;
	}

	clear();
	tiles = newd Map();

	int tile_count = 0;
	int item_count = 0;
	copyPos = Position(0xFFFF, 0xFFFF, floor);

	for(Tile* tile : g_editor.getSelection()) {
		++tile_count;

		TileLocation* newlocation = tiles->createTileL(tile->getPosition());
		Tile* copied_tile = tiles->allocator(newlocation);

		for(Item *item = tile->items; item != NULL; item = item->next){
			if(item->isSelected()){
				continue;
			}

			if(item->getFlag(BANK)){
				copied_tile->house_id = tile->house_id;
				copied_tile->flags    = tile->flags;
			}

			++item_count;
			// Copy items to copybuffer
			copied_tile->addItem(item->deepCopy());
		}

		if(tile->creature && tile->creature->isSelected()) {
			copied_tile->creature = tile->creature->deepCopy();
		}
		if(tile->spawn && tile->spawn->isSelected()) {
			copied_tile->spawn = tile->spawn->deepCopy();
		}

		tiles->setTile(copied_tile);

		if(copied_tile->getX() < copyPos.x)
			copyPos.x = copied_tile->getX();

		if(copied_tile->getY() < copyPos.y)
			copyPos.y = copied_tile->getY();
	}

	std::ostringstream ss;
	ss << "Copied " << tile_count << " tile" << (tile_count > 1 ? "s" : "") <<  " (" << item_count << " item" << (item_count > 1? "s" : "") << ")";
	g_editor.SetStatusText(wxstr(ss.str()));
}

void CopyBuffer::cut(int floor)
{
	if(!g_editor.hasSelection()) {
		g_editor.SetStatusText("No tiles to cut.");
		return;
	}

	clear();
	tiles = newd Map();

	Map &map = g_editor.map;
	int tile_count = 0;
	int item_count = 0;
	copyPos = Position(0xFFFF, 0xFFFF, floor);

	BatchAction* batch = g_editor.createBatch(ACTION_CUT_TILES);
	Action* action = g_editor.createAction(batch);

	PositionList tilestoborder;

	for(Tile* tile : g_editor.getSelection()) {
		tile_count++;

		Tile* newtile = tile->deepCopy(map);
		Tile* copied_tile = tiles->allocator(tile->getLocation());

		if(Item *ground = tile->getFirstItem(BANK)){
			if(ground->isSelected()){
				copied_tile->house_id = newtile->house_id;
				copied_tile->flags = newtile->flags;
				newtile->flags = 0;
				newtile->house_id = 0;
			}
		}

		item_count += copied_tile->addItems(newtile->popSelectedItems());

		if(newtile->creature && newtile->creature->isSelected()) {
			copied_tile->creature = newtile->creature;
			newtile->creature = nullptr;
		}

		if(newtile->spawn && newtile->spawn->isSelected()) {
			copied_tile->spawn = newtile->spawn;
			newtile->spawn = nullptr;
		}

		tiles->setTile(copied_tile->getPosition(), copied_tile);

		if(copied_tile->getX() < copyPos.x) {
			copyPos.x = copied_tile->getX();
		}

		if(copied_tile->getY() < copyPos.y) {
			copyPos.y = copied_tile->getY();
		}

		if(g_settings.getInteger(Config::USE_AUTOMAGIC)) {
			for(int y = -1; y <= 1; y++)
				for(int x = -1; x <= 1; x++)
					tilestoborder.push_back(Position(tile->getX() + x, tile->getY() + y, tile->getZ()));
		}
		action->addChange(newd Change(newtile));
	}

	batch->addAndCommitAction(action);

	// Remove duplicates
	tilestoborder.sort();
	tilestoborder.unique();

	if(g_settings.getInteger(Config::USE_AUTOMAGIC)) {
		action = g_editor.createAction(batch);
		for(PositionList::iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
			TileLocation* location = map.createTileL(*it);
			if(location->get()) {
				Tile* new_tile = location->get()->deepCopy(map);
				new_tile->borderize(&map);
				new_tile->wallize(&map);
				action->addChange(newd Change(new_tile));
			} else {
				Tile* new_tile = map.allocator(location);
				new_tile->borderize(&map);
				if(new_tile->size()) {
					action->addChange(newd Change(new_tile));
				} else {
					delete new_tile;
				}
			}
		}

		batch->addAndCommitAction(action);
	}

	g_editor.addBatch(batch);
	g_editor.updateActions();

	std::stringstream ss;
	ss << "Cut out " << tile_count << " tile" << (tile_count > 1 ? "s" : "") <<  " (" << item_count << " item" << (item_count > 1? "s" : "") << ")";
	g_editor.SetStatusText(wxstr(ss.str()));
}

void CopyBuffer::paste(const Position& toPosition)
{
	if(!tiles) {
		return;
	}

	Map &map = g_editor.map;

	BatchAction* batchAction = g_editor.createBatch(ACTION_PASTE_TILES);
	Action* action = g_editor.createAction(batchAction);
	for(MapIterator it = tiles->begin(); it != tiles->end(); ++it) {
		Tile* buffer_tile = (*it)->get();
		Position pos = buffer_tile->getPosition() - copyPos + toPosition;

		if(!pos.isValid())
			continue;

		TileLocation* location = map.createTileL(pos);
		Tile* copy_tile = buffer_tile->deepCopy(map);
		Tile* old_dest_tile = location->get();
		Tile* new_dest_tile = nullptr;
		copy_tile->setLocation(location);

		if(g_settings.getInteger(Config::MERGE_PASTE) || !copy_tile->getFlag(BANK)) {
			if(old_dest_tile)
				new_dest_tile = old_dest_tile->deepCopy(map);
			else
				new_dest_tile = map.allocator(location);
			new_dest_tile->merge(copy_tile);
			delete copy_tile;
		} else {
			// If the copied tile has ground, replace target tile
			new_dest_tile = copy_tile;
		}

		// Add all surrounding tiles to the map, so they get borders
		map.createTile(pos.x-1, pos.y-1, pos.z);
		map.createTile(pos.x  , pos.y-1, pos.z);
		map.createTile(pos.x+1, pos.y-1, pos.z);
		map.createTile(pos.x-1, pos.y  , pos.z);
		map.createTile(pos.x+1, pos.y  , pos.z);
		map.createTile(pos.x-1, pos.y+1, pos.z);
		map.createTile(pos.x  , pos.y+1, pos.z);
		map.createTile(pos.x+1, pos.y+1, pos.z);

		action->addChange(newd Change(new_dest_tile));
	}
	batchAction->addAndCommitAction(action);

	if(g_settings.getInteger(Config::USE_AUTOMAGIC) && g_settings.getInteger(Config::BORDERIZE_PASTE)) {
		action = g_editor.createAction(batchAction);
		std::vector<Tile*> borderize_tiles;

		// Go through all modified (selected) tiles (might be slow)
		for(MapIterator it = tiles->begin(); it != tiles->end(); ++it) {
			bool add_me = false; // If this tile is touched
			Position pos = (*it)->getPosition() - copyPos + toPosition;
			if(pos.z < rme::MapMinLayer || pos.z > rme::MapMaxLayer) {
				continue;
			}
			// Go through all neighbours
			Tile* t;
			t = map.getTile(pos.x-1, pos.y-1, pos.z); if(t && !t->isSelected()) { borderize_tiles.push_back(t); add_me = true; }
			t = map.getTile(pos.x  , pos.y-1, pos.z); if(t && !t->isSelected()) { borderize_tiles.push_back(t); add_me = true; }
			t = map.getTile(pos.x+1, pos.y-1, pos.z); if(t && !t->isSelected()) { borderize_tiles.push_back(t); add_me = true; }
			t = map.getTile(pos.x-1, pos.y  , pos.z); if(t && !t->isSelected()) { borderize_tiles.push_back(t); add_me = true; }
			t = map.getTile(pos.x+1, pos.y  , pos.z); if(t && !t->isSelected()) { borderize_tiles.push_back(t); add_me = true; }
			t = map.getTile(pos.x-1, pos.y+1, pos.z); if(t && !t->isSelected()) { borderize_tiles.push_back(t); add_me = true; }
			t = map.getTile(pos.x  , pos.y+1, pos.z); if(t && !t->isSelected()) { borderize_tiles.push_back(t); add_me = true; }
			t = map.getTile(pos.x+1, pos.y+1, pos.z); if(t && !t->isSelected()) { borderize_tiles.push_back(t); add_me = true; }
			if(add_me) borderize_tiles.push_back(map.getTile(pos));
		}

		{ // Remove duplicates
			std::sort(borderize_tiles.begin(), borderize_tiles.end());
			auto end = std::unique(borderize_tiles.begin(), borderize_tiles.end());
			borderize_tiles.erase(end, borderize_tiles.end());
		}

		for(Tile* tile : borderize_tiles) {
			if(tile) {
				Tile* newTile = tile->deepCopy(map);
				newTile->borderize(&map);

				if(Item *ground = tile->getFirstItem(BANK)){
					if(ground->isSelected()){
						newTile->selectGround();
					}
				}

				newTile->wallize(&map);
				action->addChange(newd Change(newTile));
			}
		}

		// Commit changes to map
		batchAction->addAndCommitAction(action);
	}

	g_editor.addBatch(batchAction);
	g_editor.updateActions();
}

bool CopyBuffer::canPaste() const
{
	return GetTileCount() > 0;
}

