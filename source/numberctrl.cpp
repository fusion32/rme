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
#include "numberctrl.h"

BEGIN_EVENT_TABLE(NumberCtrl, wxTextCtrl)
	EVT_KILL_FOCUS(NumberCtrl::OnKillFocus)
	EVT_TEXT_ENTER(wxID_ANY, NumberCtrl::OnTextEnter)
END_EVENT_TABLE()

NumberCtrl::NumberCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& sz,
		long style, int minValue, int maxValue, int value, const wxString& name) :
	wxTextCtrl(parent, id, wxEmptyString, pos, sz, style, wxTextValidator(wxFILTER_NUMERIC), name),
	minValue(minValue),
	maxValue(maxValue)
{
	SetIntValue(value);
}

NumberCtrl::~NumberCtrl(void)
{
	// no-op
}

void NumberCtrl::OnKillFocus(wxFocusEvent &event)
{
	CheckRange();
	event.Skip();
}

void NumberCtrl::OnTextEnter(wxCommandEvent &event)
{
	CheckRange();
}

void NumberCtrl::SetIntValue(int value)
{
	if(value < minValue){
		value = minValue;
	}else if(value > maxValue){
		value = maxValue;
	}

	// NOTE(fusion): ChangeValue doesn't generate events.
	ChangeValue(wxString() << value);
}

int NumberCtrl::GetIntValue()
{
	// NOTE(fusion): This shouldn't ever fail.
	int value;
	return GetValue().ToInt(&value) ? value : 0;
}

void NumberCtrl::SetRange(int minValue_, int maxValue_)
{
	if(minValue != minValue_ || maxValue != maxValue_){
		minValue = minValue_;
		maxValue = maxValue_;
		CheckRange();
	}
}

void NumberCtrl::CheckRange()
{
	int value = GetIntValue();
	if(value < minValue || value > maxValue){
		SetIntValue(value);
	}
}

