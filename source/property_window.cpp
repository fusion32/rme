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

#include "property_window.h"
#include "find_item_window.h"
#include "creature.h"
#include "editor.h"
#include "map.h"
#include "settings.h"

#include <wx/gdicmn.h>
#include <wx/grid.h>
#include <wx/sizer.h>
#include <wx/stringimpl.h>

// Container Item Button
//============================================================================
// Container Item Button
// Displayed in the container object properties menu, needs some
// custom event handling for the right-click menu etcetera so we
// need to define a custom class for it.

BEGIN_EVENT_TABLE(ContainerItemButton, ItemButton)
	EVT_LEFT_DOWN(ContainerItemButton::OnMouseDoubleLeftClick)
	EVT_RIGHT_UP(ContainerItemButton::OnMouseRightRelease)

	EVT_MENU(CONTAINER_POPUP_MENU_ADD, ContainerItemButton::OnAddItem)
	EVT_MENU(CONTAINER_POPUP_MENU_INSPECT, ContainerItemButton::OnInspectItem)
	EVT_MENU(CONTAINER_POPUP_MENU_REMOVE, ContainerItemButton::OnRemoveItem)
END_EVENT_TABLE()

ContainerItemButton::ContainerItemButton(wxWindow *parent, bool large, int index, Item *item) :
	ItemButton(parent, (large ? RENDER_SIZE_32x32 : RENDER_SIZE_16x16), (item ? item->getID() : 0)),
	index(index),
	item(item)
{
	// no-op
}

ContainerItemButton::~ContainerItemButton()
{
	// no-op
}

void ContainerItemButton::OnMouseDoubleLeftClick(wxMouseEvent &WXUNUSED(event))
{
	wxCommandEvent dummy;
	if(item){
		OnInspectItem(dummy);
	}else{
		OnAddItem(dummy);
	}
}

void ContainerItemButton::OnMouseRightRelease(wxMouseEvent &WXUNUSED(event))
{
	thread_local wxMenu *menu = NULL;
	thread_local wxMenuItem *add = NULL;
	thread_local wxMenuItem *inspect = NULL;
	thread_local wxMenuItem *remove = NULL;

	ItemPropertyWindow *parent = dynamic_cast<ItemPropertyWindow*>(GetParent());
	if(!parent){
		return;
	}

	if(menu == NULL){
		menu    = newd wxMenu();
		add     = menu->Append(CONTAINER_POPUP_MENU_ADD,     "&Add Item",     "Add a new item to the container");
		inspect = menu->Append(CONTAINER_POPUP_MENU_INSPECT, "&Inspect Item", "Open the properties menu for this item");
		remove  = menu->Append(CONTAINER_POPUP_MENU_REMOVE,  "&Remove Item",  "Remove this item from the container");
	}

	Item *container = parent->item;
	int capacity = container->getFlag(CONTAINER) ? container->getAttribute(CAPACITY) : 1;
	add->Enable(container->countItems() < capacity);
	inspect->Enable(item != NULL);
	remove->Enable(item != NULL);
	PopupMenu(menu);
}


void ContainerItemButton::OnAddItem(wxCommandEvent &WXUNUSED(event))
{
	ItemPropertyWindow *parent = dynamic_cast<ItemPropertyWindow*>(GetParent());
	if(!parent){
		return;
	}

	FindItemDialog dialog(parent, "Choose Item to add", true);
	if(dialog.ShowModal() == wxID_OK) {
		// NOTE(fusion): We're only appending an item to the parent container.
		// ItemPropertyWindow::Update will take care of updating each button item
		// and sprite.

		Item *container = parent->item;
		Item *newItem = Item::Create(dialog.getResultID());

		{
			int insertIndex = (int)index;
			Item **it = &container->content;
			while(insertIndex > 0 && *it != NULL){
				it = &(*it)->next;
				insertIndex -= 1;
			}

			newItem->next = (*it);
			(*it)         = newItem;
		}

		parent->Update();
	}
	dialog.Destroy();
}

void ContainerItemButton::OnInspectItem(wxCommandEvent &WXUNUSED(event))
{
	ASSERT(item != NULL);
	ItemPropertyWindow *parent = dynamic_cast<ItemPropertyWindow*>(GetParent());
	if(parent){
		wxDialog *dialog = newd ItemPropertyWindow(this, item,
					parent->GetPosition() + wxPoint(20, 20));
		dialog->ShowModal();
		dialog->Destroy();
	}
}

