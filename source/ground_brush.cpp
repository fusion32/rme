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

#include "ground_brush.h"
#include "editor.h"
#include "map.h"

uint32_t GroundBrush::border_types[256];

int AutoBorder::edgeNameToID(std::string_view edgename)
{
	if(edgename == "n") {
		return NORTH_HORIZONTAL;
	} else if(edgename == "w") {
		return WEST_HORIZONTAL;
	} else if(edgename == "s") {
		return SOUTH_HORIZONTAL;
	} else if(edgename == "e") {
		return EAST_HORIZONTAL;
	} else if(edgename == "cnw") {
		return NORTHWEST_CORNER;
	} else if(edgename == "cne") {
		return NORTHEAST_CORNER;
	} else if(edgename == "csw") {
		return SOUTHWEST_CORNER;
	} else if(edgename == "cse") {
		return SOUTHEAST_CORNER;
	} else if(edgename == "dnw") {
		return NORTHWEST_DIAGONAL;
	} else if(edgename == "dne") {
		return NORTHEAST_DIAGONAL;
	} else if(edgename == "dsw") {
		return SOUTHWEST_DIAGONAL;
	} else if(edgename == "dse") {
		return SOUTHEAST_DIAGONAL;
	}
	return BORDER_NONE;
}

bool AutoBorder::load(pugi::xml_node node, GroundBrush* owner, uint16_t ground_equivalent)
{
	ASSERT(ground ? ground_equivalent != 0 : true);

	bool optionalBorder = false;
	if(pugi::xml_attribute attr = node.attribute("type")){
		if(std::string_view(attr.as_string()) == "optional"){
			optionalBorder = true;
		}
	}

	if(pugi::xml_attribute attr = node.attribute("group")) {
		group = (uint16_t)attr.as_int();
	}

	for(pugi::xml_node childNode: node.children()){
		pugi::xml_attribute itemAttr = childNode.attribute("item");
		pugi::xml_attribute edgeAttr = childNode.attribute("edge");
		if(!itemAttr || !edgeAttr){
			continue;
		}

		int typeId = itemAttr.as_int();
		ItemType *type = GetMutableItemType(itemAttr.as_int());
		if(!type){
			g_editor.Warning(wxString() << "Invalid item ID " << typeId << " for border " << id);
			continue;
		}

		if(ground) { // We are a ground border
			type->ground_equivalent = ground_equivalent;
			type->brush = owner;

			ItemType* type2 = GetMutableItemType(ground_equivalent);
			type2->has_equivalent = (type2->typeId != 0);
		}

		type->isBorder = true;
		type->isOptionalBorder = type->isOptionalBorder || optionalBorder;
		if(group && !type->border_group) {
			type->border_group = group;
		}

		int32_t edge_id = edgeNameToID(edgeAttr.as_string());
		if(edge_id != BORDER_NONE) {
			tiles[edge_id] = typeId;
			if(type->border_alignment == BORDER_NONE) {
				type->border_alignment = ::BorderType(edge_id);
			}
		}
	}

	return true;
}

GroundBrush::GroundBrush() :
	z_order(0),
	has_zilch_outer_border(false),
	has_zilch_inner_border(false),
	has_outer_border(false),
	has_inner_border(false),
	optional_border(nullptr),
	use_only_optional(false),
	randomize(true),
	total_chance(0)
{
	////
}

GroundBrush::~GroundBrush()
{
	for(BorderBlock* borderBlock : borders) {
		if(borderBlock->autoborder) {
			for(SpecificCaseBlock* specificCaseBlock : borderBlock->specific_cases) {
				delete specificCaseBlock;
			}

			if(borderBlock->autoborder->ground) {
				delete borderBlock->autoborder;
			}
		}
		delete borderBlock;
	}
	borders.clear();
}

