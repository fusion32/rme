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

#include "graphics.h"
#include "artprovider.h"
#include "filehandle.h"
#include "settings.h"
#include "editor.h"
#include "creature.h"

#include <wx/mstream.h>
#include <wx/stopwatch.h>
#include <wx/dir.h>
#include <wx/rawbmp.h>
#include "pngfiles.h"

// TODO(fusion): Adjust this for whatever format 7.7 uses.
enum DatFlag: uint8_t {
	DatFlagBank             = 0,
	DatFlagClip             = 1,
	DatFlagBottom           = 2,
	DatFlagTop              = 3,
	DatFlagContainer        = 4,
	DatFlagCumulative       = 5,
	DatFlagForceUse         = 6,
	DatFlagMultiUse         = 7,
	DatFlagWrite            = 8,
	DatFlagWriteOnce        = 9,
	DatFlagLiquidContainer  = 10,
	DatFlagLiquidPool       = 11,
	DatFlagUnpass           = 12,
	DatFlagUnmove           = 13,
	DatFlagUnthrow          = 14,
	DatFlagAvoid            = 15,
	DatFlagTake             = 16,
	DatFlagHang             = 17,
	DatFlagHookSouth        = 18,
	DatFlagHookEast         = 19,
	DatFlagRotate           = 20,
	DatFlagLight            = 21,
	DatFlagDontHide         = 22,
	DatFlagTeleportRelative = 23, // maybe?
	DatFlagShift            = 24,
	DatFlagHeight           = 25,
	DatFlagLyingObject      = 26,
	DatFlagAnimateAlways    = 27,
	DatFlagAutomapColor     = 28,
	DatFlagLensHelp         = 29,
	DatFlagFullBank         = 30,
	DatFlagLast             = 255,
};

// All 133 template colors
static uint32_t TemplateOutfitLookupTable[] = {
	0xFFFFFF, 0xFFD4BF, 0xFFE9BF, 0xFFFFBF, 0xE9FFBF, 0xD4FFBF,
	0xBFFFBF, 0xBFFFD4, 0xBFFFE9, 0xBFFFFF, 0xBFE9FF, 0xBFD4FF,
	0xBFBFFF, 0xD4BFFF, 0xE9BFFF, 0xFFBFFF, 0xFFBFE9, 0xFFBFD4,
	0xFFBFBF, 0xDADADA, 0xBF9F8F, 0xBFAF8F, 0xBFBF8F, 0xAFBF8F,
	0x9FBF8F, 0x8FBF8F, 0x8FBF9F, 0x8FBFAF, 0x8FBFBF, 0x8FAFBF,
	0x8F9FBF, 0x8F8FBF, 0x9F8FBF, 0xAF8FBF, 0xBF8FBF, 0xBF8FAF,
	0xBF8F9F, 0xBF8F8F, 0xB6B6B6, 0xBF7F5F, 0xBFAF8F, 0xBFBF5F,
	0x9FBF5F, 0x7FBF5F, 0x5FBF5F, 0x5FBF7F, 0x5FBF9F, 0x5FBFBF,
	0x5F9FBF, 0x5F7FBF, 0x5F5FBF, 0x7F5FBF, 0x9F5FBF, 0xBF5FBF,
	0xBF5F9F, 0xBF5F7F, 0xBF5F5F, 0x919191, 0xBF6A3F, 0xBF943F,
	0xBFBF3F, 0x94BF3F, 0x6ABF3F, 0x3FBF3F, 0x3FBF6A, 0x3FBF94,
	0x3FBFBF, 0x3F94BF, 0x3F6ABF, 0x3F3FBF, 0x6A3FBF, 0x943FBF,
	0xBF3FBF, 0xBF3F94, 0xBF3F6A, 0xBF3F3F, 0x6D6D6D, 0xFF5500,
	0xFFAA00, 0xFFFF00, 0xAAFF00, 0x54FF00, 0x00FF00, 0x00FF54,
	0x00FFAA, 0x00FFFF, 0x00A9FF, 0x0055FF, 0x0000FF, 0x5500FF,
	0xA900FF, 0xFE00FF, 0xFF00AA, 0xFF0055, 0xFF0000, 0x484848,
	0xBF3F00, 0xBF7F00, 0xBFBF00, 0x7FBF00, 0x3FBF00, 0x00BF00,
	0x00BF3F, 0x00BF7F, 0x00BFBF, 0x007FBF, 0x003FBF, 0x0000BF,
	0x3F00BF, 0x7F00BF, 0xBF00BF, 0xBF007F, 0xBF003F, 0xBF0000,
	0x242424, 0x7F2A00, 0x7F5500, 0x7F7F00, 0x557F00, 0x2A7F00,
	0x007F00, 0x007F2A, 0x007F55, 0x007F7F, 0x00547F, 0x002A7F,
	0x00007F, 0x2A007F, 0x54007F, 0x7F007F, 0x7F0055, 0x7F002A,
	0x7F0000,
};

static const char* selection_marker_xpm16x16[] = {
	// columns rows colors chars-per-pixel
	"16 16 2 1",
	"  c None",
	". c #000080",
	// pixels
	" . . . . . . . .",
	". . . . . . . . ",
	" . . . . . . . .",
	". . . . . . . . ",
	" . . . . . . . .",
	". . . . . . . . ",
	" . . . . . . . .",
	". . . . . . . . ",
	" . . . . . . . .",
	". . . . . . . . ",
	" . . . . . . . .",
	". . . . . . . . ",
	" . . . . . . . .",
	". . . . . . . . ",
	" . . . . . . . .",
	". . . . . . . . "
};

static const char* selection_marker_xpm32x32[] = {
	// columns rows colors chars-per-pixel
	"32 32 2 1",
	"  c None",
	". c #000080",
	// pixels
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . "
};

GraphicManager::GraphicManager() :
	unloaded(true),
	itemBaseId(0),
	itemCount(0),
	creatureBaseId(0),
	creatureCount(0),
	loaded_textures(0),
	lastclean(0)
{
	animation_timer = newd wxStopWatch();
	animation_timer->Start();
}

GraphicManager::~GraphicManager()
{
	for(auto [id, spr]: sprite_space){
		delete spr;
	}

	for(auto [id, img]: image_space){
		delete img;
	}

	sprite_space.clear();
	image_space.clear();

	delete animation_timer;
}

bool GraphicManager::isUnloaded() const
{
	return unloaded;
}

GLuint GraphicManager::getFreeTextureID()
{
	static GLuint id_counter = 0x10000000;
	return id_counter++; // This should (hopefully) never run out
}