void ContainerItemButton::OnRemoveItem(wxCommandEvent &WXUNUSED(event))
{
	ASSERT(item != NULL);
	ItemPropertyWindow *parent = dynamic_cast<ItemPropertyWindow*>(GetParent());
	if(!parent){
		return;
	}

	int ret = g_editor.PopupDialog(parent, "Remove Item",
			"Are you sure you want to remove this item from the container?",
			wxYES | wxNO);
	if(ret == wxID_YES) {
		// NOTE(fusion): Similar to OnAddItem, except we're removing an item.
		Item *container = parent->item;

		{
			Item **it = &container->content;
			while(*it != NULL && *it != item){
				it = &(*it)->next;
			}

			ASSERT(*it == item);
			(*it) = (*it)->next;
			item->next = NULL;
			delete item;
		}

		parent->Update();
	}
}

void ContainerItemButton::UpdateItem(Item *newItem)
{
	item = newItem;
	SetSprite(item ? item->getLookID() : 0);
}

// Item Attribute Ctrl Helpers
//==============================================================================
static wxControl *IntAttrCtrl(wxWindow *parent, const Item *item, ObjectInstanceAttribute attr)
{
	return newd NumberCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
			0, 0, INT_MAX, item->getAttribute(attr));
}

static wxControl *IntAttrSpinCtrl(wxWindow *parent, const Item *item, ObjectInstanceAttribute attr, int min, int max)
{
	return newd wxSpinCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition,
			wxDefaultSize, wxSP_ARROW_KEYS, min, max, item->getAttribute(attr));
}

static wxControl *TextAttrCtrl(wxWindow *parent, const Item *item, ObjectInstanceAttribute attr, bool multiline)
{
	int height = (multiline ? 200 : -1);
	long style = (multiline ? wxTE_MULTILINE : 0);
	return newd wxTextCtrl(parent, wxID_ANY, item->getTextAttribute(attr),
			wxDefaultPosition, wxSize(300, height), style);
}

static wxControl *LiquidAttrCtrl(wxWindow *parent, const Item *item, ObjectInstanceAttribute attr)
{
	wxChoice *ctrl = newd wxChoice(parent, wxID_ANY);

	if(attr == CONTAINERLIQUIDTYPE){
		ctrl->Append(GetLiquidName(LIQUID_NONE), (void*)(uintptr_t)LIQUID_NONE);
	}

	for(int liquidType = LIQUID_FIRST; liquidType <= LIQUID_LAST; liquidType += 1){
		ctrl->Append(GetLiquidName(liquidType), (void*)(intptr_t)liquidType);
	}

	if(attr == CONTAINERLIQUIDTYPE || attr == POOLLIQUIDTYPE){
		ctrl->SetStringSelection(GetLiquidName(item->getAttribute(attr)));
	}else{
		ctrl->SetSelection(0);
	}

	return ctrl;
}

static wxControl *PositionAttrCtrl(wxWindow *parent, const Item *item, int attr)
{
	return newd PositionCtrl(parent,
			g_editor.map.getMinPosition(),
			g_editor.map.getMaxPosition(),
			UnpackAbsoluteCoordinate(item->getAttribute((ObjectInstanceAttribute)attr)));
}

static wxControl *AttrCtrl(wxWindow *parent, const Item *item, ObjectInstanceAttribute attr)
{
	wxControl *ctrl = NULL;
	switch(attr){
		case AMOUNT:
		case CHARGES:{
			ctrl = IntAttrSpinCtrl(parent, item, attr, 0, 100);
			break;
		}

		case CHESTQUESTNUMBER:
		case KEYNUMBER:
		case KEYHOLENUMBER:
		case DOORLEVEL:
		case DOORQUESTNUMBER:
		case DOORQUESTVALUE:
		case RESPONSIBLE:
		case REMAININGEXPIRETIME:
		case SAVEDEXPIRETIME:
		case REMAININGUSES:{
			ctrl = IntAttrCtrl(parent, item, attr);
			break;
		}

		case TEXTSTRING:{
			ctrl = TextAttrCtrl(parent, item, attr, true);
			break;
		}

		case EDITOR:{
			ctrl = TextAttrCtrl(parent, item, attr, false);
			break;
		}

		case CONTAINERLIQUIDTYPE:
		case POOLLIQUIDTYPE:{
			ctrl = LiquidAttrCtrl(parent, item, attr);
			break;
		}

		case ABSTELEPORTDESTINATION:{
			ctrl = PositionAttrCtrl(parent, item, attr);
			break;
		}

		default:{
			// no-op
			break;
		}
	}

	return ctrl;
}

