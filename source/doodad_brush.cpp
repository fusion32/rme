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

#include "doodad_brush.h"
#include "basemap.h"

//=============================================================================
// Doodad brush

DoodadBrush::DoodadBrush() :
	look_id(0),
	thickness(0),
	thickness_ceiling(0),
	draggable(false),
	on_blocking(false),
	one_size(false),
	do_new_borders(false),
	on_duplicate(false),
	clear_mapflags(0),
	clear_statflags(0)
{
	////
}

DoodadBrush::~DoodadBrush()
{
	for(AlternativeBlock *block: alternatives){
		delete block;
	}
}

DoodadBrush::AlternativeBlock::AlternativeBlock() :
	composite_chance(0),
	single_chance(0)
{
	////
}

DoodadBrush::AlternativeBlock::~AlternativeBlock()
{
	for(CompositeBlock &block: composite_items){
		for(CompositeTile &tile: block.items){
			for(Item *item = tile.first; item != NULL; item = item->next){
				delete item;
			}
		}
	}

	for(SingleBlock &block: single_items){
		delete block.item;
	}
}

static Item *CreateItem(pugi::xml_node node){
	Item *item = NULL;
	if(pugi::xml_attribute id = node.attribute("id")){
		int value = 0;
		if(pugi::xml_attribute attr = node.attribute("count")){
			value = attr.as_int();
		}else if(pugi::xml_attribute attr = node.attribute("subtype")){
			value = attr.as_int();
		}
		item = Item::Create(id.as_int(), value);
	}
	return item;
}

bool DoodadBrush::loadAlternative(pugi::xml_node node, wxArrayString& warnings, AlternativeBlock* which)
{
	AlternativeBlock* alternativeBlock;
	if(which) {
		alternativeBlock = which;
	} else {
		alternativeBlock = newd AlternativeBlock();
	}

	pugi::xml_attribute attribute;
	for(pugi::xml_node childNode: node.children()){
		const std::string& childName = as_lower_str(childNode.name());
		if(childName == "item") {
			if(!(attribute = childNode.attribute("chance"))) {
				warnings.push_back("Can't read chance tag of doodad item node.");
				continue;
			}

			Item* item = CreateItem(childNode);
			if(!item) {
				warnings.push_back("Can't create item from doodad item node.");
				continue;
			}

			if(ItemType* type = GetMutableItemType(item->getID())) {
				type->doodad_brush = this;
			}

			SingleBlock sb{ attribute.as_int(), item };
			alternativeBlock->single_items.push_back(sb);
			alternativeBlock->single_chance += sb.chance;
		} else if(childName == "composite") {
			if(!(attribute = childNode.attribute("chance"))) {
				warnings.push_back("Can't read chance tag of doodad item node.");
				continue;
			}

			alternativeBlock->composite_chance += attribute.as_int();

			CompositeBlock cb;
			cb.chance = alternativeBlock->composite_chance;

			for(pugi::xml_node compositeNode = childNode.first_child(); compositeNode; compositeNode = compositeNode.next_sibling()) {
				if(as_lower_str(compositeNode.name()) != "tile") {
					continue;
				}

				if(!(attribute = compositeNode.attribute("x"))) {
					warnings.push_back("Couldn't read positionX values of composite tile node.");
					continue;
				}

				int32_t x = attribute.as_int();
				if(!(attribute = compositeNode.attribute("y"))) {
					warnings.push_back("Couldn't read positionY values of composite tile node.");
					continue;
				}

				int32_t y = attribute.as_int();
				int32_t z = compositeNode.attribute("z").as_int();
				if(x < -0x7FFF || x > 0x7FFF) {
					warnings.push_back("Invalid range of x value on composite tile node.");
					continue;
				} else if(y < -0x7FFF || y > 0x7FFF) {
					warnings.push_back("Invalid range of y value on composite tile node.");
					continue;
				} else if(z < -0x7 || z > 0x7) {
					warnings.push_back("Invalid range of z value on composite tile node.");
					continue;
				}

				Item *first = NULL;
				Item **tail = &first;
				for(pugi::xml_node itemNode: compositeNode.children("item")){
					if(Item *item = CreateItem(itemNode)){
						*tail = item;
						tail = &item->next;
						if(ItemType *type = GetMutableItemType(item->getID())){
							type->doodad_brush = this;
						}
					}
				}

				if(first != NULL){
					cb.items.push_back(CompositeTile{Position(x, y, z), first});
				}
			}
			alternativeBlock->composite_items.push_back(cb);
		}
	}

	if(!which) {
		alternatives.push_back(alternativeBlock);
	}
	return true;
}

