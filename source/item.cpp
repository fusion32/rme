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

#include "brush.h"
#include "graphics.h"
#include "editor.h"
#include "item.h"

#include "ground_brush.h"
#include "carpet_brush.h"
#include "table_brush.h"
#include "wall_brush.h"

// Text Attributes
//==============================================================================
// NOTE(fusion): Definitely not the most optimized approach but string pointers
// should be stable as long as they're not deleted.

static std::vector<char*> g_textAttributes;

static int AddTextAttribute(const char *text){
	if(text == NULL || strlen(text) == 0){
		return 0;
	}

	int curCapacity = (int)g_textAttributes.size();
	int insertIndex = 0;
	while(insertIndex < curCapacity){
		if(g_textAttributes[insertIndex] == NULL){
			break;
		}
		insertIndex += 1;
	}

	if(insertIndex >= curCapacity){
		g_textAttributes.push_back(NULL);
	}

	g_textAttributes[insertIndex] = strdup(text);
	return 1 + insertIndex;
}

static const char *GetTextAttribute(int ref){
	if(ref == 0){
		return NULL;
	}

	int curCapacity = (int)g_textAttributes.size();
	int readIndex = ref - 1;
	if(readIndex < 0 || readIndex >= curCapacity){
		std::cout << "GetTextAttribute: Invalid text reference." << std::endl;
		return NULL;
	}

	return g_textAttributes[readIndex];
}

static int DupTextAttribute(int ref){
	return AddTextAttribute(GetTextAttribute(ref));
}

static void DeleteTextAttribute(int ref){
	if(ref == 0){
		return;
	}

	int curCapacity = (int)g_textAttributes.size();
	int deleteIndex = ref - 1;
	if(deleteIndex < 0 || deleteIndex >= curCapacity){
		std::cout << "DeleteTextAttribute: Invalid text reference." << std::endl;
		return;
	}

	if(g_textAttributes[deleteIndex]){
		free(g_textAttributes[deleteIndex]);
		g_textAttributes[deleteIndex] = NULL;
	}
}

#if 0
// TODO(fusion): Probably uneeded?
static void ClearTextAttributes(void){
	for(const char *text: g_textAttributes){
		free(text);
	}
	g_textAttributes.clear();
}
#endif

// Item
//==============================================================================
Item *Item::Create(int typeId_, int value /*= 0*/){
	if(ItemTypeExists(typeId_)){
		return newd Item(typeId_, value);
	}else{
		return NULL;
	}
}

Item::Item(int typeId_, int value /*= 0*/) : typeId(typeId_) {
	const ItemType &type = getItemType();
	if(type.getFlag(CUMULATIVE)){
		setAttribute(AMOUNT, value);
	}else if(type.getFlag(LIQUIDPOOL)){
		setAttribute(POOLLIQUIDTYPE, value);
	}else if(type.getFlag(LIQUIDCONTAINER)){
		setAttribute(CONTAINERLIQUIDTYPE, value);
	}else if(type.getFlag(KEY)){
		setAttribute(KEYNUMBER, value);
	}else if(type.getFlag(RUNE)){
		setAttribute(CHARGES, value);
	}
}

Item::~Item(void) {
	ASSERT(next == NULL);
	if(getFlag(CONTAINER) || getFlag(CHEST)){
		while(Item *it = content){
			content = it->next;
			it->next = NULL;
			delete it;
		}
	}else{
		ASSERT(content == NULL);
	}

	if(getFlag(TEXT)){
		DeleteTextAttribute(getAttribute(TEXTSTRING));
		DeleteTextAttribute(getAttribute(EDITOR));
	}
}