void GraphicManager::clear()
{
	std::unordered_map<int, Sprite*> new_sprite_space;
	for(auto [id, spr]: sprite_space){
		if(id < 0){ // keep internal sprites
			new_sprite_space.insert(std::make_pair(id, spr));
		}else{
			delete spr;
		}
	}

	for(auto [id, img]: image_space){
		delete img;
	}

	sprite_space.swap(new_sprite_space);
	image_space.clear();
	cleanup_list.clear();

	itemBaseId = 0;
	itemCount = 0;
	creatureBaseId = 0;
	creatureCount = 0;
	loaded_textures = 0;
	lastclean = time(nullptr);
	spritefile = "";

	unloaded = true;
}

void GraphicManager::cleanSoftwareSprites()
{
	for(auto [id, spr]: sprite_space){
		if(id >= 0){ // don't clean internal sprites
			spr->unloadDC();
		}
	}
}

Sprite* GraphicManager::getSprite(int id)
{
	auto it = sprite_space.find(id);
	if(it != sprite_space.end()) {
		return it->second;
	}
	return nullptr;
}

GameSprite* GraphicManager::getCreatureSprite(int id)
{
	GameSprite *sprite = NULL;
	if(id > 0){
		auto it = sprite_space.find(creatureBaseId + id - 1);
		if(it != sprite_space.end()) {
			sprite = dynamic_cast<GameSprite*>(it->second);
		}
	}
	return sprite;
}

GameSprite* GraphicManager::getEditorSprite(int id)
{
	GameSprite *sprite = NULL;
	if(id < 0){
		auto it = sprite_space.find(id);
		if(it != sprite_space.end()) {
			sprite = dynamic_cast<GameSprite*>(it->second);
		}
	}
	return sprite;
}

#define loadPNGFile(name) _wxGetBitmapFromMemory(name, sizeof(name))
static wxBitmap* _wxGetBitmapFromMemory(const unsigned char* data, int length)
{
	wxMemoryInputStream is(data, length);
	wxImage img(is, "image/png");
	if(!img.IsOk()) return nullptr;
	return newd wxBitmap(img, -1);
}

bool GraphicManager::loadEditorSprites()
{
	// Unused graphics MIGHT be loaded here, but it's a neglectable loss
	sprite_space[EDITOR_SPRITE_SELECTION_MARKER] =
		newd EditorSprite(
			newd wxBitmap(selection_marker_xpm16x16),
			newd wxBitmap(selection_marker_xpm32x32)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_CD_1x1] =
		newd EditorSprite(
			loadPNGFile(circular_1_small_png),
			loadPNGFile(circular_1_png)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_CD_3x3] =
		newd EditorSprite(
			loadPNGFile(circular_2_small_png),
			loadPNGFile(circular_2_png)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_CD_5x5] =
		newd EditorSprite(
			loadPNGFile(circular_3_small_png),
			loadPNGFile(circular_3_png)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_CD_7x7] =
		newd EditorSprite(
			loadPNGFile(circular_4_small_png),
			loadPNGFile(circular_4_png)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_CD_9x9] =
		newd EditorSprite(
			loadPNGFile(circular_5_small_png),
			loadPNGFile(circular_5_png)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_CD_15x15] =
		newd EditorSprite(
			loadPNGFile(circular_6_small_png),
			loadPNGFile(circular_6_png)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_CD_19x19] =
		newd EditorSprite(
			loadPNGFile(circular_7_small_png),
			loadPNGFile(circular_7_png)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_SD_1x1] =
		newd EditorSprite(
			loadPNGFile(rectangular_1_small_png),
			loadPNGFile(rectangular_1_png)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_SD_3x3] =
		newd EditorSprite(
			loadPNGFile(rectangular_2_small_png),
			loadPNGFile(rectangular_2_png)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_SD_5x5] =
		newd EditorSprite(
			loadPNGFile(rectangular_3_small_png),
			loadPNGFile(rectangular_3_png)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_SD_7x7] =
		newd EditorSprite(
			loadPNGFile(rectangular_4_small_png),
			loadPNGFile(rectangular_4_png)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_SD_9x9] =
		newd EditorSprite(
			loadPNGFile(rectangular_5_small_png),
			loadPNGFile(rectangular_5_png)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_SD_15x15] =
		newd EditorSprite(
			loadPNGFile(rectangular_6_small_png),
			loadPNGFile(rectangular_6_png)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_SD_19x19] =
		newd EditorSprite(
			loadPNGFile(rectangular_7_small_png),
			loadPNGFile(rectangular_7_png)
		);

	sprite_space[EDITOR_SPRITE_OPTIONAL_BORDER_TOOL] =
		newd EditorSprite(
			loadPNGFile(optional_border_small_png),
			loadPNGFile(optional_border_png)
		);
	sprite_space[EDITOR_SPRITE_ERASER] =
		newd EditorSprite(
			loadPNGFile(eraser_small_png),
			loadPNGFile(eraser_png)
		);
#if TODO
	// TODO(fusion): Find some an appropriate brush icon?
	sprite_space[EDITOR_SPRITE_REFRESH_TOOL] =
		newd EditorSprite(
			loadPNGFile(refresh_zone_small_png),
			loadPNGFile(refresh_zone_png),
		);
#endif
	sprite_space[EDITOR_SPRITE_NOLOG_TOOL] =
		newd EditorSprite(
			loadPNGFile(no_logout_small_png),
			loadPNGFile(no_logout_png)
		);
	sprite_space[EDITOR_SPRITE_PZ_TOOL] =
		newd EditorSprite(
			loadPNGFile(protection_zone_small_png),
			loadPNGFile(protection_zone_png)
		);
	sprite_space[EDITOR_SPRITE_DOOR_NORMAL] =
		newd EditorSprite(
			loadPNGFile(door_normal_small_png),
			loadPNGFile(door_normal_png)
		);
	sprite_space[EDITOR_SPRITE_DOOR_LOCKED] =
		newd EditorSprite(
			loadPNGFile(door_locked_small_png),
			loadPNGFile(door_locked_png)
		);
	sprite_space[EDITOR_SPRITE_DOOR_MAGIC] =
		newd EditorSprite(
			loadPNGFile(door_magic_small_png),
			loadPNGFile(door_magic_png)
		);
	sprite_space[EDITOR_SPRITE_DOOR_QUEST] =
		newd EditorSprite(
			loadPNGFile(door_quest_small_png),
			loadPNGFile(door_quest_png)
		);
	sprite_space[EDITOR_SPRITE_WINDOW_NORMAL] =
		newd EditorSprite(
			loadPNGFile(window_normal_small_png),
			loadPNGFile(window_normal_png)
		);
	sprite_space[EDITOR_SPRITE_WINDOW_HATCH] =
		newd EditorSprite(
			loadPNGFile(window_hatch_small_png),
			loadPNGFile(window_hatch_png)
		);

	sprite_space[EDITOR_SPRITE_SELECTION_GEM] =
		newd EditorSprite(
			loadPNGFile(gem_edit_png),
			nullptr
		);
	sprite_space[EDITOR_SPRITE_DRAWING_GEM] =
		newd EditorSprite(
			loadPNGFile(gem_move_png),
			nullptr
		);

	sprite_space[EDITOR_SPRITE_SPAWNS] = GameSprite::createFromBitmap(ART_SPAWNS);
	sprite_space[EDITOR_SPRITE_HOUSE_EXIT] = GameSprite::createFromBitmap(ART_HOUSE_EXIT);
	sprite_space[EDITOR_SPRITE_PICKUPABLE_ITEM] = GameSprite::createFromBitmap(ART_PICKUPABLE);
	sprite_space[EDITOR_SPRITE_MOVEABLE_ITEM] = GameSprite::createFromBitmap(ART_MOVEABLE);
	sprite_space[EDITOR_SPRITE_PICKUPABLE_MOVEABLE_ITEM] = GameSprite::createFromBitmap(ART_PICKUPABLE_MOVEABLE);

	return true;
}

