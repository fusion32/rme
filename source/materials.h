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

#ifndef RME_MATERIALS_H_
#define RME_MATERIALS_H_

#include "main.h"
#include "tileset.h"

class Materials {
public:
	Materials();
	~Materials();

	void clear();

	TilesetContainer tilesets;

	bool loadMaterials(const wxString &projectDir);
	void createOtherTileset();

	bool isInTileset(const Item* item, std::string tileset) const;
	bool isInTileset(Brush* brush, std::string tileset) const;

protected:
	bool loadMaterialsInternal(const wxString &filename);
	bool unserializeMaterials(const wxString &filename, pugi::xml_node node);
	bool unserializeTileset(pugi::xml_node node);

private:
	Materials(const Materials&);
	Materials& operator=(const Materials&);
};

extern Materials g_materials;

#endif
