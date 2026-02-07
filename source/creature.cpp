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

#include "editor.h"
#include "materials.h"
#include "brush.h"
#include "creature.h"
#include "creature_brush.h"

static std::vector<CreatureType> g_creatureTypes;

Creature *Creature::deepCopy() const
{
	return newd Creature(*this);
}

const std::string &Creature::getName(void) const
{
	return GetCreatureType(raceId).name;
}

Outfit Creature::getOutfit(void) const
{
	return GetCreatureType(raceId).outfit;
}

CreatureBrush *Creature::getBrush(void) const
{
	return GetCreatureType(raceId).brush;
}

int GetMinRaceId(void){
	return 0;
}

int GetMaxRaceId(void){
	return (int)g_creatureTypes.size() - 1;
}

bool CreatureTypeExists(int raceId){
	return raceId >= GetMinRaceId()
		&& raceId <= GetMaxRaceId()
		&& g_creatureTypes[raceId].raceId == raceId;
}

const CreatureType &GetCreatureType(int raceId){
	static const CreatureType dummy = {};
	if(CreatureTypeExists(raceId)){
		return g_creatureTypes[raceId];
	}else{
		return dummy;
	}
}

CreatureType *GetMutableCreatureType(int raceId){
	if(CreatureTypeExists(raceId)){
		return &g_creatureTypes[raceId];
	}else{
		return NULL;
	}
}

static CreatureType *GetOrCreateCreatureType(int raceId){
	// NOTE(fusion): Same thing as `GetOrCreateItemType`.
	constexpr size_t MIN_CAPACITY = 1024;
	constexpr size_t MAX_CAPACITY = UINT16_MAX + 1;
	size_t requiredSize = raceId + 1;
	if(requiredSize > g_creatureTypes.size()){
		size_t capacity = g_creatureTypes.capacity();
		if(requiredSize > capacity){
			if(capacity < MIN_CAPACITY)    capacity = MIN_CAPACITY;
			while(requiredSize > capacity) capacity += capacity / 2;
			if(capacity > MAX_CAPACITY)    capacity = MAX_CAPACITY;
			ASSERT(requiredSize <= capacity);
			g_creatureTypes.reserve(capacity);
		}
		g_creatureTypes.resize(requiredSize);
	}
	g_creatureTypes[raceId].raceId = raceId;
	return &g_creatureTypes[raceId];
}

bool LoadCreatureTypes(const wxString &projectDir, wxString &outError, wxArrayString &outWarnings)
{
	wxString filename;
	{
		wxPathList paths;
		paths.Add(projectDir);
		paths.Add(projectDir + "editor");
		filename = paths.FindValidPath("creatures.xml");
		if(filename.IsEmpty()){
			outError << "Unable to locate creatures.xml";
			return false;
		}
	}

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(filename.mb_str());
	if(!result) {
		outError << "Couldn't open file \"" << filename << "\":" << result.description();
		return false;
	}

	pugi::xml_node node = doc.child("creatures");
	if(!node) {
		outError << "Creatures file is missing \"creatures\" top-level node.";
		return false;
	}

	for(pugi::xml_node creatureNode: node.children("creature")){
		// TODO(fusion): Npcs store their spawn point in their own files so we
		// might need to review this later on, to be able to support NPCs.
		pugi::xml_attribute raceAttr = creatureNode.attribute("race");
		if(!raceAttr){
			outWarnings.push_back(wxString() << filename
					<< ":" << creatureNode.offset_debug()
					<< ": Missing creature \"race\" attribute...");
			continue;
		}

		int raceId = raceAttr.as_int();
		if(raceId < 0 || raceId > 1024){
			outWarnings.push_back(wxString() << filename
					<< ":" << creatureNode.offset_debug()
					<< ": Invalid creature \"race\" " << raceId);
			continue;
		}

		CreatureType *creatureType = GetOrCreateCreatureType(raceId);

		if(pugi::xml_attribute attr = creatureNode.attribute("name")){
			creatureType->name = attr.as_string();
		}else{
			creatureType->name = "Unnamed";
		}

		if(pugi::xml_attribute attr = creatureNode.attribute("looktype")){
			creatureType->outfit.lookType = attr.as_int();
			if(g_editor.gfx.getCreatureSprite(creatureType->outfit.lookType) == NULL){
				outWarnings.push_back(wxString() << filename
						  << ":" << creatureNode.offset_debug()
						  << ": Invalid creature look type "
						  << creatureType->outfit.lookType);
			}
		}else if(pugi::xml_attribute attr = creatureNode.attribute("lookitem")){
			creatureType->outfit.lookType = 0;
			creatureType->outfit.lookItem = attr.as_int();
			if(g_editor.gfx.getSprite(creatureType->outfit.lookItem) == NULL){
				outWarnings.push_back(wxString() << filename
						  << ":" << creatureNode.offset_debug()
						  << ": Invalid creature look item "
						  << creatureType->outfit.lookItem);
			}
		}

		if(pugi::xml_attribute attr = creatureNode.attribute("lookhead")){
			creatureType->outfit.lookHead = attr.as_int();
		}

		if(pugi::xml_attribute attr = creatureNode.attribute("lookbody")){
			creatureType->outfit.lookBody = attr.as_int();
		}

		if(pugi::xml_attribute attr = creatureNode.attribute("looklegs")){
			creatureType->outfit.lookLegs = attr.as_int();
		}

		if(pugi::xml_attribute attr = creatureNode.attribute("lookfeet")){
			creatureType->outfit.lookFeet = attr.as_int();
		}

		if(pugi::xml_attribute attr = creatureNode.attribute("lookaddon")){
			creatureType->outfit.lookAddon = attr.as_int();
		}

		if(pugi::xml_attribute attr = creatureNode.attribute("lookmount")){
			creatureType->outfit.lookMount = attr.as_int();
		}
	}
	return true;
}

void ClearCreatureTypes(void){
	g_creatureTypes.clear();
}

