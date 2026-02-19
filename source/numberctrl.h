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

#ifndef RME_NUMBERCTRL_H_
#define RME_NUMBERCTRL_H_

class NumberCtrl : public wxTextCtrl {
public:
	NumberCtrl(wxWindow *parent,
			wxWindowID id = wxID_ANY,
			const wxPoint &pos = wxDefaultPosition,
			const wxSize &sz = wxDefaultSize,
			long style = 0,
			int minValue = 0,
			int maxValue = 100,
			int value = 0,
			const wxString &name = wxTextCtrlNameStr);

	~NumberCtrl(void) override;

	void OnKillFocus(wxFocusEvent &event);
	void OnTextEnter(wxCommandEvent &event);

	int GetIntValue();
	void SetIntValue(int value);
	void SetRange(int minValue_, int maxValue_);

protected:
	void CheckRange();

	int minValue = 0;
	int maxValue = 0;

	DECLARE_EVENT_TABLE()
};

#endif //RME_NUMBERCTRL_H_