bool GraphicManager::loadSpriteMetadata(const wxString &projectDir)
{
	wxString filename;
	{
		wxPathList paths;
		paths.Add(projectDir);
		paths.Add(projectDir + "/editor");
		filename = paths.FindValidPath("Tibia.dat");
		if(filename.IsEmpty()){
			g_editor.Error("Unable to locate Tibia.dat");
			return false;
		}
	}

	FileReadHandle file(filename.ToStdString());
	if(!file.isOk()) {
		g_editor.Error(wxString() << "Failed to open " << filename
				<< " for reading: " << file.getErrorMessage());
		return false;
	}

	uint32_t datSignature;
	file.getU32(datSignature);

	uint16_t maxItemId, maxCreatureId;
	file.getU16(maxItemId);
	file.getU16(maxCreatureId);
	file.skip(2); // max effect id, unused
	file.skip(2); // max animation id, unused

	// NOTE(fusion): We're basically processing object types as a big single list,
	// with items starting at 100, followed by creatures, effects, and animations.
	itemBaseId     = 100;
	itemCount      = (maxItemId - 100 + 1);
	creatureBaseId = itemBaseId + itemCount;
	creatureCount  = (maxCreatureId - 1 + 1);

	int minTypeId  = itemBaseId;
	int maxTypeId  = minTypeId + itemCount + creatureCount - 1;
	for(int typeId = minTypeId; typeId <= maxTypeId; typeId += 1){
		GameSprite* sType = newd GameSprite();
		sprite_space[typeId] = sType;
		sType->id = typeId;

		// Load the sprite flags
		if(!loadSpriteMetadataFlags(file, sType)) {
			g_editor.Error(wxString() << "Failed to load flags for sprite " << sType->id);
			return false;
		}

		// Size and GameSprite data
		file.getByte(sType->width);
		file.getByte(sType->height);

		// Skipping the exact size
		if((sType->width > 1) || (sType->height > 1)){
			file.skip(1);
		}

		// NOTE(fusion): Some sprites (usually outfits) will have a second layer
		// with the color mask for multi-colored sprites.
		file.getU8(sType->layers);
		file.getU8(sType->pattern_x);
		file.getU8(sType->pattern_y);
		file.getU8(sType->pattern_z);
		file.getU8(sType->frames); // animation frames

		if(sType->frames > 1) {
			sType->animator = newd Animator(sType->frames, 0, 0, false);
		}

		sType->numsprites = (int)sType->width
				* (int)sType->height
				* (int)sType->layers
				* (int)sType->pattern_x
				* (int)sType->pattern_y
				* (int)sType->pattern_z
				* (int)sType->frames;

		for(int i = 0; i < sType->numsprites; i += 1) {
			uint16_t sprite_id;
			file.getU16(sprite_id);
			if(image_space[sprite_id] == nullptr) {
				GameSprite::NormalImage* img = newd GameSprite::NormalImage();
				img->id = sprite_id;
				image_space[sprite_id] = img;
			}
			sType->spriteList.push_back(static_cast<GameSprite::NormalImage*>(image_space[sprite_id]));
		}
	}

	// IMPORTANT(fusion): This needs to happen after all sprite metadata is
	// loaded because disguise targets doesn't have any ordering guarantees
	// and may be loaded after the type referencing it, meaning that getSprite()
	// would return NULL.
	for(int typeId = GetMinItemTypeId();
			typeId <= GetMaxItemTypeId();
			typeId += 1){
		ItemType *type = GetMutableItemType(typeId);
		if(!type){
			continue;
		}

		int lookId = type->getLookId();
		type->sprite = dynamic_cast<GameSprite*>(getSprite(lookId));
	}

	return true;
}

bool GraphicManager::loadSpriteMetadataFlags(FileReadHandle& file, GameSprite* sType)
{
	uint8_t prev_flag = 0;
	uint8_t flag = DatFlagLast;
	for(int i = 0; i < UINT8_MAX; i += 1) {
		prev_flag = flag;
		file.getU8(flag);
		if(flag == DatFlagLast) {
			return true;
		}

		switch (flag) {
			case DatFlagClip:
			case DatFlagBottom:
			case DatFlagTop:
			case DatFlagContainer:
			case DatFlagCumulative:
			case DatFlagForceUse:
			case DatFlagMultiUse:
			case DatFlagLiquidContainer:
			case DatFlagLiquidPool:
			case DatFlagUnpass:
			case DatFlagUnmove:
			case DatFlagUnthrow:
			case DatFlagAvoid:
			case DatFlagTake:
			case DatFlagHang:
			case DatFlagHookSouth:
			case DatFlagHookEast:
			case DatFlagRotate:
			case DatFlagDontHide:
			case DatFlagTeleportRelative:
			case DatFlagLyingObject:
			case DatFlagAnimateAlways:
			case DatFlagFullBank:
				break;

			case DatFlagBank:
			case DatFlagWrite:
			case DatFlagWriteOnce:
			case DatFlagLensHelp:
				file.skip(2);
				break;

			case DatFlagLight: {
				uint16_t intensity;
				uint16_t color;
				file.getU16(intensity);
				file.getU16(color);
				sType->has_light = true;
                sType->light = SpriteLight{
					static_cast<uint8_t>(intensity),
					static_cast<uint8_t>(color),
				};
				break;
			}

			case DatFlagShift: {
				uint16_t shiftX, shiftY;
				file.getU16(shiftX);
				file.getU16(shiftY);
				sType->draw_offset = wxPoint(shiftX, shiftY);
				break;
			}

			case DatFlagHeight: {
				uint16_t height;
				file.getU16(height);
				sType->draw_height = height;
				break;
			}

			case DatFlagAutomapColor: {
				uint16_t automapColor;
				file.getU16(automapColor);
				sType->minimap_color = automapColor;
				break;
			}

			default: {
				g_editor.Error(wxString() << "Metadata: Unknown flag: " << flag
						<< ". Previous flag: " << i2ws(prev_flag) << ".");
				return false;
			}
		}
	}

	return false;
}

