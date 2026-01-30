#include "objects.h"

static ObjectFlag g_typeAttrFlags[NUM_TYPE_ATTRIBUTES] = {
	BANK,					// WAYPOINTS
	CONTAINER,				// CAPACITY
	CHANGEUSE,				// CHANGETARGET
	KEYDOOR,				// KEYDOORTARGET
	NAMEDOOR,				// NAMEDOORTARGET
	LEVELDOOR,				// LEVELDOORTARGET
	QUESTDOOR,				// QUESTDOORTARGET
	FOOD,					// NUTRITION
	INFORMATION,			// INFORMATIONTYPE
	TEXT,					// FONTSIZE
	WRITE,					// MAXLENGTH
	WRITEONCE,				// MAXLENGTHONCE
	LIQUIDSOURCE,			// SOURCELIQUIDTYPE
	TELEPORTABSOLUTE,		// ABSTELEPORTEFFECT
	TELEPORTRELATIVE,		// RELTELEPORTDISPLACEMENT
	TELEPORTRELATIVE,		// RELTELEPORTEFFECT
	AVOID,					// AVOIDDAMAGETYPES
	RESTRICTLEVEL,			// MINIMUMLEVEL
	RESTRICTPROFESSION,		// PROFESSIONS
	TAKE,					// WEIGHT
	ROTATE,					// ROTATETARGET
	DESTROY,				// DESTROYTARGET
	CLOTHES,				// BODYPOSITION
	SKILLBOOST,				// SKILLNUMBER
	SKILLBOOST,				// SKILLMODIFICATION
	PROTECTION,				// PROTECTIONDAMAGETYPES
	PROTECTION,				// DAMAGEREDUCTION
	LIGHT,					// BRIGHTNESS
	LIGHT,					// LIGHTCOLOR
	CORPSE,					// CORPSETYPE
	EXPIRE,					// TOTALEXPIRETIME
	EXPIRE,					// EXPIRETARGET
	WEAROUT,				// TOTALUSES
	WEAROUT,				// WEAROUTTARGET
	WEAPON,					// WEAPONTYPE
	WEAPON,					// WEAPONATTACKVALUE
	WEAPON,					// WEAPONDEFENDVALUE
	SHIELD,					// SHIELDDEFENDVALUE
	BOW,					// BOWRANGE
	BOW,					// BOWAMMOTYPE
	THROW,					// THROWRANGE
	THROW,					// THROWATTACKVALUE
	THROW,					// THROWDEFENDVALUE
	THROW,					// THROWMISSILE
	THROW,					// THROWSPECIALEFFECT
	THROW,					// THROWEFFECTSTRENGTH
	THROW,					// THROWFRAGILITY
	WAND,					// WANDRANGE
	WAND,					// WANDMANACONSUMPTION
	WAND,					// WANDATTACKSTRENGTH
	WAND,					// WANDATTACKVARIATION
	WAND,					// WANDDAMAGETYPE
	WAND,					// WANDMISSILE
	AMMO,					// AMMOTYPE
	AMMO,					// AMMOATTACKVALUE
	AMMO,					// AMMOMISSILE
	AMMO,					// AMMOSPECIALEFFECT
	AMMO,					// AMMOEFFECTSTRENGTH
	ARMOR,					// ARMORVALUE
	HEIGHT,					// ELEVATION
	DISGUISE,				// DISGUISETARGET
	BANK,					// MEANING
};

static ObjectFlag g_instanceAttrFlags[NUM_INSTANCE_ATTRIBUTES] = {
	CONTAINER,					// CONTENT
	CHEST,						// CHESTQUESTNUMBER
	CUMULATIVE,					// AMOUNT
	KEY,						// KEYNUMBER
	KEYDOOR,					// KEYHOLENUMBER
	LEVELDOOR,					// DOORLEVEL
	QUESTDOOR,					// DOORQUESTNUMBER
	QUESTDOOR,					// DOORQUESTVALUE
	RUNE,						// CHARGES
	TEXT,						// TEXTSTRING
	TEXT,						// EDITOR
	LIQUIDCONTAINER,			// CONTAINERLIQUIDTYPE
	LIQUIDPOOL,					// POOLLIQUIDTYPE
	TELEPORTABSOLUTE,			// ABSTELEPORTDESTINATION
	MAGICFIELD,					// RESPONSIBLE
	EXPIRE,						// REMAININGEXPIRETIME
	EXPIRESTOP,					// SAVEDEXPIRETIME
	WEAROUT,					// REMAININGUSES
};

