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

#include <wx/grid.h>

#include "tile.h"
#include "item.h"
#include "town.h"
#include "house.h"
#include "map.h"
#include "editor.h"
#include "creature.h"
#include "settings.h"

#include "application.h"
#include "old_properties_window.h"
#include "container_properties_window.h"

// ============================================================================
// Old Properties Window

BEGIN_EVENT_TABLE(OldPropertiesWindow, wxDialog)
	EVT_SET_FOCUS(OldPropertiesWindow::OnFocusChange)
	EVT_BUTTON(wxID_OK, OldPropertiesWindow::OnClickOK)
	EVT_BUTTON(wxID_CANCEL, OldPropertiesWindow::OnClickCancel)
END_EVENT_TABLE()

OldPropertiesWindow::OldPropertiesWindow(wxWindow* win_parent, const Map* map, const Tile* tile_parent, Item* item, wxPoint pos) :
	ObjectPropertiesWindowBase(win_parent, "Item Properties", map, tile_parent, item, pos),
	count_field(nullptr),
	destination_field(nullptr),
	splash_type_field(nullptr),
	text_field(nullptr),
	description_field(nullptr)
{
	ASSERT(edit_item);

	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);
	if(edit_item->getFlag(CONTAINER) || edit_item->getFlag(CHEST)) {
		// Container
		wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Container Properties");

		wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);
		subsizer->AddGrowableCol(1);

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "ID " + i2ws(edit_item->getID())));
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "\"" + wxstr(edit_item->getName()) + "\""));

		boxsizer->Add(subsizer, wxSizerFlags(0).Expand());

		// Now we add the subitems!
		wxSizer* contents_sizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Contents");

		bool use_large_sprites = g_settings.getBoolean(Config::USE_LARGE_CONTAINER_ICONS);
		wxSizer* horizontal_sizer = nullptr;
		const int additional_height_increment = (use_large_sprites? 40 : 24);
		int additional_height = 0;

		int32_t maxColumns;
		if(use_large_sprites) {
			maxColumns = 6;
		} else {
			maxColumns = 12;
		}

		int index = 0;
		int capacity = edit_item->getFlag(CONTAINER)
			? edit_item->getAttribute(CAPACITY) : 1;
		Item *item = edit_item->content;
		while(item != NULL || index < capacity){
			if(!horizontal_sizer) {
				horizontal_sizer = newd wxBoxSizer(wxHORIZONTAL);
			}

			ContainerItemButton* containerItemButton = newd ContainerItemButton(this, use_large_sprites, index, map, item);
			container_items.push_back(containerItemButton);
			horizontal_sizer->Add(containerItemButton);

			if(((index + 1) % maxColumns) == 0) {
				contents_sizer->Add(horizontal_sizer);
				horizontal_sizer = nullptr;
				additional_height += additional_height_increment;
			}

			index += 1;
			if(item != NULL){
				item = item->next;
			}
		}

		if(horizontal_sizer != nullptr) {
			contents_sizer->Add(horizontal_sizer);
			additional_height += additional_height_increment;
		}

		boxsizer->Add(contents_sizer, wxSizerFlags(2).Expand());

		topsizer->Add(boxsizer, wxSizerFlags(0).Expand().Border(wxALL, 20));

		//SetSize(260, 150 + additional_height);
	} else if(edit_item->getFlag(TEXT)) {
		// Book
		wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Writeable Properties");

		wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);
		subsizer->AddGrowableCol(1);

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "ID " + i2ws(edit_item->getID())));
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "\"" + wxstr(edit_item->getName()) + "\""));

		boxsizer->Add(subsizer, wxSizerFlags(1).Expand());

		wxSizer* textsizer = newd wxBoxSizer(wxVERTICAL);
		textsizer->Add(newd wxStaticText(this, wxID_ANY, "Text"), wxSizerFlags(1).Center());
		text_field = newd wxTextCtrl(this, wxID_ANY, item->getTextAttribute(TEXTSTRING), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
		textsizer->Add(text_field, wxSizerFlags(7).Expand());

		boxsizer->Add(textsizer, wxSizerFlags(2).Expand());

		topsizer->Add(boxsizer, wxSizerFlags(0).Expand().Border(wxALL, 20));

		//SetSize(220, 310);
	} else if(edit_item->getFlag(LIQUIDPOOL) || edit_item->getFlag(LIQUIDCONTAINER)) {
		// Splash
		wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Splash Properties");

		wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);
		subsizer->AddGrowableCol(1);

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "ID " + i2ws(edit_item->getID())));
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "\"" + wxstr(edit_item->getName()) + "\""));

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Type"));

		// Splash types
		splash_type_field = newd wxChoice(this, wxID_ANY);
		if(edit_item->getFlag(LIQUIDCONTAINER)) {
			splash_type_field->Append(GetLiquidName(LIQUID_NONE), newd int32_t(LIQUID_NONE));
		}

		for(int liquidType = LIQUID_FIRST; liquidType <= LIQUID_LAST; liquidType += 1) {
			splash_type_field->Append(GetLiquidName(liquidType), newd int32_t(liquidType));
		}

		if(edit_item->getFlag(LIQUIDPOOL) || edit_item->getFlag(LIQUIDCONTAINER)){
			int liquidType = edit_item->getFlag(LIQUIDPOOL)
					? edit_item->getAttribute(POOLLIQUIDTYPE)
					: edit_item->getAttribute(CONTAINERLIQUIDTYPE);
			splash_type_field->SetStringSelection(GetLiquidName(liquidType));
		} else {
			splash_type_field->SetSelection(0);
		}

		subsizer->Add(splash_type_field, wxSizerFlags(1).Expand());

		boxsizer->Add(subsizer, wxSizerFlags(1).Expand());

		topsizer->Add(boxsizer, wxSizerFlags(0).Expand().Border(wxALL, 20));

		//SetSize(220, 190);
	} else {
		// Normal item

		wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Item Properties");
		wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);
		subsizer->AddGrowableCol(1);

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "ID " + i2ws(edit_item->getID())));
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "\"" + wxstr(edit_item->getName()) + "\""));

		if(edit_item->getFlag(CUMULATIVE)){
			subsizer->Add(newd wxStaticText(this, wxID_ANY, "Count"));
			count_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getAttribute(AMOUNT)), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 100, edit_item->getAttribute(AMOUNT));
			subsizer->Add(count_field, wxSizerFlags(1).Expand());
		}else if(edit_item->getFlag(RUNE)){
			subsizer->Add(newd wxStaticText(this, wxID_ANY, "Charges"));
			count_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getAttribute(CHARGES)), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 100, edit_item->getAttribute(CHARGES));
			subsizer->Add(count_field, wxSizerFlags(1).Expand());
		}

		boxsizer->Add(subsizer, wxSizerFlags(1).Expand());
		topsizer->Add(boxsizer, wxSizerFlags(0).Expand().Border(wxLEFT | wxRIGHT, 20));

		if(edit_item->getFlag(TELEPORTABSOLUTE)) {
			Position dest = UnpackAbsoluteCoordinate(edit_item->getAttribute(ABSTELEPORTDESTINATION));
			destination_field = new PositionCtrl(this, "Destination", dest.x, dest.y, dest.z, map->getWidth(), map->getHeight());
			topsizer->Add(destination_field, wxSizerFlags(0).Expand().Border(wxLEFT | wxRIGHT, 20));
		}
	}

	// Others attributes
	const ItemType &type = GetItemType(edit_item->getID());
	wxStaticBoxSizer* others_sizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Others");
	wxFlexGridSizer* others_subsizer = newd wxFlexGridSizer(2, 5, 10);
	others_subsizer->AddGrowableCol(1);
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, "Stackable"));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, b2yn(type.getFlag(CUMULATIVE))));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, "Movable"));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, b2yn(!type.getFlag(UNMOVE))));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, "Pickupable"));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, b2yn(type.getFlag(TAKE))));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, "Hangable"));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, b2yn(type.getFlag(HANG))));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, "Block Missiles"));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, b2yn(type.getFlag(UNTHROW))));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, "Block Pathfinder"));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, b2yn(type.getFlag(AVOID))));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, "Has Elevation"));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, b2yn(type.getFlag(HEIGHT))));
	others_sizer->Add(others_subsizer, wxSizerFlags(1).Expand());
	topsizer->Add(others_sizer, wxSizerFlags(0).Expand().Border(wxLEFT | wxRIGHT | wxBOTTOM, 20));

	wxSizer* subsizer = newd wxBoxSizer(wxHORIZONTAL);
	subsizer->Add(newd wxButton(this, wxID_OK, "OK"), wxSizerFlags(1).Center());
	subsizer->Add(newd wxButton(this, wxID_CANCEL, "Cancel"), wxSizerFlags(1).Center());
	topsizer->Add(subsizer, wxSizerFlags(0).Center().Border(wxLEFT | wxRIGHT | wxBOTTOM, 20));

	SetSizerAndFit(topsizer);
	Centre(wxBOTH);
}

