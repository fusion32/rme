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

#include "map.h"
#include "editor.h"
#include "script.h"

#include <wx/dir.h>

static Item *LoadObjects(Script *script){
	Item *items = NULL;
	Item **tail = &items;
	Item *currentItem = NULL;
	script->readSymbol('{');
	script->nextToken();
	while(!script->eof()){
		if(currentItem == NULL){
			if(script->token.kind != TOKEN_SPECIAL){
				int typeId = script->getNumber();
				if(!ItemTypeExists(typeId)){
					script->error("invalid object type");
					break;
				}

				currentItem = newd Item(typeId);
				*tail = currentItem;
				tail = &currentItem->next;
			}else{
				int special = script->getSpecial();
				if(special == '}') break;
				if(special != ','){
					script->error("expected comma");
					break;
				}
			}
			script->nextToken();
		}else{
			if(script->token.kind != TOKEN_SPECIAL){
				int attr = GetInstanceAttributeByName(script->getIdentifier());
				if(attr == -1){
					script->error("unknown attribute");
					break;
				}

				script->readSymbol('=');
				if(attr == CONTENT){
					// NOTE(fusion): Just to be sure...
					Item **contentTail = &currentItem->content;
					while(*contentTail){
						contentTail = &(*contentTail)->next;
					}

					*contentTail = LoadObjects(script);
				}else if(attr == TEXTSTRING || attr == EDITOR){
					currentItem->setTextAttribute((ObjectInstanceAttribute)attr, script->readString());
				}else{
					currentItem->setAttribute((ObjectInstanceAttribute)attr, script->readNumber());
				}

				script->nextToken();
			}else{
				// NOTE(fusion): Attributes are key-value pairs separated by space.
				// If we find a special token (probably ',' or '}'), then we're done
				// parsing attributes for the current object.
				currentItem = NULL;
			}
		}
	}
	return items;
}

bool Map::loadSector(const wxString &filename, int sectorX, int sectorY, int sectorZ,
					 wxString &outError, wxArrayString &outWarnings){
	if(!SectorValid(sectorX, sectorY, sectorZ)){
		outError << "Invalid sector coordinates "
				<< sectorX << "-" << sectorY << "-" << sectorZ;
		return false;
	}

	Tile *tile = NULL;
	std::string ident;
	Script script(filename.mb_str());
	MapSector *sector = getOrCreateSectorAt(
			sectorX * MAP_SECTOR_SIZE,
			sectorY * MAP_SECTOR_SIZE,
			sectorZ);
	while(true){
		script.nextToken();
		if(script.eof()){
			break;
		}

		if(script.token.kind == TOKEN_SPECIAL && script.getSpecial() == ','){
			continue;
		}

		if(script.token.kind == TOKEN_BYTES){
			const uint8_t *offset = script.getBytes();
			int offsetX = (int)offset[0];
			int offsetY = (int)offset[1];
			tile = sector->getTile(offsetX, offsetY);
			if(tile == NULL){
				script.error("invalid sector offset");
				break;
			}
			script.readSymbol(':');
		}else if(script.token.kind == TOKEN_IDENTIFIER){
			if(tile == NULL){
				script.error("coordinate expected");
				break;
			}

			ident = script.getIdentifier();
			if(ident == "refresh"){
				tile->setTileFlag(TILE_FLAG_REFRESH);
			}else if(ident == "nologout"){
				tile->setTileFlag(TILE_FLAG_NOLOGOUT);
			}else if(ident == "protectionzone"){
				tile->setTileFlag(TILE_FLAG_PROTECTIONZONE);
			}else if(ident == "content"){
				script.readSymbol('=');
				tile->addItems(LoadObjects(&script));
			}else{
				script.error("unknown map flag");
				break;
			}

		}else{
			script.error("next map point expected");
			break;
		}

	}

	if(const char *error = script.getError()){
		outError << error;
		return false;
	}

	return true;
}