bool GroundBrush::load(pugi::xml_node node)
{
	pugi::xml_attribute attribute;
	if((attribute = node.attribute("lookid"))) {
		look_id = (uint16_t)attribute.as_int();
	}

	if((attribute = node.attribute("z-order"))) {
		z_order = attribute.as_int();
	}

	if((attribute = node.attribute("solo_optional"))) {
		use_only_optional = attribute.as_bool();
	}

	if((attribute = node.attribute("randomize"))) {
		randomize = attribute.as_bool();
	}

	for(pugi::xml_node childNode: node.children()){
		std::string_view childName = childNode.name();
		if(childName == "item") {
			uint16_t itemId = (uint16_t)childNode.attribute("id").as_int();
			int32_t chance = childNode.attribute("chance").as_int();

			ItemType *type = GetMutableItemType(itemId);
			if(!type) {
				g_editor.Warning(wxString() << "Invalid item id " << itemId);
				return false;
			}

			if(!type->getFlag(BANK)) {
				g_editor.Warning(wxString() << "Item " << itemId << " is not a ground item.");
				return false;
			}

			if(type->brush && type->brush != this) {
				g_editor.Warning(wxString() << "Item " << itemId << " can not be member of two brushes");
				return false;
			}

			type->brush = this;
			total_chance += chance;

			ItemChanceBlock ci;
			ci.id = itemId;
			ci.chance = total_chance;
			border_items.push_back(ci);
		} else if(childName == "optional") {
			// Mountain border!
			if(optional_border) {
				g_editor.Warning("Duplicate optional borders!");
				continue;
			}

			if((attribute = childNode.attribute("ground_equivalent"))) {
				uint16_t ground_equivalent = (uint16_t)attribute.as_int();

				// Load from inline definition
				const ItemType &type = GetItemType(ground_equivalent);
				if(type.typeId == 0) {
					g_editor.Warning("Invalid id of ground dependency equivalent item.");
					continue;
				} else if(!type.getFlag(BANK)) {
					g_editor.Warning("Ground dependency equivalent is not a ground item.");
					continue;
				} else if(type.brush && type.brush != this) {
					g_editor.Warning("Ground dependency equivalent does not use the same brush as ground border.");
					continue;
				}

				AutoBorder* autoBorder = newd AutoBorder(0); // Empty id basically
				autoBorder->load(childNode, this, ground_equivalent);
				optional_border = autoBorder;
			} else {
				// Load from ID
				if(!(attribute = childNode.attribute("id"))) {
					g_editor.Warning("Missing tag id for border node");
					continue;
				}

				uint16_t id = (uint16_t)attribute.as_int();
				auto it = g_brushes.borders.find(id);
				if(it == g_brushes.borders.end() || !it->second) {
					g_editor.Warning(wxString() << "Could not find border id " << id);
					continue;
				}

				optional_border = it->second;
			}
		} else if(childName == "border") {
			AutoBorder* autoBorder;
			if(!(attribute = childNode.attribute("id"))) {
				if(!(attribute = childNode.attribute("ground_equivalent"))) {
					continue;
				}

				uint16_t ground_equivalent = (uint16_t)attribute.as_int();
				const ItemType &it = GetItemType(ground_equivalent);
				if(it.typeId == 0) {
					g_editor.Warning("Invalid id of ground dependency equivalent item.");
				}

				if(!it.getFlag(BANK)) {
					g_editor.Warning("Ground dependency equivalent is not a ground item.");
				}

				if(it.brush && it.brush != this) {
					g_editor.Warning("Ground dependency equivalent does not use the same brush as ground border.");
				}

				autoBorder = newd AutoBorder(0); // Empty id basically
				autoBorder->load(childNode, this, ground_equivalent);
			} else {
				int32_t id = attribute.as_int();
				if(id == 0) {
					autoBorder = nullptr;
				} else {
					auto it = g_brushes.borders.find(id);
					if(it == g_brushes.borders.end() || !it->second) {
						g_editor.Warning(wxString() << "Could not find border id " << id);
						continue;
					}
					autoBorder = it->second;
				}
			}

			BorderBlock* borderBlock = newd BorderBlock;
			borderBlock->super = false;
			borderBlock->autoborder = autoBorder;

			if((attribute = childNode.attribute("to"))) {
				const std::string& value = attribute.as_string();
				if(value == "all") {
					borderBlock->to = 0xFFFFFFFF;
				} else if(value == "none") {
					borderBlock->to = 0;
				} else {
					Brush* tobrush = g_brushes.getBrush(value);
					if(!tobrush) {
						g_editor.Warning(wxString() << "To brush " << value << " doesn't exist.");
						continue;
					}
					borderBlock->to = tobrush->getID();
				}
			} else {
				borderBlock->to = 0xFFFFFFFF;
			}

			if((attribute = childNode.attribute("super")) && attribute.as_bool()) {
				borderBlock->super = true;
			}

			if((attribute = childNode.attribute("align"))) {
				const std::string& value = attribute.as_string();
				if(value == "outer") {
					borderBlock->outer = true;
				} else if(value == "inner") {
					borderBlock->outer = false;
				} else {
					borderBlock->outer = true;
				}
			}

			if(borderBlock->outer) {
				if(borderBlock->to == 0) {
					has_zilch_outer_border = true;
				} else {
					has_outer_border = true;
				}
			} else {
				if(borderBlock->to == 0) {
					has_zilch_inner_border = true;
				} else {
					has_inner_border = true;
				}
			}

			for(pugi::xml_node subChildNode = childNode.first_child(); subChildNode; subChildNode = subChildNode.next_sibling()) {
				if(as_lower_str(subChildNode.name()) != "specific") {
					continue;
				}

				SpecificCaseBlock* specificCaseBlock = nullptr;
				for(pugi::xml_node superChildNode = subChildNode.first_child(); superChildNode; superChildNode = superChildNode.next_sibling()) {
					const std::string& superChildName = as_lower_str(superChildNode.name());
					if(superChildName == "conditions") {
						for(pugi::xml_node conditionChild = superChildNode.first_child(); conditionChild; conditionChild = conditionChild.next_sibling()) {
							const std::string& conditionName = as_lower_str(conditionChild.name());
							if(conditionName == "match_border") {
								if(!(attribute = conditionChild.attribute("id"))) {
									continue;
								}

								int32_t border_id = attribute.as_int();
								if(!(attribute = conditionChild.attribute("edge"))) {
									continue;
								}

								int32_t edge_id = AutoBorder::edgeNameToID(attribute.as_string());
								auto it = g_brushes.borders.find(border_id);
								if(it == g_brushes.borders.end()) {
									g_editor.Warning(wxString() << "Unknown border id in specific case match block " << border_id);
									continue;
								}

								AutoBorder* autoBorder = it->second;
								ASSERT(autoBorder != nullptr);

								uint32_t match_itemid = autoBorder->tiles[edge_id];
								if(!specificCaseBlock) {
									specificCaseBlock = newd SpecificCaseBlock();
								}
								specificCaseBlock->items_to_match.push_back(match_itemid);
							} else if(conditionName == "match_group") {
								if(!(attribute = conditionChild.attribute("group"))) {
									continue;
								}

								uint16_t group = (uint16_t)attribute.as_int();
								if(!(attribute = conditionChild.attribute("edge"))) {
									continue;
								}

								int32_t edge_id = AutoBorder::edgeNameToID(attribute.as_string());
								if(!specificCaseBlock) {
									specificCaseBlock = newd SpecificCaseBlock();
								}

								specificCaseBlock->match_group = group;
								specificCaseBlock->group_match_alignment = ::BorderType(edge_id);
								specificCaseBlock->items_to_match.push_back(group);
							} else if(conditionName == "match_item") {
								if(!(attribute = conditionChild.attribute("id"))) {
									continue;
								}

								int32_t match_itemid = attribute.as_int();
								if(!specificCaseBlock) {
									specificCaseBlock = newd SpecificCaseBlock();
								}

								specificCaseBlock->match_group = 0;
								specificCaseBlock->items_to_match.push_back(match_itemid);
							}
						}
					} else if(superChildName == "actions") {
						for(pugi::xml_node actionChild = superChildNode.first_child(); actionChild; actionChild = actionChild.next_sibling()) {
							const std::string& actionName = as_lower_str(actionChild.name());
							if(actionName == "replace_border") {
								if(!(attribute = actionChild.attribute("id"))) {
									continue;
								}

								int32_t border_id = attribute.as_int();
								if(!(attribute = actionChild.attribute("edge"))) {
									continue;
								}

								int32_t edge_id = AutoBorder::edgeNameToID(attribute.as_string());
								if(!(attribute = actionChild.attribute("with"))) {
									continue;
								}

								int32_t with_id = attribute.as_int();
								auto itt = g_brushes.borders.find(border_id);
								if(itt == g_brushes.borders.end()) {
									g_editor.Warning(wxString() << "Unknown border id in specific case match block " << border_id);
									continue;
								}

								AutoBorder* autoBorder = itt->second;
								ASSERT(autoBorder != nullptr);

								ItemType *type = GetMutableItemType(with_id);
								if(!type) {
									return false;
								}

								type->isBorder = true;
								if(!specificCaseBlock) {
									specificCaseBlock = newd SpecificCaseBlock();
								}

								specificCaseBlock->to_replace_id = autoBorder->tiles[edge_id];
								specificCaseBlock->with_id = with_id;
							} else if(actionName == "replace_item") {
								if(!(attribute = actionChild.attribute("id"))) {
									continue;
								}

								int32_t to_replace_id = attribute.as_int();
								if(!(attribute = actionChild.attribute("with"))) {
									continue;
								}

								int32_t with_id = attribute.as_int();
								ItemType *type = GetMutableItemType(with_id);
								if(!type) {
									return false;
								}

								type->isBorder = true;
								if(!specificCaseBlock) {
									specificCaseBlock = newd SpecificCaseBlock();
								}

								specificCaseBlock->to_replace_id = to_replace_id;
								specificCaseBlock->with_id = with_id;
							} else if(actionName == "delete_borders") {
								if(!specificCaseBlock) {
									specificCaseBlock = newd SpecificCaseBlock();
								}
								specificCaseBlock->delete_all = true;
							}
						}
					}
				}
			}
			borders.push_back(borderBlock);
		} else if(childName == "friend") {
			const std::string& name = childNode.attribute("name").as_string();
			if(!name.empty()) {
				if(name == "all") {
					friends.push_back(0xFFFFFFFF);
				} else {
					Brush* brush = g_brushes.getBrush(name);
					if(brush) {
						friends.push_back(brush->getID());
					} else {
						g_editor.Warning(wxString() << "Brush '" << name << "' is not defined.");
					}
				}
			}
			hate_friends = false;
		} else if(childName == "enemy") {
			const std::string& name = childNode.attribute("name").as_string();
			if(!name.empty()) {
				if(name == "all") {
					friends.push_back(0xFFFFFFFF);
				} else {
					Brush* brush = g_brushes.getBrush(name);
					if(brush) {
						friends.push_back(brush->getID());
					} else {
						g_editor.Warning(wxString() << "Brush '" << name << "' is not defined.");
					}
				}
			}
			hate_friends = true;
		} else if(childName == "clear_borders") {
			for(std::vector<BorderBlock*>::iterator it = borders.begin();
					it != borders.end();
					++it)
			{
				BorderBlock* bb = *it;
				if(bb->autoborder) {
					for(std::vector<SpecificCaseBlock*>::iterator specific_iter = bb->specific_cases.begin(); specific_iter != bb->specific_cases.end(); ++specific_iter) {
						delete *specific_iter;
					}
					if(bb->autoborder->ground) {
						delete bb->autoborder;
					}
				}
				delete bb;
			}
			borders.clear();
		} else if(childName == "clear_friends") {
			friends.clear();
			hate_friends = false;
		}
	}

	if(total_chance == 0) {
		randomize = false;
	}

	return true;
}