OldPropertiesWindow::OldPropertiesWindow(wxWindow* win_parent, const Map* map, const Tile* tile_parent, Creature* creature, wxPoint pos) :
	ObjectPropertiesWindowBase(win_parent, "Creature Properties", map, tile_parent, creature, pos),
	count_field(nullptr),
	destination_field(nullptr),
	splash_type_field(nullptr),
	text_field(nullptr),
	description_field(nullptr)
{
	ASSERT(edit_creature);

	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);
	wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Creature Properties");

	wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);
	subsizer->AddGrowableCol(1);

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Creature "));
	subsizer->Add(newd wxStaticText(this, wxID_ANY, "\"" + wxstr(edit_creature->getName()) + "\""), wxSizerFlags(1).Expand());

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Spawn interval"));
	count_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_creature->spawnInterval), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 10, 3600, edit_creature->spawnInterval);
	// count_field->SetSelection(-1, -1);
	subsizer->Add(count_field, wxSizerFlags(1).Expand());

	boxsizer->Add(subsizer, wxSizerFlags(1).Expand());

	topsizer->Add(boxsizer, wxSizerFlags(3).Expand().Border(wxALL, 20));
	//SetSize(220, 0);

	wxSizer* std_sizer = newd wxBoxSizer(wxHORIZONTAL);
	std_sizer->Add(newd wxButton(this, wxID_OK, "OK"), wxSizerFlags(1).Center());
	std_sizer->Add(newd wxButton(this, wxID_CANCEL, "Cancel"), wxSizerFlags(1).Center());
	topsizer->Add(std_sizer, wxSizerFlags(0).Center().Border(wxLEFT | wxRIGHT | wxBOTTOM, 20));

	SetSizerAndFit(topsizer);
	Centre(wxBOTH);
}