bool Map::load(const wxString &projectDir, wxString &outError, wxArrayString &outWarnings)
{
	// TODO(fusion): Not sure about creating the directory and whether we should
	// attempt to locate some directory with .sec files in it.

	wxString mapDirAttempt = projectDir + "origmap";
	if(!wxDir::Exists(mapDirAttempt)){
		if(!wxDir::Make(mapDirAttempt)){
			outError << "Unable to create new map directory";
			return false;
		}

		outWarnings.push_back(wxString()
				<< "Unable to locate existing map directory so "
				<< mapDirAttempt << " was created");
	}

	wxString saveDirAttempt = projectDir + "save";
	if(!wxDir::Exists(saveDirAttempt)){
		if(!wxDir::Make(saveDirAttempt)){
			outError << "Unable to locate nor create save directory";
			return false;
		}

		outWarnings.push_back(wxString()
				<< "Unable to locate existing save directory so "
				<< saveDirAttempt << " was created");
	}

	if(g_editor.HasLoadingBar()){
		g_editor.SetLoadDone(0, "Discovering sector files...");
	}

	int numSectors = 0;
	wxString filename;
	wxDir dir(mapDirAttempt);
	if(dir.GetFirst(&filename, "*.sec")){
		do{
			numSectors += 1;
		}while(dir.GetNext(&filename));
	}

	int numLoaded = 0;
	if(dir.GetFirst(&filename, "*.sec")){
		do{
			int sectorX, sectorY, sectorZ;
			if(sscanf(filename.mb_str(), "%d-%d-%d.sec", &sectorX, &sectorY, &sectorZ) != 3){
				//outWarnings.push_back(wxString()
				//		<< "Non sector file " << filename << " in map directory " << mapDirAttempt);
				numSectors -= 1;
				continue;
			}

			if(g_editor.HasLoadingBar()){
				g_editor.SetLoadDone((numLoaded * 100 / numSectors),
						wxString::Format("Loading sector %s (%d/%d)",
							filename, numLoaded, numSectors));
			}


			FileName fn(mapDirAttempt, filename);
			if(!loadSector(fn.GetFullPath(), sectorX, sectorY, sectorZ, outError, outWarnings)){
				outError.Prepend(wxString() << "Unable to load sector " << filename << ": ");
				return false;
			}

			numLoaded += 1;
		}while(dir.GetNext(&filename));
	}

	mapDir = std::move(mapDir);
	saveDir = std::move(saveDirAttempt);
	return true;
}

bool Map::save(void)
{
	if(mapDir.IsEmpty() || saveDir.IsEmpty()){
		return false;
	}

	// TODO
	return false;
}

void Map::clear(void)
{
	mapDir.Clear();
	saveDir.Clear();
	minSectorX = 0;
	minSectorY = 0;
	minSectorZ = 0;
	maxSectorX = 0;
	maxSectorY = 0;
	maxSectorZ = 0;
	sectors.clear();
	dirtySectors.clear();
}

MapSector *Map::getSectorAt(int x, int y, int z){
	if(!PositionValid(x, y, z)){
		return NULL;
	}

	uint32_t sectorId = GetMapSectorId(x, y, z);
	auto it = sectors.find(sectorId);
	if(it != NULL){
		return &it->second;
	}else{
		return NULL;
	}
}

MapSector *Map::getOrCreateSectorAt(int x, int y, int z){
	if(!PositionValid(x, y, z)){
		// NOTE(fusion): This is just to make sure we don't return a NULL
		static MapSector outOfBounder;
		return &outOfBounder;
	}

	uint32_t sectorId = GetMapSectorId(x, y, z);
	auto ret = sectors.try_emplace(sectorId);
	if(ret.second){
		int sectorX = x / MAP_SECTOR_SIZE;
		int sectorY = y / MAP_SECTOR_SIZE;
		int sectorZ = z;

		if(sectors.size() > 1){
			if(sectorX < minSectorX){
				minSectorX = sectorX;
			}else if(sectorX > maxSectorX){
				maxSectorX = sectorX;
			}

			if(sectorY < minSectorY){
				minSectorY = sectorY;
			}else if(sectorY > maxSectorY){
				maxSectorY = sectorY;
			}

			if(sectorZ < minSectorZ){
				minSectorZ = sectorZ;
			}else if(sectorZ > maxSectorZ){
				maxSectorZ = sectorZ;
			}
		}else{
			minSectorX = sectorX;
			minSectorY = sectorY;
			minSectorZ = sectorZ;
			maxSectorX = sectorX;
			maxSectorY = sectorY;
			maxSectorZ = sectorZ;
		}

		ret.first->second.setTilePositions(
				sectorX * MAP_SECTOR_SIZE,
				sectorY * MAP_SECTOR_SIZE,
				sectorZ);
	}

	return &ret.first->second;
}