static const char g_flagNames[NUM_FLAGS][30] = {
	"Bank",						// BANK
	"Clip",						// CLIP
	"Bottom",					// BOTTOM
	"Top",						// TOP
	"Container",				// CONTAINER
	"Chest",					// CHEST
	"Cumulative",				// CUMULATIVE
	"UseEvent",					// USEEVENT
	"ChangeUse",				// CHANGEUSE
	"ForceUse",					// FORCEUSE
	"MultiUse",					// MULTIUSE
	"DistUse",					// DISTUSE
	"MovementEvent",			// MOVEMENTEVENT
	"CollisionEvent",			// COLLISIONEVENT
	"SeparationEvent",			// SEPARATIONEVENT
	"Key",						// KEY
	"KeyDoor",					// KEYDOOR
	"NameDoor",					// NAMEDOOR
	"LevelDoor",				// LEVELDOOR
	"QuestDoor",				// QUESTDOOR
	"Bed",						// BED
	"Food",						// FOOD
	"Rune",						// RUNE
	"Information",				// INFORMATION
	"Text",						// TEXT
	"Write",					// WRITE
	"WriteOnce",				// WRITEONCE
	"LiquidContainer",			// LIQUIDCONTAINER
	"LiquidSource",				// LIQUIDSOURCE
	"LiquidPool",				// LIQUIDPOOL
	"TeleportAbsolute",			// TELEPORTABSOLUTE
	"TeleportRelative",			// TELEPORTRELATIVE
	"Unpass",					// UNPASS
	"Unmove",					// UNMOVE
	"Unthrow",					// UNTHROW
	"Unlay",					// UNLAY
	"Avoid",					// AVOID
	"MagicField",				// MAGICFIELD
	"RestrictLevel",			// RESTRICTLEVEL
	"RestrictProfession",		// RESTRICTPROFESSION
	"Take",						// TAKE
	"Hang",						// HANG
	"HookSouth",				// HOOKSOUTH
	"HookEast",					// HOOKEAST
	"Rotate",					// ROTATE
	"Destroy",					// DESTROY
	"Clothes",					// CLOTHES
	"SkillBoost",				// SKILLBOOST
	"Protection",				// PROTECTION
	"Light",					// LIGHT
	"RopeSpot",					// ROPESPOT
	"Corpse",					// CORPSE
	"Expire",					// EXPIRE
	"ExpireStop",				// EXPIRESTOP
	"WearOut",					// WEAROUT
	"Weapon",					// WEAPON
	"Shield",					// SHIELD
	"Bow",						// BOW
	"Throw",					// THROW
	"Wand",						// WAND
	"Ammo",						// AMMO
	"Armor",					// ARMOR
	"Height",					// HEIGHT
	"Disguise",					// DISGUISE
	"ShowDetail",				// SHOWDETAIL
	"Special",					// SPECIALOBJECT
};

