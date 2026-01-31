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

#ifndef RME_ITEMS_H_
#define RME_ITEMS_H_

#include "bitset.h"
#include "filehandle.h"
#include "brush_enums.h"

class Brush;
class GroundBrush;
class WallBrush;
class CarpetBrush;
class TableBrush;
class HouseBrush;
class HouseExitBrush;
class OptionalBorderBrush;
class EraserBrush;
class SpawnBrush;
class DoorBrush;
class FlagBrush;
class RAWBrush;

class GameSprite;
class ItemDatabase;

// NOTE(fusion): We could add prefixes to these enums but it would make it a lot
// more verbose and I don't think there is any risk of name collisions. We can
// probably review this if it becomes a problem (probably won't).

enum ObjectFlag {
	BANK					= 0,
	CLIP					= 1,
	BOTTOM					= 2,
	TOP						= 3,
	CONTAINER				= 4,
	CHEST					= 5,
	CUMULATIVE				= 6,
	USEEVENT				= 7,
	CHANGEUSE				= 8,
	FORCEUSE				= 9,
	MULTIUSE				= 10,
	DISTUSE					= 11,
	MOVEMENTEVENT			= 12,
	COLLISIONEVENT			= 13,
	SEPARATIONEVENT			= 14,
	KEY						= 15,
	KEYDOOR					= 16,
	NAMEDOOR				= 17,
	LEVELDOOR				= 18,
	QUESTDOOR				= 19,
	BED						= 20,
	FOOD					= 21,
	RUNE					= 22,
	INFORMATION				= 23,
	TEXT					= 24,
	WRITE					= 25,
	WRITEONCE				= 26,
	LIQUIDCONTAINER			= 27,
	LIQUIDSOURCE			= 28,
	LIQUIDPOOL				= 29,
	TELEPORTABSOLUTE		= 30,
	TELEPORTRELATIVE		= 31,
	UNPASS					= 32,
	UNMOVE					= 33,
	UNTHROW					= 34,
	UNLAY					= 35,
	AVOID					= 36,
	MAGICFIELD				= 37,
	RESTRICTLEVEL			= 38,
	RESTRICTPROFESSION		= 39,
	TAKE					= 40,
	HANG					= 41,
	HOOKSOUTH				= 42,
	HOOKEAST				= 43,
	ROTATE					= 44,
	DESTROY					= 45,
	CLOTHES					= 46,
	SKILLBOOST				= 47,
	PROTECTION				= 48,
	LIGHT					= 49,
	ROPESPOT				= 50,
	CORPSE					= 51,
	EXPIRE					= 52,
	EXPIRESTOP				= 53,
	WEAROUT					= 54,
	WEAPON					= 55,
	SHIELD					= 56,
	BOW						= 57,
	THROW					= 58,
	WAND					= 59,
	AMMO					= 60,
	ARMOR					= 61,
	HEIGHT					= 62,
	DISGUISE				= 63,
	SHOWDETAIL				= 64,
	SPECIALOBJECT			= 65,
	NUM_FLAGS				= 66,
};