static void AttrCtrlSet(wxControl *ctrl, Item *item, ObjectInstanceAttribute attr){
	switch(attr){
		case AMOUNT:
		case CHARGES:
		case CHESTQUESTNUMBER:
		case KEYNUMBER:
		case KEYHOLENUMBER:
		case DOORLEVEL:
		case DOORQUESTNUMBER:
		case DOORQUESTVALUE:
		case RESPONSIBLE:
		case REMAININGEXPIRETIME:
		case SAVEDEXPIRETIME:
		case REMAININGUSES:{
			if(wxSpinCtrl *v = dynamic_cast<wxSpinCtrl*>(ctrl)){
				item->setAttribute(attr, v->GetValue());
			}else if(NumberCtrl *v = dynamic_cast<NumberCtrl*>(ctrl)){
				item->setAttribute(attr, v->GetIntValue());
			}
			break;
		}

		case TEXTSTRING:
		case EDITOR:{
			if(wxTextCtrl *v = dynamic_cast<wxTextCtrl*>(ctrl)){
				item->setTextAttribute(attr, v->GetValue());
			}
			break;
		}

		case CONTAINERLIQUIDTYPE:
		case POOLLIQUIDTYPE:{
			if(wxChoice *v = dynamic_cast<wxChoice*>(ctrl)){
				item->setAttribute(attr, (int)(intptr_t)v->GetClientData(v->GetSelection()));
			}
			break;
		}

		case ABSTELEPORTDESTINATION:{
			if(PositionCtrl *v = dynamic_cast<PositionCtrl*>(ctrl)){
				item->setAttribute(attr, PackAbsoluteCoordinate(v->GetPosition()));
			}
			break;
		}

		default:{
			// no-op
			break;
		}
	}
}

// Item Property Window
//==============================================================================
BEGIN_EVENT_TABLE(ItemPropertyWindow, wxDialog)
	EVT_SET_FOCUS(ItemPropertyWindow::OnFocusChange)
	EVT_BUTTON(wxID_OK, ItemPropertyWindow::OnClickOk)
	EVT_BUTTON(wxID_CANCEL, ItemPropertyWindow::OnClickCancel)
END_EVENT_TABLE()