bool GraphicManager::loadSpriteData(const wxString &projectDir)
{
	wxString filename;
	{
		wxPathList paths;
		paths.Add(projectDir);
		paths.Add(projectDir + "/editor");
		filename = paths.FindValidPath("Tibia.spr");
		if(filename.IsEmpty()){
			g_editor.Error("Unable to locate Tibia.spr");
			return false;
		}
	}

	FileReadHandle fh(filename.ToStdString());

	if(!fh.isOk()) {
		g_editor.Error(wxString() << "Failed to open " << filename
				<< " for reading: " << fh.getErrorMessage());
		return false;
	}

#define safe_get(func, ...) do{							\
		if(!fh.get##func(__VA_ARGS__)) {				\
			g_editor.Error(fh.getErrorMessage());		\
			return false;								\
		}												\
	}while(false)


	uint32_t sprSignature;
	uint16_t numSprites;
	safe_get(U32, sprSignature);
	safe_get(U16, numSprites);
	if(!g_settings.getInteger(Config::USE_MEMCACHED_SPRITES)) {
		spritefile = filename.ToStdString();
		unloaded = false;
		return true;
	}

	std::vector<uint32_t> spriteOffsets;
	spriteOffsets.resize(numSprites);
	for(uint16_t i = 0; i < numSprites; i += 1) {
		safe_get(U32, spriteOffsets[i]);
	}

	// TODO(fusion): Keeping the sprite in the "compressed" form can save some
	// memory but is it really that much? Decompressing it is trivial also so
	// it may not make that big of a difference in the end, idk.
	int spriteId = 1;
	for(uint32_t offset: spriteOffsets){
		// NOTE(fusion): An offset of zero means there is no sprite.
		if(offset > 0){
			uint16_t size;
			fh.seek(offset);
			fh.skip(3); // color key ?
			safe_get(U16, size);
			auto it = image_space.find(spriteId);
			if(it != image_space.end()) {
				GameSprite::NormalImage* spr = dynamic_cast<GameSprite::NormalImage*>(it->second);
				if(spr && size > 0) {
					if(spr->size > 0) {
						g_editor.Warning(wxString() << "Duplicate GameSprite id " << spriteId);
						fh.skip(size);
					} else {
						spr->id = spriteId;
						spr->size = size;
						spr->dump = newd uint8_t[size];
						safe_get(RAW, spr->dump, size);
					}
				}
			}
		}
		spriteId += 1;
	}
#undef safe_get
	unloaded = false;
	return true;
}

bool GraphicManager::loadSpriteDump(uint8_t*& target, uint16_t& size, int sprite_id)
{
	if(g_settings.getInteger(Config::USE_MEMCACHED_SPRITES))
		return false;

	if(sprite_id == 0) {
		// Empty GameSprite
		size = 0;
		target = nullptr;
		return true;
	}

	FileReadHandle fh(spritefile);
	if(!fh.isOk())
		return false;
	unloaded = false;

	if(!fh.seek(2 + sprite_id * sizeof(uint32_t)))
		return false;

	uint32_t to_seek = 0;
	if(fh.getU32(to_seek)) {
		fh.seek(to_seek+3);
		uint16_t sprite_size;
		if(fh.getU16(sprite_size)) {
			target = newd uint8_t[sprite_size];
			if(fh.getRAW(target, sprite_size)) {
				size = sprite_size;
				return true;
			}
			delete[] target;
			target = nullptr;
		}
	}
	return false;
}

void GraphicManager::addSpriteToCleanup(GameSprite* spr)
{
	cleanup_list.push_back(spr);
	// Clean if needed
	if(cleanup_list.size() > std::max<uint32_t>(100, g_settings.getInteger(Config::SOFTWARE_CLEAN_THRESHOLD))) {
		for(int i = 0; i < g_settings.getInteger(Config::SOFTWARE_CLEAN_SIZE) && static_cast<uint32_t>(i) < cleanup_list.size(); ++i) {
			cleanup_list.front()->unloadDC();
			cleanup_list.pop_front();
		}
	}
}

void GraphicManager::garbageCollection()
{
	if(g_settings.getInteger(Config::TEXTURE_MANAGEMENT)) {
		time_t t = time(nullptr);
		if(loaded_textures > g_settings.getInteger(Config::TEXTURE_CLEAN_THRESHOLD)
		&& (t - lastclean) > g_settings.getInteger(Config::TEXTURE_CLEAN_PULSE)) {
			for(auto [id, img]: image_space){
				img->clean(t);
			}

			for(auto [id, spr]: sprite_space){
				if(GameSprite *gspr = dynamic_cast<GameSprite*>(spr)){
					gspr->clean(t);
				}
			}

			lastclean = t;
		}
	}
}

EditorSprite::EditorSprite(wxBitmap* b16x16, wxBitmap* b32x32)
{
	bm[SPRITE_SIZE_16x16] = b16x16;
	bm[SPRITE_SIZE_32x32] = b32x32;
}

EditorSprite::~EditorSprite()
{
	unloadDC();
}

void EditorSprite::DrawTo(wxDC* dc, SpriteSize sz, int start_x, int start_y, int width, int height)
{
	wxBitmap* sp = bm[sz];
	if(sp)
		dc->DrawBitmap(*sp, start_x, start_y, true);
}

void EditorSprite::unloadDC()
{
	delete bm[SPRITE_SIZE_16x16];
	delete bm[SPRITE_SIZE_32x32];
	bm[SPRITE_SIZE_16x16] = nullptr;
	bm[SPRITE_SIZE_32x32] = nullptr;
}

GameSprite::GameSprite() :
	id(0),
	height(0),
	width(0),
	layers(0),
	pattern_x(0),
	pattern_y(0),
	pattern_z(0),
	frames(0),
	numsprites(0),
	animator(nullptr),
	draw_height(0),
	minimap_color(0)
{
	dc[SPRITE_SIZE_16x16] = nullptr;
	dc[SPRITE_SIZE_32x32] = nullptr;
}

