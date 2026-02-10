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

#include "ext/pugixml.cpp"
#include "main.h"

#include "tileset.h"
#include "creature.h"
#include "creature_brush.h"
#include "items.h"
#include "raw_brush.h"

Tileset::Tileset(Brushes& brushes, const std::string& name) :
	name(name),
	brushes(brushes)
{
	////
}

Tileset::~Tileset()
{
	for(TilesetCategoryArray::iterator iter = categories.begin(); iter != categories.end(); ++iter) {
		delete *iter;
	}
}

void Tileset::clear()
{
	for(TilesetCategoryArray::iterator iter = categories.begin(); iter != categories.end(); ++iter) {
		(*iter)->brushlist.clear();
	}
}

bool Tileset::containsBrush(Brush* brush) const
{
	for(TilesetCategoryArray::const_iterator iter = categories.begin(); iter != categories.end(); ++iter)
		if((*iter)->containsBrush(brush))
			return true;

	return false;
}

TilesetCategory* Tileset::getCategory(TilesetCategoryType type)
{
	ASSERT(type >= TILESET_UNKNOWN && type <= TILESET_HOUSE);
	for(TilesetCategoryArray::iterator iter = categories.begin(); iter != categories.end(); ++iter) {
		if((*iter)->getType() == type) {
			return *iter;
		}
	}
	TilesetCategory* tsc = newd TilesetCategory(*this, type);
	categories.push_back(tsc);
	return tsc;
}

bool TilesetCategory::containsBrush(Brush* brush) const
{
	for(std::vector<Brush*>::const_iterator iter = brushlist.begin(); iter != brushlist.end(); ++iter)
		if(*iter == brush)
			return true;

	return false;
}

const TilesetCategory* Tileset::getCategory(TilesetCategoryType type) const
{
	ASSERT(type >= TILESET_UNKNOWN && type <= TILESET_HOUSE);
	for(TilesetCategoryArray::const_iterator iter = categories.begin(); iter != categories.end(); ++iter) {
		if((*iter)->getType() == type) {
			return *iter;
		}
	}
	return nullptr;
}

void Tileset::loadCategory(pugi::xml_node node, wxArrayString &warnings)
{
	TilesetCategory* category = nullptr;
	TilesetCategory* subCategory = nullptr;

	const std::string& nodeName = as_lower_str(node.name());
	if(nodeName == "terrain") {
		category = getCategory(TILESET_TERRAIN);
	} else if(nodeName == "doodad") {
		category = getCategory(TILESET_DOODAD);
	} else if(nodeName == "items") {
		category = getCategory(TILESET_ITEM);
	} else if(nodeName == "raw") {
		category = getCategory(TILESET_RAW);
	} else if(nodeName == "terrain_and_raw") {
		category = getCategory(TILESET_TERRAIN);
		subCategory = getCategory(TILESET_RAW);
	} else if(nodeName == "doodad_and_raw") {
		category = getCategory(TILESET_DOODAD);
		subCategory = getCategory(TILESET_RAW);
	} else if(nodeName == "items_and_raw") {
		category = getCategory(TILESET_ITEM);
		subCategory = getCategory(TILESET_RAW);
	} else if(nodeName == "creatures") {
		category = getCategory(TILESET_CREATURE);
	}

	if(category){
		for(pugi::xml_node childNode: node.children()){
			category->loadBrush(childNode, warnings);
			if(subCategory) {
				subCategory->loadBrush(childNode, warnings);
			}
		}
	}
}

//

TilesetCategory::TilesetCategory(Tileset& parent, TilesetCategoryType type) : type(type), tileset(parent)
{
	ASSERT(type >= TILESET_UNKNOWN && type <= TILESET_HOUSE);
}

TilesetCategory::~TilesetCategory()
{
	ASSERT(type >= TILESET_UNKNOWN && type <= TILESET_HOUSE);
}

bool TilesetCategory::isTrivial() const
{
	return (type == TILESET_ITEM) || (type == TILESET_RAW);
}

void TilesetCategory::loadBrush(pugi::xml_node node, wxArrayString& warnings)
{
	std::vector<Brush*> tmp;
	std::string_view nodeTag = node.name();
	if(nodeTag == "brush") {
		const char *brushName = node.attribute("name").as_string();
		Brush* brush = tileset.brushes.getBrush(brushName);
		if(!brush){
			warnings.push_back(wxString() << "Brush \"" << brushName << "\" doenst exist.");
			return;
		}

		brush->flagAsVisible();
		tmp.push_back(brush);
	}else if(nodeTag == "item"){
		int fromId = 0, toId = 0;
		if(pugi::xml_attribute attr = node.attribute("id")){
			fromId = attr.as_int();
			toId   = fromId;
		}else if(pugi::xml_attribute attr = node.attribute("fromid")){
			fromId = attr.as_int();
			toId   = std::max<int>(fromId, node.attribute("toid").as_int());
		}else{
			warnings.push_back("Tileset item is missing id/fromid attribute");
			return;
		}

		std::vector<Brush*> tempBrushVector;
		for(int typeId = fromId; typeId <= toId; typeId += 1){
			ItemType *type = GetMutableItemType(typeId);
			if(!type) {
				warnings.push_back(wxString() << "Tileset item has invalid item type "
						<< typeId << " (fromId=" << fromId << ", toId=" << toId << ")");
				continue;
			}

			if(!type->raw_brush){
				type->raw_brush = newd RAWBrush(type->typeId);
				type->has_raw = true;
				tileset.brushes.addBrush(type->raw_brush);
			}

			if(type->doodad_brush == NULL && !isTrivial()){
				type->doodad_brush = type->raw_brush;
			}

			type->raw_brush->flagAsVisible();
			tmp.push_back(type->raw_brush);
		}
	}else if(nodeTag == "creature"){
		pugi::xml_attribute raceAttr = node.attribute("race");
		if(!raceAttr){
			warnings.push_back("Tileset creature is missing race attribute.");
			return;
		}

		int raceId = raceAttr.as_int();
		CreatureType *creatureType = GetMutableCreatureType(raceId);
		if(!creatureType){
			warnings.push_back(wxString() << "Invalid creature race" << raceId);
			return;
		}

		if(!creatureType->brush){
			creatureType->brush = newd CreatureBrush(raceId);
			tileset.brushes.addBrush(creatureType->brush);
		}

		creatureType->brush->flagAsVisible(); // ??
		tmp.push_back(creatureType->brush);
	}

	if(pugi::xml_attribute attr = node.attribute("after")){
		const char *after = attr.as_string();
		auto it = brushlist.begin();
		while(it != brushlist.end() && (*it)->getName() != after){
			++it;
		}
		brushlist.insert(it, tmp.begin(), tmp.end());
	}else{
		brushlist.insert(brushlist.end(), tmp.begin(), tmp.end());
	}
}
