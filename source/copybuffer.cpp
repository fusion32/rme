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
#include "common.h"

#include "copybuffer.h"
#include "editor.h"
#include "creature.h"
#include "settings.h"
#include "map.h"

CopyBuffer::~CopyBuffer(void)
{
	clear();
}

void CopyBuffer::clear(void)
{
	delete buffer;
	buffer = nullptr;
}

int CopyBuffer::getTileCount(void) const
{
	return buffer ? buffer->getTileCount() : 0;
}

void CopyBuffer::copy(int floor)
{
	if(!g_editor.hasSelection()) {
		g_editor.SetStatusText("No tiles to copy.");
		return;
	}

	clear();
	buffer = newd Map();

	int numTiles = 0;
	int numItems = 0;
	copyPos = Position(0xFFFF, 0xFFFF, floor);
	for(Tile *tile: g_editor.getSelection()) {
		Tile *copy = buffer->getOrCreateTile(tile->pos);
		for(Item *item = tile->items; item != NULL; item = item->next){
			if(item->isSelected()){
				continue;
			}

			if(item->getFlag(BANK)){
				copy->houseId = tile->houseId;
				copy->flags   = tile->flags;
			}

			numItems += 1;
			// Copy items to copybuffer
			copy->addItem(item->deepCopy());
		}

		if(tile->creature && tile->creature->isSelected()) {
			copy->creature = tile->creature->deepCopy();
		}

		if(copyPos.x > copy->pos.x)
			copyPos.x = copy->pos.x;

		if(copyPos.y > copy->pos.y)
			copyPos.y = copy->pos.y;

		numTiles += 1;
	}

	g_editor.SetStatusText(wxString() << "Copied " << numTiles << " tile"
			<< (numTiles == 1 ? "" : "s") <<  " (" << numItems << " item"
			<< (numItems == 1 ? "" : "s") << ")");
}

void CopyBuffer::cut(int floor)
{
	if(!g_editor.hasSelection()) {
		g_editor.SetStatusText("No tiles to cut.");
		return;
	}

	clear();
	buffer = newd Map();

	int numTiles = 0;
	int numItems = 0;
	copyPos = Position(0xFFFF, 0xFFFF, floor);

	std::vector<Position> tilesToBorderize;
	ActionGroup *group = g_editor.actionQueue.createGroup(ACTION_CUT_TILES);
	{
		Action *action = group->createAction();
		for(Tile *tile: g_editor.getSelection()) {
			// NOTE(fusion): This can be confusing at first glance. We're copying the
			// tile into `newTile`, then splitting it into the selected and non-selected
			// parts. The selected part is put into the copy buffer while the other is
			// put into an action to be commited along with the action group later on.

			Tile newTile; newTile.deepCopy(*tile);
			Tile *copy = buffer->getOrCreateTile(tile->pos);

			if(Item *ground = newTile.getFirstItem(BANK)){
				if(ground->isSelected()){
					copy->houseId   = newTile.houseId;
					copy->flags     = newTile.flags;
					newTile.houseId = 0;
					newTile.flags   = 0;
				}
			}

			numItems += copy->addItems(newTile.popSelectedItems());
			if(newTile.creature && newTile.creature->isSelected()){
				copy->creature   = newTile.creature;
				newTile.creature = NULL;
			}

			if(copyPos.x > newTile.pos.x)
				copyPos.x = newTile.pos.x;

			if(copyPos.y > newTile.pos.y)
				copyPos.y = newTile.pos.y;

			action->changeTile(std::move(newTile));

			if(g_settings.getInteger(Config::USE_AUTOMAGIC)) {
				for(int y = -1; y <= 1; y++)
				for(int x = -1; x <= 1; x++){
					tilesToBorderize.push_back(Position(tile->pos.x + x, tile->pos.y + y, tile->pos.z));
				}
			}

			numTiles += 1;
		}
		action->commit();
	}

	VectorSortUnique(tilesToBorderize);

	if(g_settings.getInteger(Config::USE_AUTOMAGIC)) {
		Action *action = group->createAction();
		for(Position pos: tilesToBorderize){
			Tile newTile;
			if(Tile *tile = g_editor.map.getTile(pos)){
				newTile.deepCopy(*tile);
				newTile.borderize(&g_editor.map);
				newTile.wallize(&g_editor.map);
				action->changeTile(std::move(newTile));
			}else{
				newTile.pos = pos;
				newTile.borderize(&g_editor.map);
				if(!newTile.empty()){
					action->changeTile(std::move(newTile));
				}
			}
		}
		action->commit();
	}

	g_editor.updateActions();
	g_editor.SetStatusText(wxString() << "Cut out " << numTiles << " tile"
			<< (numTiles == 1 ? "" : "s") <<  " (" << numItems << " item"
			<< (numItems == 1 ? "" : "s") << ")");
}