GameSprite::~GameSprite()
{
	unloadDC();
	for(std::list<TemplateImage*>::iterator iter = instanced_templates.begin(); iter != instanced_templates.end(); ++iter) {
		delete *iter;
	}

	delete animator;
}

void GameSprite::clean(time_t time) {
	for(std::list<TemplateImage*>::iterator iter = instanced_templates.begin();
			iter != instanced_templates.end();
			++iter)
	{
		(*iter)->clean(time);
	}
}

void GameSprite::unloadDC()
{
	delete dc[SPRITE_SIZE_16x16];
	delete dc[SPRITE_SIZE_32x32];
	dc[SPRITE_SIZE_16x16] = nullptr;
	dc[SPRITE_SIZE_32x32] = nullptr;
}

int GameSprite::getIndex(int width, int height, int layer, int pattern_x, int pattern_y, int pattern_z, int frame) const
{
	return ((((((frame % this->frames) *
		this->pattern_z + pattern_z) *
		this->pattern_y + pattern_y) *
		this->pattern_x + pattern_x) *
		this->layers + layer) *
		this->height + height) *
		this->width + width;
}

GLuint GameSprite::getHardwareID(int _x, int _y, int _layer, int _pattern_x, int _pattern_y, int _pattern_z, int _frame)
{
	int v = ((((((_frame)*pattern_y+_pattern_y)*pattern_x+_pattern_x)*layers+_layer)*height+_y)*width+_x);
	if(v >= numsprites) {
		if(numsprites <= 1) {
			v = 0;
		} else {
			v %= numsprites;
		}
	}
	return spriteList[v]->getHardwareID();
}

GameSprite::TemplateImage* GameSprite::getTemplateImage(int sprite_index, const Outfit& outfit)
{
	if(instanced_templates.empty()) {
		TemplateImage* img = newd TemplateImage(this, sprite_index, outfit);
		instanced_templates.push_back(img);
		return img;
	}
	// While this is linear lookup, it is very rare for the list to contain more than 4-8 entries, so it's faster than a hashmap anyways.
	for(std::list<TemplateImage*>::iterator iter = instanced_templates.begin(); iter != instanced_templates.end(); ++iter) {
		TemplateImage* img = *iter;
		if(img->sprite_index == sprite_index) {
			uint32_t lookHash = img->lookHead << 24 | img->lookBody << 16 | img->lookLegs << 8 | img->lookFeet;
			if(outfit.getColorHash() == lookHash) {
				return img;
			}
		}
	}
	TemplateImage* img = newd TemplateImage(this, sprite_index, outfit);
	instanced_templates.push_back(img);
	return img;
}

GLuint GameSprite::getHardwareID(int _x, int _y, int _dir, int _addon, int _pattern_z, const Outfit& _outfit, int _frame)
{
	int v = getIndex(_x, _y, 0, _dir, _addon, _pattern_z, _frame);
	if(v >= numsprites) {
		if(numsprites == 1) {
			v = 0;
		} else {
			v %= numsprites;
		}
	}
	if(layers > 1) { // Template
		TemplateImage* img = getTemplateImage(v, _outfit);
		return img->getHardwareID();
	}
	return spriteList[v]->getHardwareID();
}

wxMemoryDC* GameSprite::getDC(SpriteSize size)
{
	ASSERT(size == SPRITE_SIZE_16x16 || size == SPRITE_SIZE_32x32);

	if(!dc[size]) {
		ASSERT(width >= 1 && height >= 1);

		const int bgshade = g_settings.getInteger(Config::ICON_BACKGROUND);

		int image_size = std::max<int>(width, height) * rme::SpritePixels;
		wxImage image(image_size, image_size);
		image.Clear(bgshade);

		for(uint8_t l = 0; l < layers; l++) {
			for(uint8_t w = 0; w < width; w++) {
				for(uint8_t h = 0; h < height; h++) {
					const int i = getIndex(w, h, l, 0, 0, 0, 0);
					uint8_t* data = spriteList[i]->getRGBData();
					if(data) {
						wxImage img(rme::SpritePixels, rme::SpritePixels, data);
						img.SetMaskColour(0xFF, 0x00, 0xFF);
						image.Paste(img, (width - w - 1) * rme::SpritePixels, (height - h - 1) * rme::SpritePixels);
						img.Destroy();
					}
				}
			}
		}

		// Now comes the resizing / antialiasing
		if(size == SPRITE_SIZE_16x16 || image.GetWidth() > rme::SpritePixels || image.GetHeight() > rme::SpritePixels) {
			int new_size = SPRITE_SIZE_16x16 ? 16 : 32;
			image.Rescale(new_size, new_size);
		}

		wxBitmap bmp(image);
		dc[size] = newd wxMemoryDC(bmp);
		g_editor.gfx.addSpriteToCleanup(this);
		image.Destroy();
	}
	return dc[size];
}

wxMemoryDC* GameSprite::getDC(const Outfit& outfit)
{
	ASSERT(width >= 1 && height >= 1);

	if(dc[SPRITE_SIZE_32x32]) {
		return dc[SPRITE_SIZE_32x32];
	}

	const int image_size = std::max<int>(width, height) * rme::SpritePixels;
	wxImage image(image_size, image_size);
	image.Clear(g_settings.getInteger(Config::ICON_BACKGROUND));

	const int direction = static_cast<int>(SOUTH) % pattern_x;

	for(uint8_t w = 0; w < width; w++) {
		for(uint8_t h = 0; h < height; h++) {
			const int index = getIndex(w, h, 0, direction, 0, 0, 0);
			uint8_t* data = nullptr;
			if(layers == 1) {
				data = spriteList[index]->getRGBData();
			} else {
				auto img = getTemplateImage(index, outfit);
				data = img->getRGBData();
			}
			if(data) {
				wxImage img(rme::SpritePixels, rme::SpritePixels, data);
				img.SetMaskColour(0xFF, 0x00, 0xFF);
				image.Paste(img, (width - w - 1) * rme::SpritePixels, (height - h - 1) * rme::SpritePixels);
				img.Destroy();
			}
		}
	}

	// resizing to 32x32
	if (image.GetWidth() > rme::SpritePixels || image.GetHeight() > rme::SpritePixels) {
		image.Rescale(32, 32);
	}

	wxBitmap bitmap(image);
	dc[SPRITE_SIZE_32x32] = new wxMemoryDC(bitmap);
	image.Destroy();

	g_editor.gfx.addSpriteToCleanup(this);
	return dc[SPRITE_SIZE_32x32];
}