void Item::transform(int newTypeId, int value /*= 0*/){
	if(typeId == newTypeId){
		return;
	}

	const ItemType &oldType = GetItemType(typeId);
	const ItemType &newType = GetItemType(newTypeId);
	if(oldType.getStackPriority() != newType.getStackPriority()){
		// TODO(fusion): We might want to allow the stack priority to change but
		// it would also require us to update its order in the tile.
		return;
	}

	if(oldType.getFlag(CONTAINER)){
		int count = countItems();
		int capacity = (newType.getFlag(CONTAINER) ? newType.getAttribute(CAPACITY) : 0);
		while(count > capacity){
			Item *first = content;
			content = first->next;
			delete first;
			count -= 1;
		}
	}

	// TODO(fusion): I'm trying to mimic the game servers' ChangeObject function
	// here but it actually raised a concern. What happens to non-zero attributes
	// after the item is transformed? This is because different item types will
	// have different attribute offsets which can be a problem.

	int amount = 0;
	if(oldType.getFlag(CUMULATIVE)){
		amount = getAttribute(AMOUNT);
	}

	if(oldType.getFlag(TEXT) && !newType.getFlag(TEXT)){
		DeleteTextAttribute(getAttribute(TEXTSTRING));
		setAttribute(TEXTSTRING, 0);
		DeleteTextAttribute(getAttribute(EDITOR));
		setAttribute(EDITOR, 0);
	}

	typeId = newTypeId;

	if(newType.getFlag(CUMULATIVE)){
		if(amount <= 0){
			amount = 1;
		}
		setAttribute(AMOUNT, amount);
	}

	if(newType.getFlag(RUNE)){
		if(getAttribute(CHARGES) == 0){
			setAttribute(CHARGES, 1);
		}
	}

	if(newType.getFlag(LIQUIDPOOL)){
		if(getAttribute(POOLLIQUIDTYPE) == 0){
			setAttribute(POOLLIQUIDTYPE, LIQUID_WATER);
		}
	}

	if(newType.getFlag(WEAROUT)){
		if(getAttribute(REMAININGUSES) == 0){
			setAttribute(REMAININGUSES, newType.getAttribute(TOTALUSES));
		}
	}
}

Item *Item::deepCopy(void) const {
	Item *result = newd Item(typeId);
	result->selected = selected;
	for(int i = 0; i < NARRAY(attributes); i += 1){
		result->attributes[i] = attributes[i];
	}

	if(getFlag(CONTAINER) || getFlag(CHEST)){
		Item **tail = &result->content;
		for(Item *inner = content; inner != NULL; inner = inner->next){
			*tail = inner->deepCopy();
			tail = &(*tail)->next;
		}
	}

	if(getFlag(TEXT)){
		int textString = getAttribute(TEXTSTRING);
		if(textString != 0){
			textString = DupTextAttribute(textString);
			result->setAttribute(TEXTSTRING, textString);
		}

		int editor = getAttribute(EDITOR);
		if(editor != 0){
			editor = DupTextAttribute(editor);
			result->setAttribute(EDITOR, editor);
		}
	}

	return result;
}

bool Item::getFlag(ObjectFlag flag) const
{
	return getItemType().getFlag(flag);
}

int Item::getAttribute(ObjectTypeAttribute attr) const
{
	return getItemType().getAttribute(attr);
}

int Item::getAttributeOffset(ObjectInstanceAttribute attr) const
{
	return getItemType().getAttributeOffset(attr);
}

int Item::getAttribute(ObjectInstanceAttribute attr) const
{
	int attrOffset = getItemType().getAttributeOffset(attr);
	if(attrOffset == -1){
		std::cout << "Item::getAttribute: Object type " << typeId
				<< " is missing flag for instance attribute "
				<< attr << std::endl;
		return 0;
	}

	if(attrOffset < 0 || attrOffset >= NARRAY(attributes)){
		std::cout << "Item::getAttribute: Object type " << typeId
				<< " has invalid instance attribute offset "
				<< attrOffset << std::endl;
		return 0;
	}

	return attributes[attrOffset];
}

const char *Item::getTextAttribute(ObjectInstanceAttribute attr) const
{
	return GetTextAttribute(getAttribute(attr));
}

int Item::getStackPriority(void) const {
	return getItemType().getStackPriority();
}

int Item::getLookID(void) const {
	return getItemType().getLookId();
}

