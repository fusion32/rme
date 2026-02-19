//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "dat_debug_view.h"

#include "graphics.h"
#include "editor.h"

// ============================================================================
//

class DatDebugViewListBox : public wxVListBox
{
public:
	DatDebugViewListBox(wxWindow* parent, wxWindowID id);

	void OnDrawItem(wxDC& dc, const wxRect& rect, size_t index) const override;
	wxCoord OnMeasureItem(size_t index) const override;

protected:
	std::vector<Sprite*> sprites;
};

DatDebugViewListBox::DatDebugViewListBox(wxWindow* parent, wxWindowID id) :
	wxVListBox(parent, id, wxDefaultPosition, wxDefaultSize, wxLB_SINGLE)
{
	sprites.clear();
	sprites.reserve(g_editor.gfx.getItemCount());
	for(int typeId = g_editor.gfx.getItemSpriteMinID();
			typeId < g_editor.gfx.getItemSpriteMaxID();
			typeId += 1){
		sprites.push_back(g_editor.gfx.getSprite(typeId));
	}
	SetItemCount(sprites.size());
}

void DatDebugViewListBox::OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const
{
	if(sprites[n] != NULL){
		sprites[n]->DrawTo(&dc, SPRITE_SIZE_32x32, rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight());
	}

	if(IsSelected(n)) {
		if(HasFocus())
			dc.SetTextForeground(wxColor(0xFF, 0xFF, 0xFF));
		else
			dc.SetTextForeground(wxColor(0x00, 0x00, 0xFF));
	} else {
		dc.SetTextForeground(wxColor(0x00, 0x00, 0x00));
	}

	int typeId = g_editor.gfx.getItemSpriteMinID() + (int)n;
	dc.DrawText(wxString() << typeId, rect.GetX() + 40, rect.GetY() + 6);
}

wxCoord DatDebugViewListBox::OnMeasureItem(size_t n) const
{
	return 32;
}

// ============================================================================
//

BEGIN_EVENT_TABLE(DatDebugView, wxPanel)
	EVT_TEXT(wxID_ANY, DatDebugView::OnTextChange)
	EVT_LISTBOX_DCLICK(wxID_ANY, DatDebugView::OnClickList)
END_EVENT_TABLE()

DatDebugView::DatDebugView(wxWindow* parent) : wxPanel(parent)
{
	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);

	search_field = newd wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
	search_field->SetFocus();
	sizer->Add(search_field, 0, wxEXPAND, 2);

	item_list = newd DatDebugViewListBox(this, wxID_ANY);
	item_list->SetMinSize(wxSize(470, 400));
	sizer->Add(item_list, 1, wxEXPAND | wxALL, 2);

	SetSizerAndFit(sizer);
	Centre(wxBOTH);
}

DatDebugView::~DatDebugView()
{
	////
}

void DatDebugView::OnTextChange(wxCommandEvent& evt)
{
	////
}

void DatDebugView::OnClickList(wxCommandEvent& evt)
{
	////
}