void GameSprite::DrawTo(wxDC* dc, SpriteSize sz, int start_x, int start_y, int width, int height)
{
	if(width == -1)  width = sz == SPRITE_SIZE_32x32 ? 32 : 16;
	if(height == -1) height= sz == SPRITE_SIZE_32x32 ? 32 : 16;
	wxDC* sdc = getDC(sz);
	if(sdc) {
		dc->Blit(start_x, start_y, width, height, sdc, 0, 0, wxCOPY, true);
	} else {
		const wxBrush& b = dc->GetBrush();
		dc->SetBrush(*wxRED_BRUSH);
		dc->DrawRectangle(start_x, start_y, width, height);
		dc->SetBrush(b);
	}
}

void GameSprite::DrawTo(wxDC* context, const wxRect& rect, const Outfit& outfit)
{
	wxDC* source = getDC(outfit);
	if(source) {
		context->Blit(rect.x, rect.y, rect.width, rect.height, source, 0, 0, wxCOPY, true);
	} else {
		const wxBrush& brush = context->GetBrush();
		context->SetBrush(*wxRED_BRUSH);
		context->DrawRectangle(rect);
		context->SetBrush(brush);
	}
}

GameSprite::Image::Image() :
	isGLLoaded(false),
	lastaccess(0)
{
	////
}

GameSprite::Image::~Image()
{
	unloadGLTexture(0);
}

void GameSprite::Image::createGLTexture(GLuint textureId)
{
	ASSERT(!isGLLoaded);

	uint8_t* rgba = getRGBAData();
	if(!rgba) {
		return;
	}

	isGLLoaded = true;
	g_editor.gfx.loaded_textures += 1;

	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear Filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear Filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 0x812F); // GL_CLAMP_TO_EDGE
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 0x812F); // GL_CLAMP_TO_EDGE
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rme::SpritePixels, rme::SpritePixels, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);

	delete[] rgba;
}

void GameSprite::Image::unloadGLTexture(GLuint textureId)
{
	isGLLoaded = false;
	g_editor.gfx.loaded_textures -= 1;
	glDeleteTextures(1, &textureId);
}

void GameSprite::Image::visit()
{
	lastaccess = time(nullptr);
}

void GameSprite::Image::clean(time_t time)
{
	if(isGLLoaded && (time - lastaccess) > g_settings.getInteger(Config::TEXTURE_LONGEVITY)) {
		unloadGLTexture(0);
	}
}

GameSprite::NormalImage::NormalImage() :
	id(0),
	size(0),
	dump(nullptr)
{
	////
}

GameSprite::NormalImage::~NormalImage()
{
	delete[] dump;
}

void GameSprite::NormalImage::clean(time_t time)
{
	Image::clean(time);
	if((time - lastaccess) > 5 && !g_settings.getInteger(Config::USE_MEMCACHED_SPRITES)) { // We keep dumps around for 5 seconds.
		delete[] dump;
		dump = nullptr;
	}
}

uint8_t* GameSprite::NormalImage::getRGBData()
{
	if(!dump) {
		if(g_settings.getInteger(Config::USE_MEMCACHED_SPRITES)) {
			return nullptr;
		}

		if(!g_editor.gfx.loadSpriteDump(dump, size, id)) {
			return nullptr;
		}
	}

	const int pixels_data_size = rme::SpritePixels * rme::SpritePixels * 3;
	uint8_t* data = newd uint8_t[pixels_data_size];
	uint8_t bpp = g_editor.gfx.hasTransparency() ? 4 : 3;
	int write = 0;
	int read = 0;

	// decompress pixels
	while(read < size && write < pixels_data_size) {
		int transparent = dump[read] | dump[read + 1] << 8;
		read += 2;
		for(int i = 0; i < transparent && write < pixels_data_size; i++) {
			data[write + 0] = 0xFF; // red
			data[write + 1] = 0x00; // green
			data[write + 2] = 0xFF; // blue
			write += 3;
		}

		int colored = dump[read] | dump[read + 1] << 8;
		read += 2;
		for(int i = 0; i < colored && write < pixels_data_size; i++) {
			data[write + 0] = dump[read + 0]; // red
			data[write + 1] = dump[read + 1]; // green
			data[write + 2] = dump[read + 2]; // blue
			write += 3;
			read += bpp;
		}
	}

	// fill remaining pixels
	while(write < pixels_data_size) {
		data[write + 0] = 0xFF; // red
		data[write + 1] = 0x00; // green
		data[write + 2] = 0xFF; // blue
		write += 3;
	}
	return data;
}

uint8_t* GameSprite::NormalImage::getRGBAData()
{
	if(!dump) {
		if(g_settings.getInteger(Config::USE_MEMCACHED_SPRITES)) {
			return nullptr;
		}

		if(!g_editor.gfx.loadSpriteDump(dump, size, id)) {
			return nullptr;
		}
	}

	const int pixels_data_size = rme::SpritePixelsSize * 4;
	uint8_t* data = newd uint8_t[pixels_data_size];
	bool use_alpha = g_editor.gfx.hasTransparency();
	uint8_t bpp = use_alpha ? 4 : 3;
	int write = 0;
	int read = 0;

	// decompress pixels
	while(read < size && write < pixels_data_size) {
		int transparent = dump[read] | dump[read + 1] << 8;
		if(use_alpha && transparent >= rme::SpritePixelsSize) // Corrupted sprite?
			break;
		read += 2;
		for(int i = 0; i < transparent && write < pixels_data_size; i++) {
			data[write + 0] = 0x00; // red
			data[write + 1] = 0x00; // green
			data[write + 2] = 0x00; // blue
			data[write + 3] = 0x00; // alpha
			write += 4;
		}

		int colored = dump[read] | dump[read + 1] << 8;
		read += 2;
		for(int i = 0; i < colored && write < pixels_data_size; i++) {
			data[write + 0] = dump[read + 0]; // red
			data[write + 1] = dump[read + 1]; // green
			data[write + 2] = dump[read + 2]; // blue
			data[write + 3] = use_alpha ? dump[read + 3] : 0xFF; // alpha
			write += 4;
			read += bpp;
		}
	}

	// fill remaining pixels
	while(write < pixels_data_size) {
		data[write + 0] = 0x00; // red
		data[write + 1] = 0x00; // green
		data[write + 2] = 0x00; // blue
		data[write + 3] = 0x00; // alpha
		write += 4;
	}
	return data;
}

GLuint GameSprite::NormalImage::getHardwareID()
{
	if(!isGLLoaded) {
		createGLTexture(id);
	}
	visit();
	return id;
}

void GameSprite::NormalImage::createGLTexture(GLuint textureId)
{
	Image::createGLTexture(id);
}

void GameSprite::NormalImage::unloadGLTexture(GLuint textureId)
{
	Image::unloadGLTexture(id);
}

