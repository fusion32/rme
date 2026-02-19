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

#include "main.h"

#include "table_brush.h"
#include "editor.h"
#include "map.h"

uint32_t TableBrush::table_types[256];

//=============================================================================
// Table brush

TableBrush::TableBrush() :
	look_id(0)
{
	////
}

TableBrush::~TableBrush()
{
	////
}

bool TableBrush::load(pugi::xml_node node)
{
	if(pugi::xml_attribute attribute = node.attribute("lookid")){
		look_id = (uint16_t)attribute.as_int();
	}

	for(pugi::xml_node tableNode: node.children("table")){
		const std::string& alignString = tableNode.attribute("align").as_string();
		if(alignString.empty()) {
			g_editor.Warning("Could not read type tag of table node");
			continue;
		}

		uint32_t alignment;
		if(alignString == "vertical") {
			alignment = TABLE_VERTICAL;
		} else if(alignString == "horizontal") {
			alignment = TABLE_HORIZONTAL;
		} else if(alignString == "south") {
			alignment = TABLE_SOUTH_END;
		} else if(alignString == "east") {
			alignment = TABLE_EAST_END;
		} else if(alignString == "north") {
			alignment = TABLE_NORTH_END;
		} else if(alignString == "west") {
			alignment = TABLE_WEST_END;
		} else if(alignString == "alone") {
			alignment = TABLE_ALONE;
		} else {
			g_editor.Warning(wxString() << "Unknown table alignment '" << alignString << "'");
			continue;
		}

		for(pugi::xml_node itemNode: tableNode.children("item")){
			uint16_t id = (uint16_t)itemNode.attribute("id").as_int();
			if(id == 0) {
				g_editor.Warning("Could not read id tag of item node");
				break;
			}

			ItemType* type = GetMutableItemType(id);
			if(!type) {
				g_editor.Warning(wxString() << "There is no itemtype with id " << id);
				return false;
			} else if(type->brush && type->brush != this) {
				g_editor.Warning(wxString() << "Itemtype id " << id << " already has a brush");
				return false;
			}

			type->isTable = true;
			type->brush = this;

			TableType tt;
			tt.item_id = id;
			tt.chance = itemNode.attribute("chance").as_int();

			table_items[alignment].total_chance += tt.chance;
			table_items[alignment].items.push_back(tt);
		}
	}
	return true;
}

bool TableBrush::canDraw(Map *map, const Position& position) const
{
	return true;
}

void TableBrush::undraw(Map *map, Tile* t)
{
	t->removeItems(
		[this](const Item *item){
			return item->isTable() && item->getTableBrush() == this;
		});
}

void TableBrush::draw(Map *map, Tile* tile, void* parameter)
{
	undraw(map, tile); // Remove old

	TableNode& tn = table_items[0];
	if(tn.total_chance <= 0) {
		return;
	}
	int chance = random(1, tn.total_chance);
	uint16_t type = 0;

	for(std::vector<TableType>::const_iterator table_iter = tn.items.begin(); table_iter != tn.items.end(); ++table_iter) {
		if(chance <= table_iter->chance) {
			type = table_iter->item_id;
			break;
		}
		chance -= table_iter->chance;
	}

	if(type != 0) {
		tile->addItem(Item::Create(type));
	}
}


bool hasMatchingTableBrushAtTile(Map *map, TableBrush* table_brush, int x, int y, int z)
{
	Tile* t = map->getTile(x, y, z);
	if(!t) return false;

	for(const Item *item = t->items; item != NULL; item = item->next){
		if(item->getTableBrush() == table_brush){
			return true;
		}
	}

	return false;
}

void TableBrush::doTables(Map *map, Tile* tile)
{
	ASSERT(tile);
	if(!tile->getTable()) {
		return;
	}

	int32_t x = tile->pos.x;
	int32_t y = tile->pos.y;
	int32_t z = tile->pos.z;

	for(Item *item = tile->items; item != NULL; item = item->next){
		TableBrush* table_brush = item->getTableBrush();
		if(!table_brush) {
			continue;
		}

		bool neighbours[8] = {};
		neighbours[0] = hasMatchingTableBrushAtTile(map, table_brush, x - 1, y - 1, z);
		neighbours[1] = hasMatchingTableBrushAtTile(map, table_brush, x,     y - 1, z);
		neighbours[2] = hasMatchingTableBrushAtTile(map, table_brush, x + 1, y - 1, z);
		neighbours[3] = hasMatchingTableBrushAtTile(map, table_brush, x - 1, y,     z);
		neighbours[4] = hasMatchingTableBrushAtTile(map, table_brush, x + 1, y,     z);
		neighbours[5] = hasMatchingTableBrushAtTile(map, table_brush, x - 1, y + 1, z);
		neighbours[6] = hasMatchingTableBrushAtTile(map, table_brush, x,     y + 1, z);
		neighbours[7] = hasMatchingTableBrushAtTile(map, table_brush, x + 1, y + 1, z);

		uint32_t tiledata = 0;
		for(int32_t i = 0; i < 8; ++i) {
			if(neighbours[i]) {
				// Same table as this one, calculate what border
				tiledata |= 1 << i;
			}
		}

		BorderType bt = static_cast<BorderType>(table_types[tiledata]);
		TableNode& tn = table_brush->table_items[static_cast<int32_t>(bt)];
		if(tn.total_chance == 0) {
			return;
		}

		int32_t chance = random(1, tn.total_chance);
		uint16_t newId = 0;
		for(const TableType& tableType : tn.items) {
			if(chance <= tableType.chance) {
				newId = tableType.item_id;
				break;
			}
			chance -= tableType.chance;
		}

		if(newId != 0) {
			item->transform(newId);
		}
	}
}