static const char g_typeAttrNames[NUM_TYPE_ATTRIBUTES][30] = {
	"Waypoints",					// WAYPOINTS
	"Capacity",						// CAPACITY
	"ChangeTarget",					// CHANGETARGET
	"KeydoorTarget",				// KEYDOORTARGET
	"NamedoorTarget",				// NAMEDOORTARGET
	"LeveldoorTarget",				// LEVELDOORTARGET
	"QuestdoorTarget",				// QUESTDOORTARGET
	"Nutrition",					// NUTRITION
	"InformationType",				// INFORMATIONTYPE
	"FontSize",						// FONTSIZE
	"MaxLength",					// MAXLENGTH
	"MaxLengthOnce",				// MAXLENGTHONCE
	"SourceLiquidType",				// SOURCELIQUIDTYPE
	"AbsTeleportEffect",			// ABSTELEPORTEFFECT
	"RelTeleportDisplacement",		// RELTELEPORTDISPLACEMENT
	"RelTeleportEffect",			// RELTELEPORTEFFECT
	"AvoidDamageTypes",				// AVOIDDAMAGETYPES
	"MinimumLevel",					// MINIMUMLEVEL
	"Professions",					// PROFESSIONS
	"Weight",						// WEIGHT
	"RotateTarget",					// ROTATETARGET
	"DestroyTarget",				// DESTROYTARGET
	"BodyPosition",					// BODYPOSITION
	"SkillNumber",					// SKILLNUMBER
	"SkillModification",			// SKILLMODIFICATION
	"ProtectionDamageTypes",		// PROTECTIONDAMAGETYPES
	"DamageReduction",				// DAMAGEREDUCTION
	"Brightness",					// BRIGHTNESS
	"LightColor",					// LIGHTCOLOR
	"CorpseType",					// CORPSETYPE
	"TotalExpireTime",				// TOTALEXPIRETIME
	"ExpireTarget",					// EXPIRETARGET
	"TotalUses",					// TOTALUSES
	"WearoutTarget",				// WEAROUTTARGET
	"WeaponType",					// WEAPONTYPE
	"WeaponAttackValue",			// WEAPONATTACKVALUE
	"WeaponDefendValue",			// WEAPONDEFENDVALUE
	"ShieldDefendValue",			// SHIELDDEFENDVALUE
	"BowRange",						// BOWRANGE
	"BowAmmoType",					// BOWAMMOTYPE
	"ThrowRange",					// THROWRANGE
	"ThrowAttackValue",				// THROWATTACKVALUE
	"ThrowDefendValue",				// THROWDEFENDVALUE
	"ThrowMissile",					// THROWMISSILE
	"ThrowSpecialEffect",			// THROWSPECIALEFFECT
	"ThrowEffectStrength",			// THROWEFFECTSTRENGTH
	"ThrowFragility",				// THROWFRAGILITY
	"WandRange",					// WANDRANGE
	"WandManaConsumption",			// WANDMANACONSUMPTION
	"WandAttackStrength",			// WANDATTACKSTRENGTH
	"WandAttackVariation",			// WANDATTACKVARIATION
	"WandDamageType",				// WANDDAMAGETYPE
	"WandMissile",					// WANDMISSILE
	"AmmoType",						// AMMOTYPE
	"AmmoAttackValue",				// AMMOATTACKVALUE
	"AmmoMissile",					// AMMOMISSILE
	"AmmoSpecialEffect",			// AMMOSPECIALEFFECT
	"AmmoEffectStrength",			// AMMOEFFECTSTRENGTH
	"ArmorValue",					// ARMORVALUE
	"Elevation",					// ELEVATION
	"DisguiseTarget",				// DISGUISETARGET
	"Meaning",						// MEANING
};

static const char g_instanceAttrNames[NUM_INSTANCE_ATTRIBUTES][30] = {
	"Content",						// CONTENT
	"ChestQuestNumber",				// CHESTQUESTNUMBER
	"Amount",						// AMOUNT
	"KeyNumber",					// KEYNUMBER
	"KeyholeNumber",				// KEYHOLENUMBER
	"Level",						// DOORLEVEL
	"DoorQuestNumber",				// DOORQUESTNUMBER
	"DoorQuestValue",				// DOORQUESTVALUE
	"Charges",						// CHARGES
	"String",						// TEXTSTRING
	"Editor",						// EDITOR
	"ContainerLiquidType",			// CONTAINERLIQUIDTYPE
	"PoolLiquidType",				// POOLLIQUIDTYPE
	"AbsTeleportDestination",		// ABSTELEPORTDESTINATION
	"Responsible",					// RESPONSIBLE
	"RemainingExpireTime",			// REMAININGEXPIRETIME
	"SavedExpireTime",				// SAVEDEXPIRETIME
	"RemainingUses",				// REMAININGUSES
};

// ObjectType
//==============================================================================
static std::vector<ObjectType> g_objectTypes;

bool ObjectType::getFlag(ObjectFlag flag) const {
	return flags.check((int)flag);
}

