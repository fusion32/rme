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
#include <wx/zipstrm.h>
#include <wx/wfstream.h>

static wxFileName BackupFileName(const wxString &filename){
	wxFileName fn(filename);
	fn.SetFullName(fn.GetFullName() + wxDateTime().UNow().Format("-%Y-%m-%d-%H%M%S%l.zip"));
	return fn;
}

static wxFileName BackupDirName(const wxString &dir){
	wxFileName fn(dir, "");
	const wxArrayString &dirs = fn.GetDirs();
	if(!dirs.IsEmpty()){
		fn.SetFullName(dirs.Last() + wxDateTime().UNow().Format("-%Y-%m-%d-%H%M%S%l.zip"));
	}else{
		fn.SetFullName(wxDateTime().UNow().Format("%Y-%m-%d-%H%M%S%l.zip"));
	}
	return fn;
}

static bool BackupFile(const wxString &filename){
	if(!wxFileName::Exists(filename)){
		return true;
	}

	wxFileName outputName = BackupFileName(filename);
	wxTempFileOutputStream outputFile(outputName.GetFullPath());
	if(!outputFile.IsOk()){
		g_editor.Error(wxString() << "Failed to create backup file " << outputName.GetFullName());
		return false;
	}

	{
		wxZipOutputStream zip(outputFile, 9);
		{
			wxFileInputStream inputStream(filename);
			if(!inputStream.IsOk()){
				g_editor.Error(wxString() << "Failed to open file " << filename << " for reading");
				return false;
			}

			wxString entryName = wxFileNameFromPath(filename);
			if(!zip.PutNextEntry(entryName)){
				g_editor.Error(wxString() << "Failed to append ZIP entry for " << entryName);
				return false;
			}

			zip << inputStream;
		}

		if(!zip.Close()){
			g_editor.Error(wxString() << "Failed to wrap backup file " << outputName.GetFullName());
			return false;
		}
	}

	if(!outputFile.Commit()){
		g_editor.Error(wxString() << "Failed to commit backup file " << outputName.GetFullName());
		return false;
	}

	wxRemoveFile(filename);
	return true;
}

static bool BackupSectorsAndPatches(const wxString &dir){
	wxArrayString filenames;
	wxDir::GetAllFiles(dir, &filenames, "*.sec", wxDIR_FILES | wxDIR_HIDDEN);
	wxDir::GetAllFiles(dir, &filenames, "*.pat", wxDIR_FILES | wxDIR_HIDDEN);
	if(filenames.IsEmpty()){
		return true;
	}

	wxFileName outputName = BackupDirName(dir);
	wxTempFileOutputStream outputFile(outputName.GetFullPath());
	if(!outputFile.IsOk()){
		g_editor.Error(wxString() << "Failed to create backup file " << outputName.GetFullName());
		return false;
	}

	{
		wxZipOutputStream zip(outputFile, 9);
		for(const wxString &filename: filenames){
			wxFileInputStream inputStream(filename);
			if(!inputStream.IsOk()){
				g_editor.Error(wxString() << "Failed to open file " << filename << " for reading");
				return false;
			}

			wxString entryName = wxFileNameFromPath(filename);
			if(!zip.PutNextEntry(entryName)){
				g_editor.Error(wxString() << "Failed to append ZIP entry for " << entryName);
				return false;
			}

			zip << inputStream;
		}

		if(!zip.Close()){
			g_editor.Error(wxString() << "Failed to wrap backup file " << outputName.GetFullName());
			return false;
		}
	}

	if(!outputFile.Commit()){
		g_editor.Error(wxString() << "Failed to commit backup file " << outputName.GetFullName());
		return false;
	}

	for(const wxString &filename: filenames){
		wxRemoveFile(filename);
	}

	return true;
}