GameSprite::EditorImage::EditorImage(const wxArtID& bitmapId) :
	NormalImage(),
	bitmapId(bitmapId)
{ }

void GameSprite::EditorImage::createGLTexture(GLuint textureId)
{
	ASSERT(!isGLLoaded);

	wxSize size(rme::SpritePixels, rme::SpritePixels);
	wxBitmap bitmap = wxArtProvider::GetBitmap(bitmapId, wxART_OTHER, size);

	wxNativePixelData data(bitmap);
	if(!data) return;

	const int imageSize = rme::SpritePixelsSize * 4;
	GLubyte *imageData = new GLubyte[imageSize];
	int write = 0;

	wxNativePixelData::Iterator it(data);
	it.Offset(data, 0, 0);

	for(size_t y = 0; y < rme::SpritePixels; ++y)
	{
		wxNativePixelData::Iterator row_start = it;

		for(size_t x = 0; x < rme::SpritePixels; ++x, it++)
		{
			uint8_t red = it.Red();
			uint8_t green = it.Green();
			uint8_t blue = it.Blue();
			bool transparent = red == 0xFF && green == 0x00 && blue == 0xFF;

			imageData[write + 0] = red;
			imageData[write + 1] = green;
			imageData[write + 2] = blue;
			imageData[write + 3] = transparent ? 0x00 : 0xFF;
			write += 4;
		}

		it = row_start;
		it.OffsetY(data, 1);
	}

	isGLLoaded = true;
	id = g_editor.gfx.getFreeTextureID();
	g_editor.gfx.loaded_textures += 1;

	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear Filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear Filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 0x812F); // GL_CLAMP_TO_EDGE
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 0x812F); // GL_CLAMP_TO_EDGE
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rme::SpritePixels, rme::SpritePixels, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);

	delete[] imageData;
}

void GameSprite::EditorImage::unloadGLTexture(GLuint textureId)
{
	Image::unloadGLTexture(id);
}

GameSprite::TemplateImage::TemplateImage(GameSprite* parent, int v, const Outfit& outfit) :
	gl_tid(0),
	parent(parent),
	sprite_index(v),
	lookHead(outfit.lookHead),
	lookBody(outfit.lookBody),
	lookLegs(outfit.lookLegs),
	lookFeet(outfit.lookFeet)
{
	////
}

GameSprite::TemplateImage::~TemplateImage()
{
	////
}

void GameSprite::TemplateImage::colorizePixel(uint8_t color, uint8_t& red, uint8_t& green, uint8_t& blue)
{
	// Thanks! Khaos, or was it mips? Hmmm... =)
	uint8_t ro = (TemplateOutfitLookupTable[color] & 0xFF0000) >> 16; // rgb outfit
	uint8_t go = (TemplateOutfitLookupTable[color] & 0xFF00) >> 8;
	uint8_t bo = (TemplateOutfitLookupTable[color] & 0xFF);
	red = (uint8_t)(red * (ro / 255.f));
	green = (uint8_t)(green * (go / 255.f));
	blue = (uint8_t)(blue * (bo / 255.f));
}

uint8_t* GameSprite::TemplateImage::getRGBData()
{
	uint8_t* rgbdata = parent->spriteList[sprite_index]->getRGBData();
	uint8_t* template_rgbdata = parent->spriteList[sprite_index + parent->height * parent->width]->getRGBData();

	if(!rgbdata) {
		delete[] template_rgbdata;
		return nullptr;
	}
	if(!template_rgbdata) {
		delete[] rgbdata;
		return nullptr;
	}

	if(lookHead > (sizeof(TemplateOutfitLookupTable) / sizeof(TemplateOutfitLookupTable[0]))) {
		lookHead = 0;
	}
	if(lookBody > (sizeof(TemplateOutfitLookupTable) / sizeof(TemplateOutfitLookupTable[0]))) {
		lookBody = 0;
	}
	if(lookLegs > (sizeof(TemplateOutfitLookupTable) / sizeof(TemplateOutfitLookupTable[0]))) {
		lookLegs = 0;
	}
	if(lookFeet > (sizeof(TemplateOutfitLookupTable) / sizeof(TemplateOutfitLookupTable[0]))) {
		lookFeet = 0;
	}

	for(int y = 0; y < rme::SpritePixels; ++y) {
		for(int x = 0; x < rme::SpritePixels; ++x) {
			uint8_t& red   = rgbdata[y*rme::SpritePixels*3 + x*3 + 0];
			uint8_t& green = rgbdata[y*rme::SpritePixels*3 + x*3 + 1];
			uint8_t& blue  = rgbdata[y*rme::SpritePixels*3 + x*3 + 2];

			uint8_t& tred   = template_rgbdata[y*rme::SpritePixels*3 + x*3 + 0];
			uint8_t& tgreen = template_rgbdata[y*rme::SpritePixels*3 + x*3 + 1];
			uint8_t& tblue  = template_rgbdata[y*rme::SpritePixels*3 + x*3 + 2];

			if(tred && tgreen && !tblue) { // yellow => head
				colorizePixel(lookHead, red, green, blue);
			} else if(tred && !tgreen && !tblue) { // red => body
				colorizePixel(lookBody, red, green, blue);
			} else if(!tred && tgreen && !tblue) { // green => legs
				colorizePixel(lookLegs, red, green, blue);
			} else if(!tred && !tgreen && tblue) { // blue => feet
				colorizePixel(lookFeet, red, green, blue);
			}
		}
	}
	delete[] template_rgbdata;
	return rgbdata;
}