void GroundBrush::undraw(Map *map, Tile* tile)
{
	ASSERT(tile);
	tile->removeItems([this](const Item *item){ return item->getGroundBrush() == this; });
}

void GroundBrush::draw(Map *map, Tile* tile, void* parameter)
{
	ASSERT(tile);
	if(border_items.empty()) return;

	if(parameter != nullptr) {
		std::pair<bool, GroundBrush*>& param = *reinterpret_cast<std::pair<bool, GroundBrush*>* >(parameter);
		GroundBrush* other = tile->getGroundBrush();
		if(param.first) { // Volatile? :)
			if(other != nullptr) {
				return;
			}
		} else if(other != param.second) {
			return;
		}
	}
	int chance = random(1, total_chance);
	uint16_t id = 0;
	for(std::vector<ItemChanceBlock>::const_iterator it = border_items.begin(); it != border_items.end(); ++it) {
		if(chance < it->chance) {
			id = it->id;
			break;
		}
	}
	if(id == 0) {
		id = border_items.front().id;
	}

	tile->addItem(Item::Create(id));
}

const GroundBrush::BorderBlock* GroundBrush::getBrushTo(GroundBrush* first, GroundBrush* second) {
	//printf("Border from %s to %s : ", first->getName().c_str(), second->getName().c_str());
	if(first) {
		if(second) {
			if(first->getZ() < second->getZ() && second->hasOuterBorder()) {
				if(first->hasInnerBorder()) {
					for(std::vector<BorderBlock*>::iterator it = first->borders.begin(); it != first->borders.end(); ++it) {
						BorderBlock* bb = *it;
						if(bb->outer) {
							continue;
						} else if(bb->to == second->getID() || bb->to == 0xFFFFFFFF) {
							//printf("%d\n", bb->autoborder);
							return bb;
						}
					}
				}
				for(std::vector<BorderBlock*>::iterator it = second->borders.begin(); it != second->borders.end(); ++it) {
					BorderBlock* bb = *it;
					if(!bb->outer) {
						continue;
					} else if(bb->to == first->getID()) {
						//printf("%d\n", bb->autoborder);
						return bb;
					} else if(bb->to == 0xFFFFFFFF) {
						//printf("%d\n", bb->autoborder);
						return bb;
					}
				}
			} else if(first->hasInnerBorder()) {
				for(std::vector<BorderBlock*>::iterator it = first->borders.begin(); it != first->borders.end(); ++it) {
					BorderBlock* bb = *it;
					if(bb->outer) {
						continue;
					} else if(bb->to == second->getID()) {
						//printf("%d\n", bb->autoborder);
						return bb;
					} else if(bb->to == 0xFFFFFFFF) {
						//printf("%d\n", bb->autoborder);
						return bb;
					}
				}
			}
		} else if(first->hasInnerZilchBorder()) {
			for(std::vector<BorderBlock*>::iterator it = first->borders.begin(); it != first->borders.end(); ++it) {
				BorderBlock* bb = *it;
				if(bb->outer) {
					continue;
				} else if(bb->to == 0) {
					//printf("%d\n", bb->autoborder);
					return bb;
				}
			}
		}
	} else if(second && second->hasOuterZilchBorder()) {
		for(std::vector<BorderBlock*>::iterator it = second->borders.begin(); it != second->borders.end(); ++it) {
			BorderBlock* bb = *it;
			if(!bb->outer) {
				continue;
			} else if(bb->to == 0) {
				//printf("%d\n", bb->autoborder);
				return bb;
			}
		}
	}
	//printf("None\n");
	return nullptr;
}