enum ObjectTypeAttribute {
	WAYPOINTS				= 0,
	CAPACITY				= 1,
	CHANGETARGET			= 2,
	KEYDOORTARGET			= 3,
	NAMEDOORTARGET			= 4,
	LEVELDOORTARGET			= 5,
	QUESTDOORTARGET			= 6,
	NUTRITION				= 7,
	INFORMATIONTYPE			= 8,
	FONTSIZE				= 9,
	MAXLENGTH				= 10,
	MAXLENGTHONCE			= 11,
	SOURCELIQUIDTYPE		= 12,
	ABSTELEPORTEFFECT		= 13,
	RELTELEPORTDISPLACEMENT	= 14,
	RELTELEPORTEFFECT		= 15,
	AVOIDDAMAGETYPES		= 16,
	MINIMUMLEVEL			= 17,
	PROFESSIONS				= 18,
	WEIGHT					= 19,
	ROTATETARGET			= 20,
	DESTROYTARGET			= 21,
	BODYPOSITION			= 22,
	SKILLNUMBER				= 23,
	SKILLMODIFICATION		= 24,
	PROTECTIONDAMAGETYPES	= 25,
	DAMAGEREDUCTION			= 26,
	BRIGHTNESS				= 27,
	LIGHTCOLOR				= 28,
	CORPSETYPE				= 29,
	TOTALEXPIRETIME			= 30,
	EXPIRETARGET			= 31,
	TOTALUSES				= 32,
	WEAROUTTARGET			= 33,
	WEAPONTYPE				= 34,
	WEAPONATTACKVALUE		= 35,
	WEAPONDEFENDVALUE		= 36,
	SHIELDDEFENDVALUE		= 37,
	BOWRANGE				= 38,
	BOWAMMOTYPE				= 39,
	THROWRANGE				= 40,
	THROWATTACKVALUE		= 41,
	THROWDEFENDVALUE		= 42,
	THROWMISSILE			= 43,
	THROWSPECIALEFFECT		= 44,
	THROWEFFECTSTRENGTH		= 45,
	THROWFRAGILITY			= 46,
	WANDRANGE				= 47,
	WANDMANACONSUMPTION		= 48,
	WANDATTACKSTRENGTH		= 49,
	WANDATTACKVARIATION		= 50,
	WANDDAMAGETYPE			= 51,
	WANDMISSILE				= 52,
	AMMOTYPE				= 53,
	AMMOATTACKVALUE			= 54,
	AMMOMISSILE				= 55,
	AMMOSPECIALEFFECT		= 56,
	AMMOEFFECTSTRENGTH		= 57,
	ARMORVALUE				= 58,
	ELEVATION				= 59,
	DISGUISETARGET			= 60,
	MEANING					= 61,
	NUM_TYPE_ATTRIBUTES		= 62,
};

enum ObjectInstanceAttribute {
	CONTENT					= 0,
	CHESTQUESTNUMBER		= 1,
	AMOUNT					= 2,
	KEYNUMBER				= 3,
	KEYHOLENUMBER			= 4,
	DOORLEVEL				= 5,
	DOORQUESTNUMBER			= 6,
	DOORQUESTVALUE			= 7,
	CHARGES					= 8,
	TEXTSTRING				= 9,
	EDITOR					= 10,
	CONTAINERLIQUIDTYPE		= 11,
	POOLLIQUIDTYPE			= 12,
	ABSTELEPORTDESTINATION	= 13,
	RESPONSIBLE				= 14,
	REMAININGEXPIRETIME		= 15,
	SAVEDEXPIRETIME			= 16,
	REMAININGUSES			= 17,
	NUM_INSTANCE_ATTRIBUTES	= 18,
};

class ItemType {
public:
	ItemType(void);

public:
	// editor related
	GameSprite	*sprite = nullptr;
	Brush		*brush = nullptr;
	Brush		*doodad_brush = nullptr;
	RAWBrush	*raw_brush = nullptr;
	bool		is_metaitem = false;
	// This is needed as a consequence of the item palette & the raw palette
	// using the same brushes ("others" category consists of items with this
	// flag set to false)
	bool		has_raw = false;
	bool		in_other_tileset = false;
	uint16_t	ground_equivalent = 0;
	uint32_t	border_group = 0;
	bool		has_equivalent = false; // True if any item has this as ground_equivalent
	bool		wall_hate_me = false; // (For wallbrushes, regard this as not part of the wall)
	bool		isBorder = false;
	bool		isOptionalBorder = false;
	bool		isWall = false;
	bool		isBrushDoor = false;
	bool		isOpen = false;
	bool		isTable = false;
	bool		isCarpet = false;
	BorderType	border_alignment = BORDER_NONE;

public:
	uint16_t	typeId = 0;
	BitSet<NUM_FLAGS> flags = {};
	int			attributes[NUM_TYPE_ATTRIBUTES] = {};
	int			attributeOffsets[NUM_INSTANCE_ATTRIBUTES] = {};
	std::string	name = {};
	std::string	description = {};

	bool getFlag(ObjectFlag flag) const;
	int getAttribute(ObjectTypeAttribute attr) const;
	int getAttributeOffset(ObjectInstanceAttribute attr) const;
};

int GetFlagByName(const char *name);
int GetTypeAttributeByName(const char *name);
int GetInstanceAttributeByName(const char *name);
const char *GetFlagName(int flag);
const char *GetTypeAttributeName(int attr);
const char *GetInstanceAttributeName(int attr);
bool ItemTypeExists(uint16_t typeId);
int GetMaxItemTypeId(void);
const ItemType &GetItemType(uint16_t typeId);
ItemType *GetMutableItemType(uint16_t typeId);
bool LoadItemTypes(const char *filename, wxString &outError, wxArrayString &outWarnings);

#endif