bool DoodadBrush::load(pugi::xml_node node, wxArrayString& warnings)
{
	pugi::xml_attribute attribute;
	if((attribute = node.attribute("lookid"))) {
		look_id = attribute.as_ushort();
	}

	if((attribute = node.attribute("on_blocking"))) {
		on_blocking = attribute.as_bool();
	}

	if((attribute = node.attribute("on_duplicate"))) {
		on_duplicate = attribute.as_bool();
	}

	if((attribute = node.attribute("redo_borders")) || (attribute = node.attribute("reborder"))) {
		do_new_borders = attribute.as_bool();
	}

	if((attribute = node.attribute("one_size"))) {
		one_size = attribute.as_bool();
	}

	if((attribute = node.attribute("draggable"))) {
		draggable = attribute.as_bool();
	}

	if(node.attribute("remove_optional_border").as_bool()) {
		if(!do_new_borders) {
			warnings.push_back("remove_optional_border will not work without redo_borders\n");
		}
		clear_statflags |= TILESTATE_OP_BORDER;
	}

	const std::string& thicknessString = node.attribute("thickness").as_string();
	if(!thicknessString.empty()) {
		size_t slash = thicknessString.find('/');
		if(slash != std::string::npos) {
			try {
				thickness = std::stoi(thicknessString.substr(0, slash));
			} catch (const std::invalid_argument&) {
				thickness = 0;
			} catch (const std::out_of_range&) {
				thickness = 0;
			}
			try {
				thickness_ceiling = std::stoi(thicknessString.substr(slash + 1));
			} catch (const std::invalid_argument&) {
				thickness_ceiling = 0;
			} catch (const std::out_of_range&) {
				thickness_ceiling = 0;
			}
		}
	}

	for(pugi::xml_node childNode = node.first_child(); childNode; childNode = childNode.next_sibling()) {
		if(as_lower_str(childNode.name()) != "alternate") {
			continue;
		}
		if(!loadAlternative(childNode, warnings)) {
			return false;
		}
	}
	loadAlternative(node, warnings, alternatives.empty() ? nullptr : alternatives.back());
	return true;
}

bool DoodadBrush::AlternativeBlock::ownsItem(uint16_t id) const
{
	for(const SingleBlock &block: single_items){
		if(block.item->getID() == id){
			return true;
		}
	}

	for(const CompositeBlock &block: composite_items){
		for(const CompositeTile &tile: block.items){
			for(const Item *item = tile.first; item != NULL; item = item->next){
				if(item->getID() == id){
					return true;
				}
			}
		}
	}

	return false;
}

bool DoodadBrush::ownsItem(const Item* item) const
{
	if(item->getDoodadBrush() == this) return true;
	uint16_t id = item->getID();

	for(AlternativeBlock *block: alternatives){
		if(block->ownsItem(id)) {
			return true;
		}
	}
	return false;
}

void DoodadBrush::undraw(BaseMap* map, Tile* tile)
{
	// Remove all doodad-related
	bool doodadBrushEraseLike = g_settings.getInteger(Config::DOODAD_BRUSH_ERASE_LIKE);
	tile->removeItems(
		[this, doodadBrushEraseLike](const Item *item){
			return item->getDoodadBrush() && (!doodadBrushEraseLike || ownsItem(item));
		});
}

void DoodadBrush::draw(BaseMap* map, Tile* tile, void* parameter)
{
	int variation = 0;
	if(parameter) {
		variation = *reinterpret_cast<int*>(parameter);
	}

	if(alternatives.empty()) return;

	variation %= alternatives.size();
	const AlternativeBlock* ab_ptr = alternatives[variation];
	ASSERT(ab_ptr);

	int roll = random(1, ab_ptr->single_chance);
	for(std::vector<SingleBlock>::const_iterator block_iter = ab_ptr->single_items.begin(); block_iter != ab_ptr->single_items.end(); ++block_iter) {
		const SingleBlock& sb = *block_iter;
		if(roll <= sb.chance) {
			// Use this!
			tile->addItem(sb.item->deepCopy());
			break;
		}
		roll -= sb.chance;
	}
	if(clear_mapflags || clear_statflags) {
		tile->setMapFlags(tile->getMapFlags() & (~clear_mapflags));
		tile->setMapFlags(tile->getStatFlags() & (~clear_statflags));
	}
}

const std::vector<DoodadBrush::CompositeTile> &DoodadBrush::getComposite(int variation) const
{
	static std::vector<CompositeTile> empty;
	if(alternatives.empty())
		return empty;

	const AlternativeBlock* ab = alternatives[variation % alternatives.size()];
	ASSERT(ab_ptr);
	int roll = random(1, ab->composite_chance);
	for(const CompositeBlock &cb: ab->composite_items){
		if(roll <= cb.chance) {
			return cb.items;
		}
	}
	return empty;
}

bool DoodadBrush::isEmpty(int variation) const
{
	if(hasCompositeObjects(variation))
		return false;
	if(hasSingleObjects(variation))
		return false;
	if(thickness <= 0)
		return false;
	return true;
}

int DoodadBrush::getCompositeChance(int ab) const
{
	if(alternatives.empty()) return 0;
	ab %= alternatives.size();
	const AlternativeBlock* ab_ptr = alternatives[ab];
	ASSERT(ab_ptr);
	return ab_ptr->composite_chance;
}

int DoodadBrush::getSingleChance(int ab) const
{
	if(alternatives.empty()) return 0;
	ab %= alternatives.size();
	const AlternativeBlock* ab_ptr = alternatives[ab];
	ASSERT(ab_ptr);
	return ab_ptr->single_chance;
}

int DoodadBrush::getTotalChance(int ab) const
{
	if(alternatives.empty()) return 0;
	ab %= alternatives.size();
	const AlternativeBlock* ab_ptr = alternatives[ab];
	ASSERT(ab_ptr);
	return ab_ptr->composite_chance + ab_ptr->single_chance;
}

bool DoodadBrush::hasSingleObjects(int ab) const
{
	if(alternatives.empty()) return false;
	ab %= alternatives.size();
	AlternativeBlock* ab_ptr = alternatives[ab];
	ASSERT(ab_ptr);
	return ab_ptr->single_chance > 0;
}

bool DoodadBrush::hasCompositeObjects(int ab) const
{
	if(alternatives.empty()) return false;
	ab %= alternatives.size();
	AlternativeBlock* ab_ptr = alternatives[ab];
	ASSERT(ab_ptr);
	return ab_ptr->composite_chance > 0;
}