void CopyBuffer::paste(const Position& toPosition)
{
	if(!buffer) {
		return;
	}

	ActionGroup *group = g_editor.actionQueue.createGroup(ACTION_PASTE_TILES);
	{
		Action *action = group->createAction();
		buffer->forEachTile(
			[this, toPosition, action](const Tile *copy, double progress){
				Position pos = toPosition + (copy->pos - copyPos);
				if(!pos.isValid()){
					return;
				}

				Tile newTile; newTile.pos = pos;
				if(g_settings.getBoolean(Config::MERGE_PASTE) || !copy->getFlag(BANK)){
					if(Tile *tile = g_editor.map.getTile(pos)){
						newTile.deepCopy(*tile);
					}
					newTile.mergeCopy(*copy);
				}else{
					newTile.deepCopy(*copy);
				}

				// TODO(fusion): This might be unnecessary?
				// Add all surrounding tiles to the map, so they get borders.
				for(int offsetY = -1; offsetY <= 1; offsetY += 1)
				for(int offsetX = -1; offsetX <= 1; offsetX += 1){
					if(offsetX == 0 && offsetY == 0){
						continue;
					}
					g_editor.map.getOrCreateTile(pos.x + offsetX, pos.y + offsetY, pos.z);
				}

				action->changeTile(std::move(newTile));
			});
		action->commit();
	}

	if(g_settings.getInteger(Config::USE_AUTOMAGIC) && g_settings.getInteger(Config::BORDERIZE_PASTE)) {
		std::vector<Tile*> tilesToBorderize;
		buffer->forEachTile(
			[this, toPosition, &tilesToBorderize](const Tile *copy, double progress){
				Position pos = toPosition + (copy->pos - copyPos);
				if(!pos.isValid()){
					return;
				}

				bool addMe = false;
				for(int offsetY = -1; offsetY <= 1; offsetY += 1)
				for(int offsetX = -1; offsetX <= 1; offsetX += 1){
					if(offsetX == 0 && offsetY == 0){
						continue;
					}
					if(Tile *neighbor = g_editor.map.getTile(pos.x + offsetX, pos.y + offsetY, pos.z)){
						if(!neighbor->isSelected()){
							tilesToBorderize.push_back(neighbor);
							addMe = true;
						}
					}
					g_editor.map.getOrCreateTile(pos.x + offsetX, pos.y + offsetY, pos.z);
				}

				if(addMe){
					if(Tile *tile = g_editor.map.getTile(pos)){
						tilesToBorderize.push_back(tile);
					}
				}
			});

		VectorSortUnique(tilesToBorderize);

		Action *action = group->createAction();
		for(Tile *tile: tilesToBorderize) {
			Tile newTile; newTile.deepCopy(*tile);
			newTile.borderize(&g_editor.map);
			newTile.wallize(&g_editor.map);
			if(Item *ground = tile->getFirstItem(BANK)){
				if(ground->isSelected()){
					newTile.selectGround();
				}
			}
			action->changeTile(std::move(newTile));
		}
		action->commit();
	}

	g_editor.updateActions();
}

