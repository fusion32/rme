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

#include "editor.h"
#include "materials.h"
#include "brush.h"
#include "creature.h"
#include "creature_brush.h"
#include "script.h"

#include <wx/dir.h>

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
	return 1;
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

static Outfit ReadOutfit(Script *script){
	Outfit outfit = {};
	script->readSymbol('(');
	outfit.lookType = script->readNumber();
	script->readSymbol(',');
	if(outfit.lookType != 0){
		const uint8_t *colors = script->readBytes();
		outfit.lookHead = (int)colors[0];
		outfit.lookBody = (int)colors[1];
		outfit.lookLegs = (int)colors[2];
		outfit.lookFeet = (int)colors[3];
	}else{
		outfit.lookItem = script->readNumber();
	}
	script->readSymbol(')');
	return outfit;
}

static bool LoadCreatureType(const wxString &filename){
	Script script(filename.mb_str());

	// NOTE(fusion): Make sure we don't attempt to parse anything if there was
	// an error while opening the file. This is a bit different because some
	// of the data is parsed outside the parsing loop.
	if(!script.eof()){
		if(strcmp(script.readIdentifier(), "racenumber") != 0){
			g_editor.Error("Excepted race number as the first attribute");
			return false;
		}

		script.readSymbol('=');
		int raceId = script.readNumber();
		if(raceId <= 0 || raceId > 1024){
			g_editor.Error(wxString() << "Illegal race number " << raceId);
			return false;
		}

		if(CreatureTypeExists(raceId)){
			g_editor.Error(wxString() << "Race " << raceId << " already defined");
			return false;
		}

		CreatureType *type = GetOrCreateCreatureType(raceId);
		type->outfit.lookType = raceId;

		std::string ident;
		while(true){
			script.nextToken();
			if(script.eof()){
				break;
			}

			ident = script.getIdentifier();
			script.readSymbol('=');
			if(ident == "name"){
				type->name = script.readString();
			}else if(ident == "article"){ // unused
				script.readString();
			}else if(ident == "outfit"){
				type->outfit = ReadOutfit(&script);
			}else if(ident == "corpse"){ // unused
				script.readNumber();
			}else if(ident == "corpses"){ // unused
				script.readNumber();
				script.readSymbol(',');
				script.readNumber();
			}else if(ident == "blood"){ // unused
				script.readIdentifier();
			}else if(ident == "experience"){ // unused
				script.readNumber();
			}else if(ident == "summoncost"){ // unused
				script.readNumber();
			}else if(ident == "fleethreshold"){ // unused
				script.readNumber();
			}else if(ident == "attack"){ // unused
				script.readNumber();
			}else if(ident == "defend"){ // unused
				script.readNumber();
			}else if(ident == "armor"){ // unused
				script.readNumber();
			}else if(ident == "poison"){ // unused
				script.readNumber();
			}else if(ident == "losetarget"){ // unused
				script.readNumber();
			}else if(ident == "strategy"){ // unused
				script.readSymbol('(');
				script.readNumber(); script.readSymbol(',');
				script.readNumber(); script.readSymbol(',');
				script.readNumber(); script.readSymbol(',');
				script.readNumber(); script.readSymbol(')');
			}else if(ident == "flags"){ // unused
				script.readSymbol('{');
				do{
					script.readIdentifier();
				}while(script.readSpecial() != '}');
			}else if(ident == "skills"){ // unused
				script.readSymbol('{');
				do{
					script.readSymbol('(');
					script.readIdentifier(); script.readSymbol(','); // name
					script.readNumber(); script.readSymbol(','); // level
					script.readNumber(); script.readSymbol(','); // minimum
					script.readNumber(); script.readSymbol(','); // maximum
					script.readNumber(); script.readSymbol(','); // next level
					script.readNumber(); script.readSymbol(','); // factor percent
					script.readNumber(); script.readSymbol(')'); // add level
				}while(script.readSpecial() != '}');
			}else if(ident == "talk"){ // unused
				script.readSymbol('{');
				do{
					script.readString();
				}while(script.readSpecial() != '}');
			}else if(ident == "inventory"){ // unused
				script.readSymbol('{');
				do{
					script.readSymbol('(');
					script.readNumber(); script.readSymbol(','); // type
					script.readNumber(); script.readSymbol(','); // maximum
					script.readNumber(); script.readSymbol(')'); // probability
				}while(script.readSpecial() != '}');
			}else if(ident == "spells"){ // unused
				script.readSymbol('{');
				do{
					{ // Spell Shape
						std::string_view spellShape = script.readIdentifier();
						if(spellShape == "actor"){
							script.readSymbol('(');
							script.readNumber(); script.readSymbol(')');
						}else if(spellShape == "victim"){
							script.readSymbol('(');
							script.readNumber(); script.readSymbol(',');
							script.readNumber(); script.readSymbol(',');
							script.readNumber(); script.readSymbol(')');
						}else if(spellShape == "origin"){
							script.readSymbol('(');
							script.readNumber(); script.readSymbol(',');
							script.readNumber(); script.readSymbol(')');
						}else if(spellShape == "destination"){
							script.readSymbol('(');
							script.readNumber(); script.readSymbol(',');
							script.readNumber(); script.readSymbol(',');
							script.readNumber(); script.readSymbol(',');
							script.readNumber(); script.readSymbol(')');
						}else if(spellShape == "angle"){
							script.readSymbol('(');
							script.readNumber(); script.readSymbol(',');
							script.readNumber(); script.readSymbol(',');
							script.readNumber(); script.readSymbol(')');
						}else{
							script.error("unknown spell shape");
						}
					}

					script.readSymbol('I');

					{ // Spell Impact
						std::string_view spellImpact = script.readIdentifier();
						if(spellImpact == "damage"){
							script.readSymbol('(');
							script.readNumber(); script.readSymbol(',');
							script.readNumber(); script.readSymbol(',');
							script.readNumber(); script.readSymbol(')');
						}else if(spellImpact == "field"){
							script.readSymbol('(');
							script.readNumber(); script.readSymbol(')');
						}else if(spellImpact == "healing"){
							script.readSymbol('(');
							script.readNumber(); script.readSymbol(',');
							script.readNumber(); script.readSymbol(')');
						}else if(spellImpact == "speed"){
							script.readSymbol('(');
							script.readNumber(); script.readSymbol(',');
							script.readNumber(); script.readSymbol(',');
							script.readNumber(); script.readSymbol(')');
						}else if(spellImpact == "drunken"){
							script.readSymbol('(');
							script.readNumber(); script.readSymbol(',');
							script.readNumber(); script.readSymbol(',');
							script.readNumber(); script.readSymbol(')');
						}else if(spellImpact == "strength"){
							script.readSymbol('(');
							script.readNumber(); script.readSymbol(',');
							script.readNumber(); script.readSymbol(',');
							script.readNumber(); script.readSymbol(',');
							script.readNumber(); script.readSymbol(')');
						}else if(spellImpact == "outfit"){
							script.readSymbol('(');
							ReadOutfit(&script); script.readSymbol(',');
							script.readNumber(); script.readSymbol(')');
						}else if(spellImpact == "summon"){
							script.readSymbol('(');
							script.readNumber(); script.readSymbol(',');
							script.readNumber(); script.readSymbol(')');
						}else{
							script.error("unknown spell impact");
						}
					}

					script.readSymbol(':');

					// Spell Delay
					if(script.readNumber() == 0){
						script.error("zero spell delay");
					}

				}while(script.readSpecial() != '}');
			}else{
				script.error("unknown race property");
			}
		}
	}

	if(const char *error = script.getError()){
		g_editor.Error(error);
		return false;
	}

	return true;
}

bool LoadCreatureTypes(const wxString &projectDir){
	wxString monsterDir = ConcatPath(projectDir, "mon");
	if(!wxDir::Exists(monsterDir)){
		g_editor.Error("Unable to locate monster directory");
		return false;
	}

	wxString filename;
	wxDir dir(monsterDir);
	if(dir.GetFirst(&filename, "*.mon", wxDIR_FILES)){
		do{
			wxFileName fn(monsterDir, filename);
			if(!LoadCreatureType(fn.GetFullPath())){
				g_editor.Error(wxString() << "Unable to load monster " << filename);
				return false;
			}
		}while(dir.GetNext(&filename));
	}

	return true;
}

void ClearCreatureTypes(void){
	g_creatureTypes.clear();
}