Tile *Map::getTile(int x, int y, int z)
{
	if(MapSector *sector = getSectorAt(x, y, z)){
		int offsetX = x & MAP_SECTOR_MASK;
		int offsetY = y & MAP_SECTOR_MASK;
		return sector->getTile(offsetX, offsetY);
	}else{
		return NULL;
	}
}

Tile *Map::getOrCreateTile(int x, int y, int z)
{
	MapSector *sector = getOrCreateSectorAt(x, y, z);
	ASSERT(sector != NULL);
	int offsetX = x & MAP_SECTOR_MASK;
	int offsetY = y & MAP_SECTOR_MASK;
	return sector->getTile(offsetX, offsetY);
}

void Map::cleanInvalidTiles(bool showDialog /*= false*/)
{
	if(showDialog)
		g_editor.CreateLoadBar("Removing invalid tiles...");

	double nextUpdate = 0.0;
	removeItems(
		[&nextUpdate, showDialog](const Item *item, double progress){
			if(showDialog && progress >= nextUpdate){
				g_editor.SetLoadDone((int)(progress * 100.0));
				nextUpdate = progress + 0.01;
			}
			return !ItemTypeExists(item->getID());
		});

	if(showDialog)
		g_editor.DestroyLoadBar();
}

bool Map::exportMinimap(const FileName &filename,
						int floor /*= rme::MapGroundLayer*/,
						bool showDialog /*= false*/)
{
	if(isEmpty()){
		return true;
	}

	FileWriteHandle fh(nstr(filename.GetFullPath()));
	if(!fh.isOpen()) {
		return false;
	}

	if(showDialog){
		g_editor.CreateLoadBar("Exporting minimap...");
	}

	Position min = getMinPosition();
	int width = getWidth();
	int height = getHeight();
	auto bitmap = std::make_unique<uint8_t[]>(width * height);
	memset(bitmap.get(), 0, width * height);

	{
		double nextUpdate = 0.0;
		forEachTile(
			[&nextUpdate, floor, showDialog, min, width, height, &bitmap](const Tile *tile, double progress){
				if(showDialog && progress >= nextUpdate){
					g_editor.SetLoadDone((int)(progress * 90));
					nextUpdate = progress + 0.01;
				}

				if(tile->empty() || tile->pos.z != floor){
					return;
				}

				int relX = tile->pos.x - min.x;
				int relY = tile->pos.y - min.y;
				int index = (height - relY - 1) * width + relX;
				bitmap[index] = tile->getMiniMapColor();
			});
	}


	int pitch = ((width + 3) / 4) * 4;
	int padding = width - pitch;
	int headerSize = (14 + 40 + 4 * 256);
	int fileSize = headerSize + pitch * height;

	// BMP header
	fh.addRAW("BM");			// identifier
	fh.addU32(fileSize);		// file size
	fh.addU16(0);				// reserved
	fh.addU16(0);				// reserved
	fh.addU32(headerSize);		// bitmap data offset

	// DIB header
	fh.addU32(40);				// DIB header size
	fh.addU32(width);			// width
	fh.addU32(height);			// height
	fh.addU16(1);				// color planes
	fh.addU16(8);				// bits per pixel
	fh.addU32(0);				// compression type
	fh.addU32(0);				// size of the raw bitmap data, can be zero when not compressed
	fh.addU32(4000);			// horizontal resolution in pixels per meter
	fh.addU32(4000);			// vertical resolution in pixels per meter
	fh.addU32(256);				// colors in the color palette
	fh.addU32(0);				// important colors, generally ignored

	// Palette
	for(int i = 0; i < 256; i += 1)
		fh.addU32(colorFromEightBit(i).GetRGB());

	// Bitmap
	{
		double nextUpdate = 0.0;
		for(int y = 0; y < height; y += 1){
			fh.addRAW(&bitmap[y*width], width);
			for(int i = 0; i < padding; i += 1){
				fh.addU8(0);
			}

			double progress = (double)y / (double)height;
			if(showDialog && nextUpdate >= progress){
				g_editor.SetLoadDone(90 + (int)(progress * 10.0));
				nextUpdate = progress + 0.01;
			}
		}
	}

	if(showDialog){
		g_editor.DestroyLoadBar();
	}

	fh.close();
	return true;
}