int ObjectType::getAttribute(ObjectTypeAttribute attr) const {
	if(!getFlag(g_typeAttrFlags[attr])){
		// TODO(fusion): It's unclear how to log these kind of errors...
		std::cout << "Object type " << typeId << " is missing flag "
				<< g_typeAttrFlags[attr] << " for type attribute "
				<< attr << std::endl;
		return 0;
	}
	return attributes[attr];
}

int ObjectType::getAttributeOffset(ObjectInstanceAttribute attr) const {
	if(!getFlag(g_instanceAttrFlags[attr])){
		// NOTE(fusion): The CONTENT attribute seems to be the only one that maps
		// to two attribute flags, which is why we have this extra check here.
		if(!(attr == CONTENT && getFlag(CHEST))){
			return -1;
		}
	}
	return attributeOffsets[attr];
}

int GetFlagByName(const char *name){
	int Result = -1;
	for(int i = 0; i < NUM_FLAGS; i += 1){
		if(strcmp_ci(g_flagNames[i], name) == 0){
			Result = i;
			break;
		}
	}
	return Result;
}

int GetTypeAttributeByName(const char *name){
	int Result = -1;
	for(int i = 0; i < NUM_TYPE_ATTRIBUTES; i += 1){
		if(strcmp_ci(g_typeAttrNames[i], name) == 0){
			Result = i;
			break;
		}
	}
	return Result;
}

int GetInstanceAttributeByName(const char *name){
	int Result = -1;
	for(int i = 0; i < NUM_INSTANCE_ATTRIBUTES; i += 1){
		if(strcmp_ci(g_instanceAttrNames[i], name) == 0){
			Result = i;
			break;
		}
	}
	return Result;
}

const char *GetFlagName(int flag){
	ASSERT(flag >= 0 && flag <= NUM_FLAGS);
	return g_flagNames[flag];
}

const char *GetTypeAttributeName(int attr){
	ASSERT(attr >= 0 && attr <= NUM_TYPE_ATTRIBUTES);
	return g_typeAttrNames[attr];
}

const char *GetInstanceAttributeName(int attr){
	ASSERT(attr >= 0 && attr <= NUM_INSTANCE_ATTRIBUTES);
	return g_instanceAttrNames[attr];
}

bool ObjectTypeExists(int typeId){
	return typeId >= 0 && typeId <= (int)g_objectTypes.size()
		&& g_objectTypes[typeId].typeId == typeId;
}

const ObjectType *GetObjectType(int typeId){
	static const ObjectType dummy = {};
	if(ObjectTypeExists(typeId)){
		return &g_objectTypes[typeId];
	}else{
		return &dummy;
	}
}

void LoadObjects(const char *filename){
	// Then load dat+spr ? Actually dat+spr is only for drawing, unless we want
	// to do a consistency check too, which could probably just be thrown into
	// the dat loader.
}

// Object
//==============================================================================
//static int g_ObjectContentsCounter = 1;
//static std::unordered_map<int, Object*>     g_ObjectContents;

//static int g_ObjectStringsCounter = 1;
//static std::unordered_map<int, std::string> g_ObjectStrings;

bool Object::getFlag(ObjectFlag flag) const {
	return GetObjectType(typeId)->getFlag(flag);
}

int Object::getAttribute(ObjectTypeAttribute attr) const {
	return GetObjectType(typeId)->getAttribute(attr);
}

int Object::getAttribute(ObjectInstanceAttribute attr) const {
	int attrOffset = GetObjectType(typeId)->getAttributeOffset(attr);
	if(attrOffset == -1){
		std::cout << "Object type " << typeId << " is missing flag "
				<< g_instanceAttrFlags[attr] << " for instance attribute "
				<< attr << std::endl;
		return 0;
	}

	if(attrOffset < 0 || attrOffset >= NARRAY(attributes)){
		std::cout << "Object type " << typeId
				<< " has invalid instance attribute offset "
				<< attrOffset << std::endl;
		return 0;
	}

	return attributes[attrOffset];
}

Object *CreateObject(int typeId){
	if(!ObjectTypeExists(typeId)){
		std::cout << "Invalid object type " << typeId << std::endl;
		return NULL;
	}

	Object *obj = newd Object;
	obj->typeId = typeId;
	return obj;
}

void DestroyObject(Object *obj){
	// unlink obj
	// delete strings
	// delete contents
	// etc...
	delete obj;
}

