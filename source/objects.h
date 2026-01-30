#ifndef RME_OBJECTS_H_
#define RME_OBJECTS_H_ 1

#include "main.h"
#include "bitset.h"

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

// ObjectType
//==============================================================================
struct ObjectType {
	const char *name;
	//const char *plural; // TODO?
	const char *description;
	int typeId = 0;
	BitSet<NUM_FLAGS> flags = {};
	int attributes[NUM_TYPE_ATTRIBUTES] = {};
	int attributeOffsets[NUM_INSTANCE_ATTRIBUTES] = {};

	// editor related:
	// brush, etc....

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
bool ObjectTypeExists(int typeId);
const ObjectType *GetObjectType(int type);
void LoadObjects(const char *filename);

// Object
//==============================================================================
struct Object {
	Object *container;
	Object *next;
	int typeId;
	int attributes[4];

	bool getFlag(ObjectFlag flag) const;
	int getAttribute(ObjectTypeAttribute attr) const;
	int getAttribute(ObjectInstanceAttribute attr) const;
};

//Object *CreateObject(int type);
//void DeleteObject(Object *obj);

// TODO(fusion): Ok... this works for strings but what about CONTENT? Probably
// have a separate table too?
//Object *GetFirstContainerObject(int containerId);
//std::string &GetStringAttribute(int stringId);

#endif //RME_OBJECTS_H_