static GroundBrush *extractGroundBrushFromTile(Map *map, uint32_t x, uint32_t y, uint32_t z){
	GroundBrush *brush = NULL;
	if(Tile* tile = map->getTile(x, y, z)) {
		brush = tile->getGroundBrush();
	}
	return brush;
};

void GroundBrush::doBorders(Map *map, Tile* tile)
{
	ASSERT(tile);

	uint32_t x = tile->pos.x;
	uint32_t y = tile->pos.y;
	uint32_t z = tile->pos.z;

	// Pair of visited / what border type
	std::pair<bool, GroundBrush*> neighbours[8];
	neighbours[0] = { false, extractGroundBrushFromTile(map, x - 1, y - 1, z) };
	neighbours[1] = { false, extractGroundBrushFromTile(map, x,     y - 1, z) };
	neighbours[2] = { false, extractGroundBrushFromTile(map, x + 1, y - 1, z) };
	neighbours[3] = { false, extractGroundBrushFromTile(map, x - 1, y,     z) };
	neighbours[4] = { false, extractGroundBrushFromTile(map, x + 1, y,     z) };
	neighbours[5] = { false, extractGroundBrushFromTile(map, x - 1, y + 1, z) };
	neighbours[6] = { false, extractGroundBrushFromTile(map, x,     y + 1, z) };
	neighbours[7] = { false, extractGroundBrushFromTile(map, x + 1, y + 1, z) };

	static std::vector<const BorderBlock*> specificList;
	specificList.clear();

	std::vector<BorderCluster> borderList;
	GroundBrush *borderBrush = tile->getGroundBrush();
	for(int32_t i = 0; i < 8; ++i) {
		auto& neighbourPair = neighbours[i];
		if(neighbourPair.first) {
			continue;
		}

		//printf("Checking neighbour #%d\n", i);
		//printf("\tNeighbour not checked before\n");

		GroundBrush* other = neighbourPair.second;
		if(borderBrush) {
			if(other) {
				//printf("\tNeighbour has brush\n");
				if(other->getID() == borderBrush->getID()) {
					//printf("\tNeighbour has same brush as we\n");
					continue;
				}

				if(other->hasOuterBorder() || borderBrush->hasInnerBorder()) {
					bool only_mountain = false;
					if(/*!borderBrush->hasInnerBorder() && */(other->friendOf(borderBrush) || borderBrush->friendOf(other))) {
						if(!other->hasOptionalBorder()) {
							continue;
						}
						only_mountain = true;
					}

					uint32_t tiledata = 0;
					for(int32_t j = i; j < 8; ++j) {
						auto& otherPair = neighbours[j];
						if(!otherPair.first && otherPair.second && otherPair.second->getID() == other->getID()) {
							otherPair.first = true;
							tiledata |= 1 << j;
						}
					}

					if(tiledata != 0) {
						// Add mountain if appropriate!
						if(other->hasOptionalBorder() && tile->hasOptionalBorder()) {
							BorderCluster borderCluster;
							borderCluster.alignment = tiledata;
							borderCluster.z = 0x7FFFFFFF; // Above all other borders
							borderCluster.border = other->optional_border;

							borderList.push_back(borderCluster);
							if(other->useSoloOptionalBorder()) {
								only_mountain = true;
							}
						}

						if(!only_mountain) {
							const BorderBlock* borderBlock = getBrushTo(borderBrush, other);
							if(borderBlock) {
								bool found = false;
								for(BorderCluster& borderCluster : borderList) {
									if(borderCluster.border == borderBlock->autoborder) {
										borderCluster.alignment |= tiledata;
										if(borderCluster.z < other->getZ()) {
											borderCluster.z = other->getZ();
										}

										if(!borderBlock->specific_cases.empty()) {
											if(std::find(specificList.begin(), specificList.end(), borderBlock) == specificList.end()) {
												specificList.push_back(borderBlock);
											}
										}

										found = true;
										break;
									}
								}

								if(!found) {
									BorderCluster borderCluster;
									borderCluster.alignment = tiledata;
									borderCluster.z = other->getZ();
									borderCluster.border = borderBlock->autoborder;

									borderList.push_back(borderCluster);
									if(!borderBlock->specific_cases.empty()) {
										if(std::find(specificList.begin(), specificList.end(), borderBlock) == specificList.end()) {
											specificList.push_back(borderBlock);
										}
									}
								}
							}
						}
					}
				}
			} else if(borderBrush->hasInnerZilchBorder()) {
				// Border against nothing (or undefined tile)
				uint32_t tiledata = 0;
				for(int32_t j = i; j < 8; ++j) {
					auto& otherPair = neighbours[j];
					if(!otherPair.first && !otherPair.second) {
						otherPair.first = true;
						tiledata |= 1 << j;
					}
				}

				if(tiledata != 0) {
					const BorderBlock* borderBlock = getBrushTo(borderBrush, nullptr);
					if(!borderBlock) {
						continue;
					}

					if(borderBlock->autoborder) {
						BorderCluster borderCluster;
						borderCluster.alignment = tiledata;
						borderCluster.z = 5000;
						borderCluster.border = borderBlock->autoborder;

						borderList.push_back(borderCluster);
					}

					if(!borderBlock->specific_cases.empty()) {
						if(std::find(specificList.begin(), specificList.end(), borderBlock) == specificList.end()) {
							specificList.push_back(borderBlock);
						}
					}
				}
				continue;
			}
		} else if(other && other->hasOuterZilchBorder()) {
			// Border against nothing (or undefined tile)
			uint32_t tiledata = 0;
			for(int32_t j = i; j < 8; ++j) {
				auto& otherPair = neighbours[j];
				if(!otherPair.first && otherPair.second && otherPair.second->getID() == other->getID()) {
					otherPair.first = true;
					tiledata |= 1 << j;
				}
			}

			if(tiledata != 0) {
				const BorderBlock* borderBlock = getBrushTo(nullptr, other);
				if(borderBlock) {
					if(borderBlock->autoborder) {
						BorderCluster borderCluster;
						borderCluster.alignment = tiledata;
						borderCluster.z = other->getZ();
						borderCluster.border = borderBlock->autoborder;

						borderList.push_back(borderCluster);
					}

					if(!borderBlock->specific_cases.empty()) {
						if(std::find(specificList.begin(), specificList.end(), borderBlock) == specificList.end()) {
							specificList.push_back(borderBlock);
						}
					}
				}

				// Add mountain if appropriate!
				if(other->hasOptionalBorder() && tile->hasOptionalBorder()) {
					BorderCluster borderCluster;
					borderCluster.alignment = tiledata;
					borderCluster.z = 0x7FFFFFFF; // Above other zilch borders
					borderCluster.border = other->optional_border;

					borderList.push_back(borderCluster);
				} else {
					tile->setOptionalBorder(false);
				}
			}
		}
		// Check tile as done
		neighbourPair.first = true;
	}

	std::sort(borderList.begin(), borderList.end());
	tile->removeBorders();

	while(!borderList.empty()) {
		BorderCluster& borderCluster = borderList.back();
		if(!borderCluster.border) {
			borderList.pop_back();
			continue;
		}

		BorderType directions[4] = {
			static_cast<BorderType>((border_types[borderCluster.alignment] & 0x000000FF) >> 0),
			static_cast<BorderType>((border_types[borderCluster.alignment] & 0x0000FF00) >> 8),
			static_cast<BorderType>((border_types[borderCluster.alignment] & 0x00FF0000) >> 16),
			static_cast<BorderType>((border_types[borderCluster.alignment] & 0xFF000000) >> 24)
		};

		for(int32_t i = 0; i < 4; ++i) {
			BorderType direction = directions[i];
			if(direction == BORDER_NONE) {
				break;
			}

			if(borderCluster.border->tiles[direction]) {
				tile->addItem(Item::Create(borderCluster.border->tiles[direction]));
			} else {
				if(direction == NORTHWEST_DIAGONAL) {
					tile->addItem(Item::Create(borderCluster.border->tiles[WEST_HORIZONTAL]));
					tile->addItem(Item::Create(borderCluster.border->tiles[NORTH_HORIZONTAL]));
				} else if(direction == NORTHEAST_DIAGONAL) {
					tile->addItem(Item::Create(borderCluster.border->tiles[EAST_HORIZONTAL]));
					tile->addItem(Item::Create(borderCluster.border->tiles[NORTH_HORIZONTAL]));
				} else if(direction == SOUTHWEST_DIAGONAL) {
					tile->addItem(Item::Create(borderCluster.border->tiles[SOUTH_HORIZONTAL]));
					tile->addItem(Item::Create(borderCluster.border->tiles[WEST_HORIZONTAL]));
				} else if(direction == SOUTHEAST_DIAGONAL) {
					tile->addItem(Item::Create(borderCluster.border->tiles[SOUTH_HORIZONTAL]));
					tile->addItem(Item::Create(borderCluster.border->tiles[EAST_HORIZONTAL]));
				}
			}
		}

		borderList.pop_back();
	}

	for(const BorderBlock* borderBlock : specificList) {
		for(const SpecificCaseBlock* specificCaseBlock : borderBlock->specific_cases) {
			/*
			printf("New round\n");
			if(specificCaseBlock->to_replace_id == 0) {
				continue;
			}

			if(specificCaseBlock->with_id == 0) {
				continue;
			}
			*/
			uint32_t matches = 0;
			for(Item *item = tile->items; item != NULL; item = item->next){
				if(!item->getFlag(CLIP)) {
					break;
				}

				if(specificCaseBlock->match_group > 0) {
					//printf("Matching %d == %d : %d == %d\n", item->getBorderGroup(), specificCaseBlock->match_group, item->getBorderAlignment(), specificCaseBlock->group_match_alignment);
					if(item->getBorderGroup() == specificCaseBlock->match_group && item->getBorderAlignment() == specificCaseBlock->group_match_alignment) {
						//printf("Successfully matched %d == %d : %d == %d\n", item->getBorderGroup(), specificCaseBlock->match_group, item->getBorderAlignment(), specificCaseBlock->group_match_alignment);
						++matches;
						continue;
					}
				}

				//printf("\tInvestigating first item id:%d\n", item->getID());
				for(uint16_t matchId : specificCaseBlock->items_to_match) {
					if(item->getID() == matchId) {
						//printf("\t\tMatched item id %d\n", item->getID());
						++matches;
					}
				}
			}

			//printf("\t\t%d matches of %d\n", matches, scb->items_to_match.size());
			if(matches == specificCaseBlock->items_to_match.size()) {
				if(specificCaseBlock->delete_all) {
					// Delete all matching borders
					tile->removeItems(
						[specificCaseBlock](const Item *item){
							if(item->getFlag(CLIP)){
								for(uint16_t matchId: specificCaseBlock->items_to_match){
									if(item->getID() == matchId){
										return true;
									}
								}
							}
							return false;
						});
				} else {
					// All matched, replace!
					for(Item *item = tile->items; item != NULL; item = item->next){
						if(!item->getFlag(CLIP)){
							return;
						}

						if(item->getID() == specificCaseBlock->to_replace_id){
							item->transform(specificCaseBlock->with_id);
							return;
						}
					}
				}
			}
		}
	}
}