ItemPropertyWindow::ItemPropertyWindow(wxWindow *parent, Item *item, wxPoint pos) :
	wxDialog(parent, wxID_ANY, "Item Properties", pos, wxSize(600, 400), wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER),
	item(item)
{
	ASSERT(item != NULL);

	wxSizer *windowSizer = newd wxBoxSizer(wxVERTICAL);

	{
		wxSizer *panelSizer = newd wxBoxSizer(wxHORIZONTAL);

		{ // NOTE(fusion): Flags panel.
			wxStaticBoxSizer *flagSizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Flags");

			int numFlags = 0;
			for(int flag = 0; flag < NUM_FLAGS; flag += 1){
				if(item->getFlag((ObjectFlag)flag)){
					flagSizer->Add(newd wxStaticText(this, wxID_ANY, GetFlagName(flag)),
							wxSizerFlags(0).Border(wxLEFT | wxRIGHT, 5));
					numFlags += 1;
				}
			}

			if(numFlags <= 0){
				flagSizer->Add(newd wxStaticText(this, wxID_ANY, "No flags set."));
			}

			flagSizer->Add(0, 10, wxSizerFlags(1).Expand()); // padding at the bottom
			flagSizer->SetMinSize(100, 0);
			panelSizer->Add(flagSizer, wxSizerFlags(0).Expand().Border(wxALL, 5));
		}

		{ // NOTE(fusion): Attributes panel.
			wxStaticBoxSizer *attrSizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Attributes");

			{
				wxSizerFlags textFlags = wxSizerFlags(0).CenterVertical();

				wxFlexGridSizer *gridSizer = newd wxFlexGridSizer(2, 5, 20);
				gridSizer->AddGrowableCol(1);

				gridSizer->Add(newd wxStaticText(this, wxID_ANY, "TypeID"), textFlags);
				gridSizer->Add(newd wxStaticText(this, wxID_ANY, i2ws(item->getID())), textFlags);

				gridSizer->Add(newd wxStaticText(this, wxID_ANY, "Name"), textFlags);
				gridSizer->Add(newd wxStaticText(this, wxID_ANY, item->getName()), textFlags);

				if(!item->getDescription().empty()){
					gridSizer->Add(newd wxStaticText(this, wxID_ANY, "Description"), textFlags);
					gridSizer->Add(newd wxStaticText(this, wxID_ANY, item->getDescription()), textFlags);
				}

				for(int attr = 0; attr < NUM_INSTANCE_ATTRIBUTES; attr += 1){
					int attrOffset = item->getAttributeOffset((ObjectInstanceAttribute)attr);
					if(attrOffset < 0 || attrOffset >= NARRAY(attrCtrl) || attr == CONTENT){
						continue;
					}

					if(wxControl *ctrl = AttrCtrl(this, item, (ObjectInstanceAttribute)attr)){
						gridSizer->Add(newd wxStaticText(this, wxID_ANY, GetInstanceAttributeName(attr)), textFlags);
						gridSizer->Add(ctrl, wxSizerFlags(1).Expand());
						attrCtrl[attrOffset] = ctrl;
					}
				}

				attrSizer->Add(gridSizer, wxSizerFlags(1).Expand().Border(wxALL, 10));
			}

			if(item->getAttributeOffset(CONTENT) != -1){
				bool useLargeSprites = g_settings.getBoolean(Config::USE_LARGE_CONTAINER_ICONS);
				int maxColumns = useLargeSprites ? 6 : 12;
				wxStaticBoxSizer *contentSizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Content");
				wxGridSizer *gridSizer = newd wxGridSizer(maxColumns, 5, 5);

				int index = 0;
				int capacity = item->getFlag(CONTAINER) ? item->getAttribute(CAPACITY) : 1;
				Item *inner = item->content;
				while(inner != NULL || index < capacity){
					ContainerItemButton *containerButton = newd ContainerItemButton(this, useLargeSprites, index, inner);
					containerButtons.push_back(containerButton);
					gridSizer->Add(containerButton);

					index += 1;
					if(inner != NULL){
						inner = inner->next;
					}
				}

				contentSizer->Add(gridSizer, wxSizerFlags(0).Expand().Border(wxALL, 5));
				attrSizer->Add(contentSizer, wxSizerFlags(0).Expand().Border(wxALL, 10));
			}

			attrSizer->SetMinSize(200, 0);
			panelSizer->Add(attrSizer, wxSizerFlags(1).Expand().Border(wxALL, 5));
		}

		windowSizer->Add(panelSizer, wxSizerFlags(1).Expand());
	}

	{ // NOTE(fusion): Buttons.
		wxSizer *buttonSizer = newd wxBoxSizer(wxHORIZONTAL);

		buttonSizer->Add(newd wxButton(this, wxID_OK, "OK"), wxSizerFlags(1).Center());
		buttonSizer->Add(newd wxButton(this, wxID_CANCEL, "Cancel"), wxSizerFlags(1).Center());

		windowSizer->Add(buttonSizer, wxSizerFlags(0).Center().Border(wxALL, 10));
	}

	SetSizerAndFit(windowSizer);
}

ItemPropertyWindow::~ItemPropertyWindow()
{
	// no-op
}

void ItemPropertyWindow::OnFocusChange(wxFocusEvent &event)
{
	wxWindow *win = event.GetWindow();
	if(wxSpinCtrl *spin = dynamic_cast<wxSpinCtrl*>(win)){
		spin->SetSelection(-1, -1);
	}else if(wxTextCtrl *text = dynamic_cast<wxTextCtrl*>(win)){
		text->SetSelection(-1, -1);
	}
}

void ItemPropertyWindow::OnClickOk(wxCommandEvent &WXUNUSED(event))
{
	ASSERT(item != NULL);
	for(int attr = 0; attr < NUM_INSTANCE_ATTRIBUTES; attr += 1){
		int attrOffset = item->getAttributeOffset((ObjectInstanceAttribute)attr);
		if(attrOffset < 0 || attrOffset >= NARRAY(attrCtrl) || attr == CONTENT){
			continue;
		}

		if(attrCtrl[attrOffset] != NULL){
			AttrCtrlSet(attrCtrl[attrOffset], item, (ObjectInstanceAttribute)attr);
		}
	}

	// NOTE(fusion): The content is updated dynamically by the container buttons.
	//if(item->getAttributeOffset(CONTENT) != -1){
	//	// no-op
	//}

	EndModal(1);
}

