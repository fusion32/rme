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

#include "materials.h"
#include "editor.h"
#include "script.h"

#include "items.h"
#include "item.h"

static std::vector<ItemType> g_itemTypes = {};

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

ItemType::ItemType(void) {
	for(int &attrOffset: attributeOffsets){
		attrOffset = -1;
	}
}

bool ItemType::getFlag(ObjectFlag flag) const {
	return flags.check((int)flag);
}

int ItemType::getAttribute(ObjectTypeAttribute attr) const {
	if(!getFlag(g_typeAttrFlags[attr])){
		// TODO(fusion): It's unclear how to log these kind of errors...
		std::cout << "Object type " << typeId << " is missing flag "
				<< g_typeAttrFlags[attr] << " for type attribute "
				<< attr << std::endl;
		return 0;
	}
	return attributes[attr];
}

int ItemType::getAttributeOffset(ObjectInstanceAttribute attr) const {
	if(!getFlag(g_instanceAttrFlags[attr])){
		// NOTE(fusion): The CONTENT attribute seems to be the only one that maps
		// to two attribute flags, which is why we have this extra check here.
		if(!(attr == CONTENT && getFlag(CHEST))){
			return -1;
		}
	}
	return attributeOffsets[attr];
}

int ItemType::getStackPriority(void) const {
	if(getFlag(BANK))   return STACK_PRIORITY_BANK;
	if(getFlag(CLIP))   return STACK_PRIORITY_CLIP;
	if(getFlag(BOTTOM)) return STACK_PRIORITY_BOTTOM;
	if(getFlag(TOP))    return STACK_PRIORITY_TOP;
	return STACK_PRIORITY_LOW;
}

int GetFlagByName(const char *name){
	int result = -1;
	for(int i = 0; i < NUM_FLAGS; i += 1){
		if(strcmp_ci(g_flagNames[i], name) == 0){
			result = i;
			break;
		}
	}
	return result;
}

int GetTypeAttributeByName(const char *name){
	int result = -1;
	for(int i = 0; i < NUM_TYPE_ATTRIBUTES; i += 1){
		if(strcmp_ci(g_typeAttrNames[i], name) == 0){
			result = i;
			break;
		}
	}
	return result;
}

