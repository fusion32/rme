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

#ifndef RME_PROPERTY_WINDOW_H_
#define RME_PROPERTY_WINDOW_H_

#include "main.h"
#include "common_windows.h"

class ContainerItemButton : public ItemButton {
public:
	ContainerItemButton(wxWindow *parent, bool large, int index, Item* item);
	~ContainerItemButton(void) override;
	void OnMouseDoubleLeftClick(wxMouseEvent &event);
	void OnMouseRightRelease(wxMouseEvent &event);
	void OnAddItem(wxCommandEvent &event);
	void OnInspectItem(wxCommandEvent &event);
	void OnRemoveItem(wxCommandEvent &event);
	void UpdateItem(Item *newItem);

private:
	int index = 0;
	Item *item = NULL;

	DECLARE_EVENT_TABLE()
};

class ItemPropertyWindow : public wxDialog {
public:
	ItemPropertyWindow(wxWindow *parent, Item *item, wxPoint pos = wxDefaultPosition);
	~ItemPropertyWindow(void) override;
	void Update() override;

	void OnFocusChange(wxFocusEvent &event);
	void OnClickOk(wxCommandEvent &event);
	void OnClickCancel(wxCommandEvent &event);

protected:
	Item *item = NULL;
	wxControl *attrCtrl[4] = {};
	std::vector<ContainerItemButton*> containerButtons;

	friend class ContainerItemButton;

	DECLARE_EVENT_TABLE()
};

class CreaturePropertyWindow: public wxDialog {
public:
	CreaturePropertyWindow(wxWindow *parent, Creature* creature, wxPoint pos = wxDefaultPosition);
	~CreaturePropertyWindow(void) override;
	void OnClickOk(wxCommandEvent &event);

protected:
	Creature *creature;
	wxSpinCtrl *spawn_radius;
	wxSpinCtrl *spawn_amount;
	wxSpinCtrl *spawn_interval;
	DECLARE_EVENT_TABLE()
};

#endif //RME_PROPERTY_WINDOW_H_