OldPropertiesWindow::~OldPropertiesWindow()
{
	// Warning: edit_item may no longer be valid, DONT USE IT!
	if(splash_type_field) {
		for(uint32_t i = 0; i < splash_type_field->GetCount(); ++i) {
			delete reinterpret_cast<int*>(splash_type_field->GetClientData(i));
		}
	}
}

void OldPropertiesWindow::OnFocusChange(wxFocusEvent& event)
{
	wxWindow* win = event.GetWindow();
	if(wxSpinCtrl* spin = dynamic_cast<wxSpinCtrl*>(win))
		spin->SetSelection(-1, -1);
	else if(wxTextCtrl* text = dynamic_cast<wxTextCtrl*>(win))
		text->SetSelection(-1, -1);
}

void OldPropertiesWindow::OnClickOK(wxCommandEvent& WXUNUSED(event))
{
	if(edit_item) {
		if(edit_item->getFlag(TEXT)){
			std::string text = nstr(text_field->GetValue());
			if(edit_item->getFlag(WRITE) || edit_item->getFlag(WRITEONCE)){
				int maxLength = edit_item->getFlag(WRITE)
						? edit_item->getAttribute(MAXLENGTH)
						: edit_item->getAttribute(MAXLENGTHONCE);
				if((int)text.length() >= maxLength){
					int ret = g_editor.PopupDialog(this, "Error", "Text is longer than the maximum supported length of this book type, do you still want to change it?", wxYES | wxNO);
					if(ret != wxID_YES) {
						return;
					}
				}
			}
			edit_item->setTextAttribute(TEXTSTRING, text.c_str());
		} else if(edit_item->getFlag(LIQUIDPOOL) || edit_item->getFlag(LIQUIDCONTAINER)){
			int *new_type = reinterpret_cast<int*>(splash_type_field->GetClientData(splash_type_field->GetSelection()));
			if(new_type) {
				if(edit_item->getFlag(LIQUIDPOOL)){
					edit_item->setAttribute(POOLLIQUIDTYPE, *new_type);
				}else{
					edit_item->setAttribute(CONTAINERLIQUIDTYPE, *new_type);
				}
			}
		} else {
			// Normal item
			if(edit_item->getFlag(TELEPORTABSOLUTE)) {
				Position dest = destination_field->GetPosition();
				const Tile *destTile = edit_map->getTile(dest);
				if(!destTile || destTile->getFlag(UNPASS) || destTile->getFlag(AVOID)){
					int ret = g_editor.PopupDialog(this, "Warning", "This teleport leads nowhere, or to an invalid location. Do you want to change the destination?", wxYES | wxNO);
					if(ret == wxID_YES) {
						return;
					}
				}
				edit_item->setAttribute(ABSTELEPORTDESTINATION, PackAbsoluteCoordinate(dest));
			}

			if(edit_item->getFlag(TEXT) && description_field) {
				edit_item->setTextAttribute(TEXTSTRING, description_field->GetValue());
			}

			int new_count = count_field ? count_field->GetValue() : 1;
			if(edit_item->getFlag(CUMULATIVE)){
				edit_item->setAttribute(AMOUNT, new_count);
			}else if(edit_item->getFlag(RUNE)){
				edit_item->setAttribute(CHARGES, new_count);
			}else if(edit_item->getFlag(WEAROUT)){
				edit_item->setAttribute(REMAININGUSES, new_count);
			}
		}
	} else if(edit_creature) {
		edit_creature->spawnInterval = count_field->GetValue();
	}
	EndModal(1);
}

void OldPropertiesWindow::OnClickCancel(wxCommandEvent& WXUNUSED(event))
{
	// Just close this window
	EndModal(0);
}

void OldPropertiesWindow::Update()
{
	if(edit_item->getFlag(CONTAINER) || edit_item->getFlag(CHEST)) {
		int index = 0;
		int capacity = edit_item->getFlag(CONTAINER)
			? edit_item->getAttribute(CAPACITY) : 1;
		Item *item = edit_item->content;
		while(item != NULL || index < capacity){
			container_items[index]->setItem(item);

			index += 1;
			if(item != NULL){
				item = item->next;
			}
		}
	}

	wxDialog::Update();
}