int GetInstanceAttributeByName(const char *name){
	int result = -1;
	for(int i = 0; i < NUM_INSTANCE_ATTRIBUTES; i += 1){
		if(strcmp_ci(g_instanceAttrNames[i], name) == 0){
			result = i;
			break;
		}
	}
	return result;
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

const char *GetLiquidName(int liquidType){
	const char *liquidName = "unknown";
	switch(liquidType) {
		case LIQUID_NONE:		liquidName = "nothing"; break;
		case LIQUID_WATER:		liquidName = "water"; break;
		case LIQUID_WINE:		liquidName = "wine"; break;
		case LIQUID_BEER:		liquidName = "beer"; break;
		case LIQUID_MUD:		liquidName = "mud"; break;
		case LIQUID_BLOOD:		liquidName = "blood"; break;
		case LIQUID_SLIME:		liquidName = "slime"; break;
		case LIQUID_OIL:		liquidName = "oil"; break;
		case LIQUID_URINE:		liquidName = "urine"; break;
		case LIQUID_MILK:		liquidName = "milk"; break;
		case LIQUID_MANA:		liquidName = "manafluid"; break;
		case LIQUID_LIFE:		liquidName = "lifefluid"; break;
		case LIQUID_LEMONADE:	liquidName = "lemonade"; break;
	}
	return liquidName;
}

int GetLiquidColor(int liquidType){
	int liquidColor = LIQUID_COLORLESS;
	switch(liquidType){
		case LIQUID_NONE:		liquidColor = LIQUID_COLORLESS; break;
		case LIQUID_WATER:		liquidColor = LIQUID_BLUE; break;
		case LIQUID_WINE:		liquidColor = LIQUID_PURPLE; break;
		case LIQUID_BEER:		liquidColor = LIQUID_BROWN; break;
		case LIQUID_MUD:		liquidColor = LIQUID_BROWN; break;
		case LIQUID_BLOOD:		liquidColor = LIQUID_RED; break;
		case LIQUID_SLIME:		liquidColor = LIQUID_GREEN; break;
		case LIQUID_OIL:		liquidColor = LIQUID_BROWN; break;
		case LIQUID_URINE:		liquidColor = LIQUID_YELLOW; break;
		case LIQUID_MILK:		liquidColor = LIQUID_WHITE; break;
		case LIQUID_MANA:		liquidColor = LIQUID_PURPLE; break;
		case LIQUID_LIFE:		liquidColor = LIQUID_RED; break;
		case LIQUID_LEMONADE:	liquidColor = LIQUID_YELLOW; break;
	}
	return liquidColor;
}

int GetMinItemTypeId(void){
	return 0;
}

int GetMaxItemTypeId(void){
	return (int)g_itemTypes.size() - 1;
}

bool ItemTypeExists(uint16_t typeId){
	return typeId >= GetMinItemTypeId()
		&& typeId <= GetMaxItemTypeId()
		&& g_itemTypes[typeId].typeId == typeId;
}

const ItemType &GetItemType(uint16_t typeId){
	static const ItemType dummy = {};
	if(ItemTypeExists(typeId)){
		return g_itemTypes[typeId];
	}else{
		return dummy;
	}
}

ItemType *GetMutableItemType(uint16_t typeId){
	if(ItemTypeExists(typeId)){
		return &g_itemTypes[typeId];
	}else{
		return NULL;
	}
}

static ItemType *GetOrCreateItemType(uint16_t typeId){
	constexpr size_t MIN_CAPACITY = 1024;
	constexpr size_t MAX_CAPACITY = UINT16_MAX + 1;
	size_t requiredSize = typeId + 1;
	if(requiredSize > g_itemTypes.size()){
		// NOTE(fusion): The behaviour of `vector::resize` depends on the
		// implementation, but we can make sure it grows exponentially by
		// calling `vector::reserve` before resizing.
		size_t capacity = g_itemTypes.capacity();
		if(requiredSize > capacity){
			if(capacity < MIN_CAPACITY)    capacity = MIN_CAPACITY;
			while(requiredSize > capacity) capacity += capacity / 2;
			if(capacity > MAX_CAPACITY)    capacity = MAX_CAPACITY;
			ASSERT(requiredSize <= capacity);
			g_itemTypes.reserve(capacity);
		}
		g_itemTypes.resize(requiredSize);
	}
	g_itemTypes[typeId].typeId = typeId;
	return &g_itemTypes[typeId];
}

bool LoadItemTypes(const wxString &projectDir, wxString &outError, wxArrayString &outWarnings){
	wxString filename;
	{
		wxPathList paths;
		paths.Add(projectDir);
		paths.Add(projectDir + "editor");
		paths.Add(projectDir + "dat");
		filename = paths.FindValidPath("objects.srv");
		if(filename.IsEmpty()){
			outError << "Unable to locate objects.srv";
			return false;
		}
	}

	int typeId = -1;
	std::string ident;
	Script script(filename.mb_str());
	while(true){
		script.nextToken();
		if(script.eof()){
			break;
		}

		ident = script.getIdentifier();
		script.readSymbol('=');
		if(ident == "typeid"){
			typeId = script.readNumber();
			GetOrCreateItemType(typeId);
		}else if(ItemType *itemType = GetMutableItemType(typeId)){
			if(ident == "name"){
				itemType->name = script.readString();
			}else if(ident == "description"){
				itemType->description = script.readString();
			}else if(ident == "flags"){
				script.readSymbol('{');
				while(!script.eof()){
					script.nextToken();
					if(script.token.kind == TOKEN_SPECIAL){
						char special = script.getSpecial();
						if(special == ',') continue;
						if(special == '}') break;
					}

					int flag = GetFlagByName(script.getIdentifier());
					if(flag == -1){
						script.error("unknown flag");
						break;
					}

					itemType->flags.set(flag);
				}

				// NOTE(fusion): Assign instance attribute offsets after parsing flags.
				int instanceAttrCount = 0;
				for(int instanceAttr = 0;
						instanceAttr < NUM_INSTANCE_ATTRIBUTES;
						instanceAttr += 1){
					// NOTE(fusion): Same quirk as `ItemType::getAttributeOffset`.
					if(itemType->flags.check(g_instanceAttrFlags[instanceAttr])
					|| (instanceAttr == CONTENT && itemType->flags.check(CHEST))){
						itemType->attributeOffsets[instanceAttr] = instanceAttrCount;
						instanceAttrCount += 1;
					}
				}
			}else if(ident == "attributes"){
				script.readSymbol('{');
				while(!script.eof()){
					script.nextToken();
					if(script.token.kind == TOKEN_SPECIAL){
						char special = script.getSpecial();
						if(special == ',') continue;
						if(special == '}') break;
					}

					int typeAttr = GetTypeAttributeByName(script.getIdentifier());
					if(typeAttr == -1){
						script.error("unknown type attribute");
						break;
					}

					script.readSymbol('=');
					itemType->attributes[typeAttr] = script.readNumber();
				}
			}else{
				script.error("unknown object type attribute");
			}
		}else{
			script.error("invalid type id");
		}
	}

	if(const char *error = script.getError()){
		outError << error;
		return false;
	}

	return true;
}

void ClearItemTypes(void){
	g_itemTypes.clear();
}