static Item *LoadObjects(Script *script){
	ASSERT(script != NULL);

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

static void SaveObjects(ScriptWriter *script, const Item *first){
	ASSERT(script != NULL);
	script->writeText("{");
	for(const Item *item = first; item != NULL; item = item->next){
		if(item != first) script->writeText(", ");
		script->writeNumber(item->getID());

		for(int attr = 0; attr < NUM_INSTANCE_ATTRIBUTES; attr += 1){
			if(attr == CONTENT || item->getAttributeOffset((ObjectInstanceAttribute)attr) == -1){
				continue;
			}

			script->writeText(" ");
			script->writeText(GetInstanceAttributeName(attr));
			script->writeText("=");
			if(attr == TEXTSTRING || attr == EDITOR){
				script->writeString(item->getTextAttribute((ObjectInstanceAttribute)attr));
			}else{
				script->writeNumber(item->getAttribute((ObjectInstanceAttribute)attr));
			}
		}

		if(item->getAttributeOffset(CONTENT) != -1){
			script->writeText(" Content=");
			SaveObjects(script, item->content);
		}
	}
	script->writeText("}");
}

static void SaveTile(ScriptWriter *script, int offsetX, int offsetY, const Tile *tile){
	ASSERT(script != NULL && tile != NULL);

	script->writeNumber(offsetX);
	script->writeText("-");
	script->writeNumber(offsetY);
	script->writeText(": ");

	if(tile->getTileFlag(TILE_FLAG_REFRESH)){
		script->writeText("Refresh, ");
	}

	if(tile->getTileFlag(TILE_FLAG_NOLOGOUT)){
		script->writeText("NoLogout, ");
	}

	if(tile->getTileFlag(TILE_FLAG_PROTECTIONZONE)){
		script->writeText("ProtectionZone, ");
	}

	script->writeText("Content=");
	SaveObjects(script, tile->items);

	script->writeLn();
}

void Map::loadSector(SectorType type, MapSector *sector, Script *script){
	ASSERT(sector != NULL && script != NULL);

	// NOTE(fusion): A full patch replaces the whole sector.
	if(type == SECTOR_FULL_PATCH){
		for(Tile &tile: sector->tiles){
			tile.clear();
			tile.setTileFlag(TILE_FLAG_DIRTY);
		}
	}

	Tile *tile = NULL;
	std::string ident;
	while(true){
		script->nextToken();
		if(script->eof()){
			break;
		}

		if(script->token.kind == TOKEN_SPECIAL && script->getSpecial() == ','){
			continue;
		}

		if(script->token.kind == TOKEN_BYTES){
			const uint8_t *offset = script->getBytes();
			int offsetX = (int)offset[0];
			int offsetY = (int)offset[1];
			script->readSymbol(':');

			tile = sector->getTile(offsetX, offsetY);
			if(tile == NULL){
				script->error("invalid sector offset");
				break;
			}

			// NOTE(fusion): A regular patch replaces only specified tiles.
			if(type == SECTOR_PATCH){
				tile->clear();
				tile->setTileFlag(TILE_FLAG_DIRTY);
			}

			// NOTE(fusion): An overlay add changes on top of specified tiles.
			if(type == SECTOR_OVERLAY){
				tile->setTileFlag(TILE_FLAG_DIRTY);
			}

		}else if(script->token.kind == TOKEN_IDENTIFIER){
			if(tile == NULL){
				script->error("coordinate expected");
				break;
			}

			ident = script->getIdentifier();
			if(ident == "refresh"){
				tile->setTileFlag(TILE_FLAG_REFRESH);
			}else if(ident == "nologout"){
				tile->setTileFlag(TILE_FLAG_NOLOGOUT);
			}else if(ident == "protectionzone"){
				tile->setTileFlag(TILE_FLAG_PROTECTIONZONE);
			}else if(ident == "content"){
				script->readSymbol('=');
				tile->addItems(LoadObjects(script));
			}else{
				script->error("unknown map flag");
				break;
			}
		}else{
			script->error("next map point expected");
			break;
		}
	}
}

bool Map::loadSector(SectorType type, const wxFileName &filename){
	int sectorX, sectorY, sectorZ;
	if(sscanf(filename.GetFullName().mb_str(), "%d-%d-%d.sec", &sectorX, &sectorY, &sectorZ) != 3){
		g_editor.Error(wxString() << "Invalid sector name format: " << filename.GetFullName());
		return false;
	}

	if(!SectorValid(sectorX, sectorY, sectorZ)){
		g_editor.Error(wxString() << "Invalid sector coordinates "
				<< sectorX << "-" << sectorY << "-" << sectorZ);
		return false;
	}

	Script script(filename.GetFullPath().mb_str());
	MapSector *sector = getOrCreateSectorAt(sectorX * MAP_SECTOR_SIZE, sectorY * MAP_SECTOR_SIZE, sectorZ);
	loadSector(type, sector, &script);
	if(const char *error = script.getError()){
		g_editor.Error(error, ProblemSource::FromSector(sectorX, sectorY, sectorZ));
		return false;
	}

	return true;
}

bool Map::loadPatch(SectorType type, const wxFileName &filename){
	int patchNumber;
	if(sscanf(filename.GetFullName().mb_str(), "%d.sec", &patchNumber) != 1){
		g_editor.Error(wxString() << "Invalid patch name format: " << filename.GetFullName());
		return false;
	}

	Script script(filename.GetFullPath().mb_str());
	if(strcmp(script.readIdentifier(), "sector") != 0){
		g_editor.Error("Expected patch sector identifier");
		return false;
	}

	int sectorX = script.readNumber();
	script.readSymbol(',');
	int sectorY = script.readNumber();
	script.readSymbol(',');
	int sectorZ = script.readNumber();
	if(!script.eof()){
		MapSector *sector = getOrCreateSectorAt(
				sectorX * MAP_SECTOR_SIZE,
				sectorY * MAP_SECTOR_SIZE,
				sectorZ);
		loadSector(type, sector, &script);
	}

	if(const char *error = script.getError()){
		g_editor.Error(error, ProblemSource::FromSector(sectorX, sectorY, sectorZ));
		return false;
	}

	return true;
}

bool Map::loadSpawns(const wxString &projectDir){
	wxString filename;
	{
		wxPathList paths;
		paths.Add(projectDir);
		paths.Add(projectDir + "/dat");
		filename = paths.FindValidPath("monster.db");
		if(filename.IsEmpty()){
			g_editor.Error("Unable to locate monster.db");
			return false;
		}
	}

	// NOTE(fusion): A singular ZERO is used to denote the end of the file.
	Script script(filename.mb_str());
	while(int raceId = script.readNumber()){
		int x = script.readNumber();
		int y = script.readNumber();
		int z = script.readNumber();
		int radius = script.readNumber();
		int amount = script.readNumber();
		int interval = script.readNumber();
		if(Tile *tile = getTile(x, y, z)){
			if(Creature *creature = tile->creature){
				g_editor.Warning(wxString() << "Spawn of " << GetCreatureType(raceId).name
						<< " overriding spawn of " << GetCreatureType(creature->raceId).name,
						ProblemSource::FromPosition(x, y, z));
			}
			tile->placeCreature(raceId, radius, amount, interval);
		}else{
			g_editor.Warning(wxString()
					<< "Spawn of " << GetCreatureType(raceId).name << " is out of bounds",
					ProblemSource::FromPosition(x, y, z));
		}
	}

	if(const char *error = script.getError()){
		g_editor.Error(error);
		return false;
	}

	spawnsFile = std::move(filename);
	return true;
}

bool Map::loadHouses(const wxString &projectDir){
	return false;
}

bool Map::load(const wxString &projectDir)
{
	// TODO(fusion): Not sure about creating the directory and whether we should
	// attempt to locate some directory with .sec files in it.

	{
		wxString mapDirAttempt = projectDir + "/origmap";
		if(!wxDir::Exists(mapDirAttempt)){
			if(!wxDir::Make(mapDirAttempt, 0755)){
				g_editor.Error("Unable to create new map directory");
				return false;
			}

			g_editor.Notice(wxString() << "Created new map directory at " << mapDirAttempt);
		}

		wxString saveDirAttempt = projectDir + "/save";
		if(!wxDir::Exists(saveDirAttempt)){
			if(!wxDir::Make(saveDirAttempt, 0755)){
				g_editor.Error("Unable to locate nor create save directory");
				return false;
			}

			g_editor.Notice(wxString() << "Created new save directory at " << mapDirAttempt);
		}

		mapDir = std::move(mapDirAttempt);
		saveDir = std::move(saveDirAttempt);
	}

	{ // NOTE(fusion): Load base map.
		g_editor.SetLoadDone(0, "Discovering sector files...");

		int numSectors = 0;
		wxString filename;
		wxDir dir(mapDir);
		if(dir.GetFirst(&filename, "*.sec", wxDIR_FILES)){
			do{
				numSectors += 1;
			}while(dir.GetNext(&filename));
		}

		int numLoaded = 0;
		if(dir.GetFirst(&filename, "*.sec", wxDIR_FILES)){
			do{
				g_editor.SetLoadDone((numLoaded * 95 / numSectors),
						wxString::Format("Loading sector %s (%d/%d)",
							filename, numLoaded, numSectors));

				wxFileName fn(mapDir, filename);
				if(!loadSector(SECTOR_BASELINE, fn)){
					g_editor.Error(wxString() << "Unable to load sector " << fn.GetFullPath());
				}

				numLoaded += 1;
			}while(dir.GetNext(&filename));
		}
	}

	{ // NOTE(fusion): Load patches.
		g_editor.SetLoadDone(94, "Loading patches...");

		int numLoaded = 0;
		wxString filename;
		wxDir dir(saveDir);

		if(dir.GetFirst(&filename, "*.sec", wxDIR_FILES)){
			do{
				g_editor.SetLoadDone(94, wxString::Format("Loading sector patch %s (%d)", filename, numLoaded));

				wxFileName fn(saveDir, filename);
				if(!loadSector(SECTOR_FULL_PATCH, fn)){
					g_editor.Error(wxString() << "Unable to load full patch " << fn.GetFullPath());
				}

				numLoaded += 1;
			}while(dir.GetNext(&filename));
		}

		if(dir.GetFirst(&filename, "*.pat", wxDIR_FILES)){
			do{
				g_editor.SetLoadDone(95, wxString::Format("Loading sector patch %s (%d)", filename, numLoaded));

				wxFileName fn(saveDir, filename);
				if(!loadPatch(SECTOR_PATCH, fn)){
					g_editor.Error(wxString() << "Unable to load patch " << fn.GetFullPath());
				}

				numLoaded += 1;
			}while(dir.GetNext(&filename));
		}
	}


	{ // NOTE(fusion): Load spawns.
		g_editor.SetLoadDone(96, "Loading spawns...");
		if(!loadSpawns(projectDir)){
			g_editor.Error("Unable to load spawns");
		}
	}

#if TODO
	{ // TODO(fusion): Load houses.
		g_editor.SetLoadDone(97, "Loading house areas...");
		g_editor.SetLoadDone(98, "Loading houses...");
	}
#endif

#if TODO
	{ // TODO(fusion): Load map.dat (find an appropriate name? maybe map meta
	  // data, following the convention for sprite meta data). We need to load
	  // it completely so we're able to save it with minimal changes.
		g_editor.SetLoadDone(99, "Loading map.dat...");
	}
#endif

	return true;
}

bool Map::saveSector(const wxString &dir, const MapSector *sector){
	ASSERT(sector != NULL);
	int sectorX = sector->tiles[0].pos.x / MAP_SECTOR_SIZE;
	int sectorY = sector->tiles[0].pos.y / MAP_SECTOR_SIZE;
	int sectorZ = sector->tiles[0].pos.z;

	ScriptWriter script;
	wxString filename = wxString::Format("%s/%04d-%04d-%02d.sec", dir, sectorX, sectorY, sectorZ);
	if(!script.begin(filename.mb_str())){
		g_editor.Warning(wxString()
				<< "Failed to open sector script " << filename << " for writing",
				ProblemSource::FromSector(sectorX, sectorY, sectorZ));
		return false;
	}

	script.writeText("# Tibia - graphical Multi-User-Dungeon");
	script.writeLn();
	script.writeText("# Data for sector ");
	script.writeNumber(sectorX);
	script.writeText("/");
	script.writeNumber(sectorY);
	script.writeText("/");
	script.writeNumber(sectorZ);
	script.writeLn();
	script.writeLn();

	for(const Tile &tile: sector->tiles){
		int offsetX = tile.pos.x & MAP_SECTOR_MASK;
		int offsetY = tile.pos.y & MAP_SECTOR_MASK;
		SaveTile(&script, offsetX, offsetY, &tile);
	}

	if(!script.end()){
		g_editor.Error(wxString()
				<< "Failed to wrap sector script " << filename
					<< ", it may not have been properly stored.",
				ProblemSource::FromSector(sectorX, sectorY, sectorZ));
		return false;
	}

	return true;
}

bool Map::savePatch(const wxString &dir, const MapSector *sector, int patchNumber){
	ASSERT(sector != NULL);
	int sectorX = sector->tiles[0].pos.x / MAP_SECTOR_SIZE;
	int sectorY = sector->tiles[0].pos.y / MAP_SECTOR_SIZE;
	int sectorZ = sector->tiles[0].pos.z;

	ScriptWriter script;
	wxString filename = wxString::Format("%s/%03d.pat", dir, patchNumber);
	if(!script.begin(filename.mb_str())){
		g_editor.Warning(wxString()
				<< "Failed to open patch script " << filename << " for writing",
				ProblemSource::FromSector(sectorX, sectorY, sectorZ));
		return false;
	}

	script.writeText("# Tibia - graphical Multi-User-Dungeon");
	script.writeLn();
	script.writeText("# Patch File");
	script.writeLn();
	script.writeLn();
	script.writeText("Sector ");
	script.writeNumber(sectorX);
	script.writeText(",");
	script.writeNumber(sectorY);
	script.writeText(",");
	script.writeNumber(sectorZ);
	script.writeLn();
	script.writeLn();

	for(const Tile &tile: sector->tiles){
		if(!tile.getTileFlag(TILE_FLAG_DIRTY)){
			continue;
		}

		int offsetX = tile.pos.x & MAP_SECTOR_MASK;
		int offsetY = tile.pos.y & MAP_SECTOR_MASK;
		SaveTile(&script, offsetX, offsetY, &tile);
	}

	if(!script.end()){
		g_editor.Warning(wxString()
				<< "Failed to wrap patch script " << filename
					<<  ", it may not have been properly stored.",
				ProblemSource::FromSector(sectorX, sectorY, sectorZ));
		return false;
	}

	return true;
}

bool Map::saveSpawns(void){
	if(spawnsFile.IsEmpty()){
		return false;
	}

	// TODO(fusion): Probably backup the whole dat directory to reduce the number
	// of files once we're actually modifying houses and marks?
	if(true){ //if(g_settings.getBoolean(Config::BACKUP_SPAWNS)){
		while(!BackupFile(spawnsFile)){
			int ret = g_editor.PopupDialog("Backup error",
					"There was an error while backing up spawns, do you want to retry?",
					wxYES | wxNO | wxCANCEL);
			if(ret == wxID_CANCEL) return false;
			if(ret == wxID_NO)     break;
		}
	}


	struct Spawn{
		int x, y, z;
		int raceId;
		int radius;
		int amount;
		int regen;
	};

	std::vector<Spawn> spawns;

	{
		// TODO(fusion): I wanted to have creatures/spawns separate from the tile,
		// in such a way that this gather step wouldn't be necessary.
		for(const auto &[sectorId, sector]: sectors){
			for(const Tile &tile: sector.tiles){
				Creature *creature = tile.creature;
				if(!creature){
					continue;
				}

				Spawn spawn = {};
				spawn.x = tile.pos.x;
				spawn.y = tile.pos.y;
				spawn.z = tile.pos.z;
				spawn.raceId = creature->raceId;
				spawn.radius = creature->spawnRadius;
				spawn.amount = creature->spawnAmount;
				spawn.regen = creature->spawnInterval;
				spawns.push_back(spawn);
			}
		}

		std::sort(spawns.begin(), spawns.end(),
			[](const Spawn &a, const Spawn &b){
				int aSectorX = a.x / MAP_SECTOR_SIZE;
				int aOffsetX = a.x % MAP_SECTOR_SIZE;
				int bSectorX = b.x / MAP_SECTOR_SIZE;
				int bOffsetX = b.x % MAP_SECTOR_SIZE;
				if(aSectorX != bSectorX){
					return aSectorX < bSectorX;
				}

				int aSectorY = a.y / MAP_SECTOR_SIZE;
				int aOffsetY = a.y % MAP_SECTOR_SIZE;
				int bSectorY = b.y / MAP_SECTOR_SIZE;
				int bOffsetY = b.y % MAP_SECTOR_SIZE;
				if(aSectorY != bSectorY){
					return aSectorY < bSectorY;
				}

				int aSectorZ = a.z;
				int bSectorZ = b.z;
				if(aSectorZ != bSectorZ){
					return aSectorZ < bSectorZ;
				}

				if(a.raceId != b.raceId){
					return a.raceId < b.raceId;
				}

				// TODO(fusion): Some groups follow this ascending, descending,
				// and others don't seem to follow this at all.
				int aOffset = aOffsetY * MAP_SECTOR_SIZE + aOffsetX;
				int bOffset = bOffsetY * MAP_SECTOR_SIZE + bOffsetX;
				return aOffset < bOffset;
			});
	}

	ScriptWriter script;
	if(!script.begin(spawnsFile.mb_str())){
		g_editor.Warning(wxString() << "Failed to open spawn file " << spawnsFile << " for writing");
		return false;
	}

	script.writeText("# Tibia - graphical Multi-User-Dungeon");
	script.writeLn();
	script.writeText("# MonsterHomes File");
	script.writeLn();
	script.writeLn();
	script.writeText("# Race     X     Y  Z Radius Amount Regen.");
	script.writeLn();
	script.writeLn();

	char line[256] = {};
	uint32_t currentSectorId = 0;
	for(const Spawn &spawn: spawns){
		uint32_t sectorId = GetMapSectorId(spawn.x, spawn.y, spawn.z);
		if(sectorId != currentSectorId){
			snprintf(line, sizeof(line),
					"# ====== %04d,%04d,%02d ====================",
					(spawn.x / MAP_SECTOR_SIZE),
					(spawn.y / MAP_SECTOR_SIZE),
					spawn.z);
			script.writeText(line);
			script.writeLn();
			currentSectorId = sectorId;
		}

		snprintf(line, sizeof(line), "%6d %5d %5d %2d %6d %6d %6d",
				spawn.raceId, spawn.x, spawn.y, spawn.z,
				spawn.radius, spawn.amount, spawn.regen);
		script.writeText(line);
		script.writeLn();
	}

	script.writeText("0 # zero for end of file");
	script.writeLn();

	if(!script.end()){
		g_editor.Warning(wxString()
				<< "Failed to wrap spawn file " << spawnsFile
					<<  ", it may not have been properly stored.");
		return false;
	}

	return true;
}

bool Map::saveHouses(void){
	if(housesFile.IsEmpty() || houseAreasFile.IsEmpty()){
		return false;
	}

	// TODO(fusion): Save houses.

	return false;
}

bool Map::save(void) // datDir, saveDir, backupDir
{
	if(saveDir.IsEmpty()){
		return false;
	}

	// TODO(fusion): Probably have a way to put all relevant files into the same
	// backup zip, including patches, spawns, houses, etc...

	if(true){ //if(g_settings.getBoolean(Config::BACKUP_PATCHES)){
		while(!BackupSectorsAndPatches(saveDir)){
			int ret = g_editor.PopupDialog("Backup error",
					"There was an error while backing up patches, do you want to retry?",
					wxYES | wxNO | wxCANCEL);
			if(ret == wxID_CANCEL) return false;
			if(ret == wxID_NO)     break;
		}
	}

	int nextPatchNumber = 0;
	int fullPatchThreshold = (MAP_SECTOR_SIZE * MAP_SECTOR_SIZE) / 2;
	for(const auto &[sectorId, sector]: sectors){
		int numDirty = 0;
		for(const Tile &tile: sector.tiles){
			if(tile.getTileFlag(TILE_FLAG_DIRTY)){
				numDirty += 1;
			}
		}

		// TODO(fusion): I have no idea whether this is a good heuristic,
		// or if there something else when determining a full patch. There
		// is also the issue with tiles being dirty if we modify respawns
		// so we might want to do a different approach like only tagging
		// sectors as dirty and checking the differences with the original
		// sector.
		if(numDirty >= fullPatchThreshold){
			saveSector(saveDir, &sector);
		}else if(numDirty > 0){
			savePatch(saveDir, &sector, nextPatchNumber);
			nextPatchNumber += 1;
		}
	}

	// TODO(fusion): Modify map.dat as well since the size of the map may have
	// changed. We'll also want to modify it to parse and save waypoints but
	// that's also pending.

	saveSpawns();
	//saveHouses();
	//saveHouseAreas();
	//saveMapMetadata(); // ??

	return true;
}

void Map::clear(void)
{
	mapDir.Clear();
	saveDir.Clear();
	spawnsFile.Clear();
	housesFile.Clear();
	houseAreasFile.Clear();
	minSectorX = 0;
	minSectorY = 0;
	minSectorZ = 0;
	maxSectorX = 0;
	maxSectorY = 0;
	maxSectorZ = 0;
	sectors.clear();
}

MapSector *Map::getSectorAt(int x, int y, int z){
	if(!PositionValid(x, y, z)){
		return NULL;
	}

	uint32_t sectorId = GetMapSectorId(x, y, z);
	auto it = sectors.find(sectorId);
	if(it != sectors.end()){
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

bool Map::exportMinimap(const wxFileName &filename,
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