void ItemPropertyWindow::OnClickCancel(wxCommandEvent &WXUNUSED(event))
{
	EndModal(0); // just close this window
}

void ItemPropertyWindow::Update()
{
	ASSERT(item != NULL);
	if(item->getFlag(CONTAINER) || item->getFlag(CHEST)) {
		int index = 0;
		int capacity = (int)containerButtons.size();
		Item *inner = item->content;
		while(index < capacity){
			containerButtons[index]->UpdateItem(inner);

			index += 1;
			if(inner != NULL){
				inner = inner->next;
			}
		}
	}

	wxDialog::Update();
}

// CreaturePropertyWindow
//==============================================================================
BEGIN_EVENT_TABLE(CreaturePropertyWindow, wxDialog)
	EVT_SET_FOCUS(ItemPropertyWindow::OnFocusChange)
	EVT_BUTTON(wxID_OK, CreaturePropertyWindow::OnClickOk)
	EVT_BUTTON(wxID_CANCEL, CreaturePropertyWindow::OnClickCancel)
END_EVENT_TABLE()

CreaturePropertyWindow::CreaturePropertyWindow(wxWindow *parent, Creature *creature, wxPoint pos) :
	wxDialog(parent, wxID_ANY, "Creature Properties", pos, wxSize(600, 400), wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER),
	creature(creature)
{
	ASSERT(creature != NULL);

	wxSizer *windowSizer = newd wxBoxSizer(wxVERTICAL);

	{
		wxSizerFlags textFlags = wxSizerFlags(0).CenterVertical();

		wxFlexGridSizer *gridSizer = newd wxFlexGridSizer(2, 10, 20);
		gridSizer->AddGrowableCol(1);

		gridSizer->Add(newd wxStaticText(this, wxID_ANY, "Name"), textFlags);
		gridSizer->Add(newd wxStaticText(this, wxID_ANY, creature->getName()), textFlags);

		gridSizer->Add(newd wxStaticText(this, wxID_ANY, "Spawn Radius"), textFlags);
		spawnRadiusCtrl = newd wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 50, creature->spawnRadius);
		gridSizer->Add(spawnRadiusCtrl, wxSizerFlags(1).Expand());

		gridSizer->Add(newd wxStaticText(this, wxID_ANY, "Spawn Amount"), textFlags);
		spawnAmountCtrl = newd wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 50, creature->spawnAmount);
		gridSizer->Add(spawnAmountCtrl, wxSizerFlags(1).Expand());

		gridSizer->Add(newd wxStaticText(this, wxID_ANY, "Spawn Interval"), textFlags);
		spawnIntervalCtrl = newd wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 3600, creature->spawnInterval);
		gridSizer->Add(spawnIntervalCtrl, wxSizerFlags(1).Expand());

		windowSizer->Add(gridSizer, wxSizerFlags(1).Expand().Border(wxALL, 10));
	}

	{
		wxSizer *buttonSizer = newd wxBoxSizer(wxHORIZONTAL);

		buttonSizer->Add(newd wxButton(this, wxID_OK, "OK"));
		buttonSizer->Add(newd wxButton(this, wxID_CANCEL, "Cancel"));

		windowSizer->Add(buttonSizer, wxSizerFlags(0).Center().Border(wxALL, 10));
	}

	SetSizerAndFit(windowSizer);
}

CreaturePropertyWindow::~CreaturePropertyWindow(void)
{
	// no-op
}

void CreaturePropertyWindow::OnFocusChange(wxFocusEvent &event)
{
	wxWindow *win = event.GetWindow();
	if(wxSpinCtrl *spin = dynamic_cast<wxSpinCtrl*>(win)){
		spin->SetSelection(-1, -1);
	}
}

void CreaturePropertyWindow::OnClickOk(wxCommandEvent &event)
{
	ASSERT(creature != NULL);
	creature->spawnRadius = spawnRadiusCtrl->GetValue();
	creature->spawnAmount = spawnAmountCtrl->GetValue();
	creature->spawnInterval = spawnIntervalCtrl->GetValue();
	EndModal(1);
}

void CreaturePropertyWindow::OnClickCancel(wxCommandEvent &event)
{
	EndModal(0); // just close this window
}

