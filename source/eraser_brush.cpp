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

#include "brush.h"
#include "settings.h"
#include "tile.h"

//=============================================================================
// Eraser brush

EraserBrush::EraserBrush()
{
	////
}

EraserBrush::~EraserBrush()
{
	////
}

std::string EraserBrush::getName() const
{
	return "Eraser";
}

int EraserBrush::getLookID() const
{
	return EDITOR_SPRITE_ERASER;
}

bool EraserBrush::canDraw(Map *map, const Position& position) const
{
	return true;
}

void EraserBrush::undraw(Map *map, Tile* tile)
{
	// TODO(fusion): Figure out which items we want to keep here and how to
	// interact with Config::ERASER_LEAVE_UNIQUE.
	tile->removeItems(
		[](const Item *item){
			return true;
		});
}

void EraserBrush::draw(Map *map, Tile* tile, void* parameter)
{
	tile->removeItems(
		[](const Item *item){
			return !item->getFlag(BANK) && !item->getFlag(CLIP);
		});
}