uint8_t* GameSprite::TemplateImage::getRGBAData()
{
	uint8_t* rgbadata = parent->spriteList[sprite_index]->getRGBAData();
	uint8_t* template_rgbdata = parent->spriteList[sprite_index + parent->height * parent->width]->getRGBData();

	if(!rgbadata) {
		delete[] template_rgbdata;
		return nullptr;
	}
	if(!template_rgbdata) {
		delete[] rgbadata;
		return nullptr;
	}

	if(lookHead > (sizeof(TemplateOutfitLookupTable) / sizeof(TemplateOutfitLookupTable[0]))) {
		lookHead = 0;
	}
	if(lookBody > (sizeof(TemplateOutfitLookupTable) / sizeof(TemplateOutfitLookupTable[0]))) {
		lookBody = 0;
	}
	if(lookLegs > (sizeof(TemplateOutfitLookupTable) / sizeof(TemplateOutfitLookupTable[0]))) {
		lookLegs = 0;
	}
	if(lookFeet > (sizeof(TemplateOutfitLookupTable) / sizeof(TemplateOutfitLookupTable[0]))) {
		lookFeet = 0;
	}

	for(int y = 0; y < rme::SpritePixels; ++y) {
		for(int x = 0; x < rme::SpritePixels; ++x) {
			uint8_t& red   = rgbadata[y*rme::SpritePixels*4 + x*4 + 0];
			uint8_t& green = rgbadata[y*rme::SpritePixels*4 + x*4 + 1];
			uint8_t& blue  = rgbadata[y*rme::SpritePixels*4 + x*4 + 2];

			uint8_t& tred   = template_rgbdata[y*rme::SpritePixels*3 + x*3 + 0];
			uint8_t& tgreen = template_rgbdata[y*rme::SpritePixels*3 + x*3 + 1];
			uint8_t& tblue  = template_rgbdata[y*rme::SpritePixels*3 + x*3 + 2];

			if(tred && tgreen && !tblue) { // yellow => head
				colorizePixel(lookHead, red, green, blue);
			} else if(tred && !tgreen && !tblue) { // red => body
				colorizePixel(lookBody, red, green, blue);
			} else if(!tred && tgreen && !tblue) { // green => legs
				colorizePixel(lookLegs, red, green, blue);
			} else if(!tred && !tgreen && tblue) { // blue => feet
				colorizePixel(lookFeet, red, green, blue);
			}
		}
	}
	delete[] template_rgbdata;
	return rgbadata;
}

GLuint GameSprite::TemplateImage::getHardwareID()
{
	if(!isGLLoaded) {
		if(gl_tid == 0) {
			gl_tid = g_editor.gfx.getFreeTextureID();
		}
		createGLTexture(gl_tid);
		if(!isGLLoaded) {
			return 0;
		}
	}
	visit();
	return gl_tid;
}

void GameSprite::TemplateImage::createGLTexture(GLuint unused)
{
	Image::createGLTexture(gl_tid);
}

void GameSprite::TemplateImage::unloadGLTexture(GLuint unused)
{
	Image::unloadGLTexture(gl_tid);
}

GameSprite* GameSprite::createFromBitmap(const wxArtID& bitmapId)
{
	GameSprite::EditorImage* image = new GameSprite::EditorImage(bitmapId);

	GameSprite* sprite = new GameSprite();
	sprite->width = 1;
	sprite->height = 1;
	sprite->layers = 1;
	sprite->pattern_x = 1;
	sprite->pattern_y = 1;
	sprite->pattern_z = 1;
	sprite->frames = 1;
	sprite->numsprites = 1;
	sprite->spriteList.push_back(image);
	return sprite;
}

// ============================================================================
// Animator

Animator::Animator(int frame_count, int start_frame, int loop_count, bool async) :
	frame_count(frame_count),
	start_frame(start_frame),
	loop_count(loop_count),
	async(async),
	current_frame(0),
	current_loop(0),
	current_duration(0),
	total_duration(0),
	direction(ANIMATION_FORWARD),
	last_time(0),
	is_complete(false)
{
	ASSERT(start_frame >= -1 && start_frame < frame_count);

	for(int i = 0; i < frame_count; i++) {
		durations.push_back(newd FrameDuration(ITEM_FRAME_DURATION, ITEM_FRAME_DURATION));
	}

	reset();
}

Animator::~Animator()
{
	for(int i = 0; i < frame_count; i++) {
		delete durations[i];
	}
	durations.clear();
}

int Animator::getStartFrame() const
{
	if(start_frame > -1)
		return start_frame;
	return uniform_random(0, frame_count - 1);
}

FrameDuration* Animator::getFrameDuration(int frame)
{
	ASSERT(frame >= 0 && frame < frame_count);
	return durations[frame];
}

int Animator::getFrame()
{
	long time = g_editor.gfx.getElapsedTime();
	if(time != last_time && !is_complete) {
		long elapsed = time - last_time;
		if(elapsed >= current_duration) {
			int frame = 0;
			if(loop_count < 0)
				frame = getPingPongFrame();
			else
				frame = getLoopFrame();

			if(current_frame != frame) {
				int duration = getDuration(frame) - (elapsed - current_duration);
				if(duration < 0 && !async) {
					calculateSynchronous();
				} else {
					current_frame = frame;
					current_duration = std::max<int>(0, duration);
				}
			} else {
				is_complete = true;
			}
		} else {
			current_duration -= elapsed;
		}

		last_time = time;
	}
	return current_frame;
}

void Animator::setFrame(int frame)
{
	ASSERT(frame == -1 || frame == 255 || frame == 254 || (frame >= 0 && frame < frame_count));

	if(current_frame == frame)
		return;

	if(async) {
		if(frame == 255) // Async mode
			current_frame = 0;
		else if(frame == 254) // Random mode
			current_frame = uniform_random(0, frame_count - 1);
		else if(frame >= 0 && frame < frame_count)
			current_frame = frame;
		else
			current_frame = getStartFrame();

		is_complete = false;
		last_time = g_editor.gfx.getElapsedTime();
		current_duration = getDuration(current_frame);
		current_loop = 0;
	} else {
		calculateSynchronous();
	}
}

void Animator::reset()
{
	total_duration = 0;
	for(int i = 0; i < frame_count; i++)
		total_duration += durations[i]->max;

	is_complete = false;
	direction = ANIMATION_FORWARD;
	current_loop = 0;
	async = false;
	setFrame(-1);
}

int Animator::getDuration(int frame) const
{
	ASSERT(frame >= 0 && frame < frame_count);
	return durations[frame]->getDuration();
}

int Animator::getPingPongFrame()
{
	int count = direction == ANIMATION_FORWARD ? 1 : -1;
	int next_frame = current_frame + count;
	if(next_frame < 0 || next_frame >= frame_count) {
		direction = direction == ANIMATION_FORWARD ? ANIMATION_BACKWARD : ANIMATION_FORWARD;
		count *= -1;
	}
	return current_frame + count;
}

int Animator::getLoopFrame()
{
	int next_phase = current_frame + 1;
	if(next_phase < frame_count)
		return next_phase;

	if(loop_count == 0)
		return 0;

	if(current_loop < (loop_count - 1)) {
		current_loop++;
		return 0;
	}
	return current_frame;
}

void Animator::calculateSynchronous()
{
	long time = g_editor.gfx.getElapsedTime();
	if(time > 0 && total_duration > 0) {
		long elapsed = time % total_duration;
		int total_time = 0;
		for(int i = 0; i < frame_count; i++) {
			int duration = getDuration(i);
			if(elapsed >= total_time && elapsed < total_time + duration) {
				current_frame = i;
				current_duration = duration - (elapsed - total_time);
				break;
			}
			total_time += duration;
		}
		last_time = time;
	}
}
