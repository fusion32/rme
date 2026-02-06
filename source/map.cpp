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

#include <wx/dir.h>

bool Map::load(const wxString &projectDir, wxString &outError, wxArrayString &outWarnings)
{
	wxString mapDirAttempt = projectDir + "origmap";
	if(!wxDir::Exists(mapDirAttempt)){
		outError << "Unable to locate map directory";
		return false;
	}

	wxString saveDirAttempt = projectDir + "save";
	if(!wxDir::Exists(saveDirAttempt)){
		outError << "Unable to locate save directory";
		return false;
	}

	wxString filename;
	wxDir dir(mapDirAttempt);
	if(dir.GetFirst(&filename, "*.sec")){
		do{
			// TODO(fusion): Load sectors...
			std::cout << filename << std::endl;
		}while(dir.GetNext(&filename));
	}

	//mapDir = std::move(mapDir);
	//saveDir = std::move(saveDirAttempt);

	outError << "OK";
	return false;
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

Tile *Map::getTile(int x, int y, int z)
{
	uint32_t sectorId = GetMapSectorId(x, y, z);
	auto it = sectors.find(sectorId);
	if(it == sectors.end()){
		return NULL;
	}

	int offsetX = x & MAP_SECTOR_MASK;
	int offsetY = y & MAP_SECTOR_MASK;
	return &it->second.tiles[offsetY * MAP_SECTOR_SIZE + offsetX];
}

Tile *Map::getOrCreateTile(int x, int y, int z)
{
	uint32_t sectorId = GetMapSectorId(x, y, z);
	auto ret = sectors.try_emplace(sectorId);
	if(ret.second){
		int sectorX = x / MAP_SECTOR_SIZE;
		int sectorY = y / MAP_SECTOR_SIZE;
		int sectorZ = z;

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

		ret.first->second.setTilePositions(
				sectorX * MAP_SECTOR_SIZE,
				sectorY * MAP_SECTOR_SIZE,
				sectorZ);
	}

	int offsetX = x & MAP_SECTOR_MASK;
	int offsetY = y & MAP_SECTOR_MASK;
	return &ret.first->second.tiles[offsetY * MAP_SECTOR_SIZE + offsetX];
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