void Item::setAttribute(ObjectInstanceAttribute attr, int value){
	int attrOffset = getItemType().getAttributeOffset(attr);
	if(attrOffset == -1){
		std::cout << "Item::setAttribute: Object type " << typeId
				<< " is missing flag for instance attribute "
				<< attr << std::endl;
		return;
	}

	if(attrOffset < 0 || attrOffset >= NARRAY(attributes)){
		std::cout << "Item::setAttribute: Object type " << typeId
				<< " has invalid instance attribute offset "
				<< attrOffset << std::endl;
		return;
	}

	if(value == 0){
		if(attr == AMOUNT || attr == POOLLIQUIDTYPE || attr == CHARGES){
			value = 1;
		}else if(attr == REMAININGUSES){
			value = getItemType().getAttribute(TOTALUSES);
		}
	}

	attributes[attrOffset] = value;
}

void Item::setTextAttribute(ObjectInstanceAttribute attr, const char *text)
{
	DeleteTextAttribute(getAttribute(attr));
	setAttribute(attr, AddTextAttribute(text));
}

int Item::countItems(void) const {
	int result = 0;
	if(getFlag(CONTAINER)){
		for(Item *it = content; it != NULL; it = it->next){
			result += 1;
		}
	}
	return result;
}

int Item::getFrame(void) const {
	int frame = 0;
	if(GameSprite *sprite = getItemType().sprite){
		if(sprite->animator){
			frame = sprite->animator->getFrame();
		}
	}
	return frame;
}

uint8_t Item::getMiniMapColor() const
{
	if(GameSprite* sprite = getItemType().sprite) {
		return sprite->getMiniMapColor();
	}
	return 0;
}

wxPoint Item::getDrawOffset() const
{
	const ItemType &type = getItemType();
	if(type.sprite) {
		return type.sprite->getDrawOffset();
	}
	return wxPoint(0, 0);
}

SpriteLight Item::getLight() const
{
	return SpriteLight{
		(uint8_t)getAttribute(BRIGHTNESS),
		(uint8_t)getAttribute(LIGHTCOLOR),
	};
}

BorderType Item::getWallAlignment() const
{
	const ItemType &type = getItemType();
	if(!type.isWall) {
		return BORDER_NONE;
	}
	return type.border_alignment;
}

BorderType Item::getBorderAlignment() const
{
	return getItemType().border_alignment;
}

GroundBrush* Item::getGroundBrush() const
{
	const ItemType &type = getItemType();
	if(type.getFlag(BANK) && type.brush && type.brush->isGround()) {
		return type.brush->asGround();
	}
	return nullptr;
}

WallBrush* Item::getWallBrush() const
{
	const ItemType &type = getItemType();
	if(type.isWall && type.brush && type.brush->isWall())
		return type.brush->asWall();
	return nullptr;
}

DoorBrush* Item::getDoorBrush() const
{
	const ItemType &type = getItemType();
	if(!type.isWall || !type.isBrushDoor || !type.brush || !type.brush->isWall()) {
		return nullptr;
	}

	DoorType door_type = type.brush->asWall()->getDoorTypeFromID(typeId);
	DoorBrush* door_brush = nullptr;
	// Quite a horrible dependency on a global here, meh.
	switch(door_type) {
		case WALL_DOOR_NORMAL: {
			door_brush = g_editor.normal_door_brush;
			break;
		}
		case WALL_DOOR_LOCKED: {
			door_brush = g_editor.locked_door_brush;
			break;
		}
		case WALL_DOOR_QUEST: {
			door_brush = g_editor.quest_door_brush;
			break;
		}
		case WALL_DOOR_MAGIC: {
			door_brush = g_editor.magic_door_brush;
			break;
		}
		case WALL_WINDOW: {
			door_brush = g_editor.window_door_brush;
			break;
		}
		case WALL_HATCH_WINDOW: {
			door_brush = g_editor.hatch_door_brush;
			break;
		}
		default: {
			break;
		}
	}
	return door_brush;
}

TableBrush* Item::getTableBrush() const
{
	const ItemType &type = getItemType();
	if(type.isTable && type.brush && type.brush->isTable()) {
		return type.brush->asTable();
	}
	return nullptr;
}

CarpetBrush* Item::getCarpetBrush() const
{
	const ItemType &type = getItemType();
	if(type.isCarpet && type.brush && type.brush->isCarpet()) {
		return type.brush->asCarpet();
	}
	return nullptr;
}

