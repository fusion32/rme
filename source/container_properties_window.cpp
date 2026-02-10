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

#include "container_properties_window.h"

#include "old_properties_window.h"
#include "properties_window.h"
#include "find_item_window.h"
#include "editor.h"
#include "map.h"

// ============================================================================
// Container Item Button
// Displayed in the container object properties menu, needs some
// custom event handling for the right-click menu etcetera so we
// need to define a custom class for it.

std::unique_ptr<ContainerItemPopupMenu> ContainerItemButton::popup_menu;

BEGIN_EVENT_TABLE(ContainerItemButton, ItemButton)
	EVT_LEFT_DOWN(ContainerItemButton::OnMouseDoubleLeftClick)
	EVT_RIGHT_UP(ContainerItemButton::OnMouseRightRelease)

	EVT_MENU(CONTAINER_POPUP_MENU_ADD, ContainerItemButton::OnAddItem)
	EVT_MENU(CONTAINER_POPUP_MENU_EDIT, ContainerItemButton::OnEditItem)
	EVT_MENU(CONTAINER_POPUP_MENU_REMOVE, ContainerItemButton::OnRemoveItem)
END_EVENT_TABLE()

ContainerItemButton::ContainerItemButton(wxWindow* parent, bool large, int _index, const Map* map, Item* item) :
	ItemButton(parent, (large? RENDER_SIZE_32x32 : RENDER_SIZE_16x16), (item ? item->getID() : 0)),
	edit_map(map),
	edit_item(item),
	index(_index)
{
	////
}

ContainerItemButton::~ContainerItemButton()
{
	////
}

void ContainerItemButton::OnMouseDoubleLeftClick(wxMouseEvent& WXUNUSED(event))
{
	wxCommandEvent dummy;

	if(edit_item) {
		OnEditItem(dummy);
		return;
	}

	Item *container = getParentContainer();
	if(container->countItems() < container->getAttribute(CAPACITY)){
		OnAddItem(dummy);
	}
}

void ContainerItemButton::OnMouseRightRelease(wxMouseEvent& WXUNUSED(event))
{
	if(!popup_menu) {
		popup_menu.reset(newd ContainerItemPopupMenu);
	}

	popup_menu->Update(this);
	PopupMenu(popup_menu.get());
}

void ContainerItemButton::OnAddItem(wxCommandEvent& WXUNUSED(event))
{
	FindItemDialog dialog(GetParent(), "Choose Item to add", true);

	if(dialog.ShowModal() == wxID_OK) {
		Item *container = getParentContainer();
		Item *item = Item::Create(dialog.getResultID());

		{ // TODO(fusion): This could probably be its own function.
			int insertIndex = (int)index;
			Item **it = &container->content;
			while(insertIndex > 0 && *it != NULL){
				it = &(*it)->next;
				insertIndex -= 1;
			}

			item->next = (*it);
			(*it)      = item;
		}

		ObjectPropertiesWindowBase* propertyWindow = getParentContainerWindow();
		if(propertyWindow)
			propertyWindow->Update();
	}
	dialog.Destroy();
}

void ContainerItemButton::OnEditItem(wxCommandEvent& WXUNUSED(event))
{
	ASSERT(edit_item);

	wxPoint newDialogAt;
	wxWindow *w = this;
	while((w = w->GetParent())) {
		if(ObjectPropertiesWindowBase* o = dynamic_cast<ObjectPropertiesWindowBase*>(w)) {
			newDialogAt = o->GetPosition();
			break;
		}
	}

	newDialogAt += wxPoint(20, 20);
	wxDialog *d = newd OldPropertiesWindow(this, edit_map, nullptr, edit_item, newDialogAt);
	d->ShowModal();
	d->Destroy();
}

void ContainerItemButton::OnRemoveItem(wxCommandEvent& WXUNUSED(event))
{
	ASSERT(edit_item);
	int32_t ret = g_editor.PopupDialog(GetParent(),
		"Remove Item",
		"Are you sure you want to remove this item from the container?",
		wxYES | wxNO
	);

	if(ret != wxID_YES) {
		return;
	}

	Item *container = getParentContainer();

	{ // TODO(fusion): This could also be its own function (?).
		Item **it = &container->content;
		while(*it != NULL && *it != edit_item){
			it = &(*it)->next;
		}

		ASSERT(*it == edit_item);
		(*it) = (*it)->next;
		edit_item->next = NULL;
		delete edit_item;
	}

	ObjectPropertiesWindowBase* propertyWindow = getParentContainerWindow();
	if(propertyWindow) {
		propertyWindow->Update();
	}
}

void ContainerItemButton::setItem(Item* item)
{
	edit_item = item;
	if(edit_item) {
		SetSprite(edit_item->getLookID());
	} else {
		SetSprite(0);
	}
}

ObjectPropertiesWindowBase* ContainerItemButton::getParentContainerWindow()
{
	for(wxWindow* window = GetParent(); window != nullptr; window = window->GetParent()) {
		ObjectPropertiesWindowBase* propertyWindow = dynamic_cast<ObjectPropertiesWindowBase*>(window);
		if(propertyWindow) {
			return propertyWindow;
		}
	}
	return nullptr;
}

Item *ContainerItemButton::getParentContainer()
{
	ObjectPropertiesWindowBase* propertyWindow = getParentContainerWindow();
	if(propertyWindow) {
		return propertyWindow->getItemBeingEdited();
	}
	return nullptr;
}

// ContainerItemPopupMenu
ContainerItemPopupMenu::ContainerItemPopupMenu() : wxMenu("")
{
	////
}

ContainerItemPopupMenu::~ContainerItemPopupMenu()
{
	////
}

void ContainerItemPopupMenu::Update(ContainerItemButton* btn)
{
	// Clear the menu of all items
	while(GetMenuItemCount() != 0) {
		wxMenuItem* m_item = FindItemByPosition(0);
		// If you add a submenu, this won't delete it.
		Delete(m_item);
	}

	wxMenuItem* addItem = nullptr;
	if(btn->edit_item) {
		Append(CONTAINER_POPUP_MENU_EDIT, "&Edit Item", "Open the properties menu for this item");
		addItem = Append(CONTAINER_POPUP_MENU_ADD, "&Add Item", "Add a new item to the container");
		Append(CONTAINER_POPUP_MENU_REMOVE, "&Remove Item", "Remove this item from the container");
	} else {
		addItem = Append(CONTAINER_POPUP_MENU_ADD, "&Add Item", "Add a new item to the container");
	}

	Item *container = btn->getParentContainer();
	if(container->countItems() >= container->getAttribute(CAPACITY))
		addItem->Enable(false);
}
