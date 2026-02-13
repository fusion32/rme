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

#ifndef _RME_POSITION_CTRL_H_
#define _RME_POSITION_CTRL_H_

#include "numberctrl.h"
#include "position.h"

class PositionCtrl : public wxControl
{
public:
	PositionCtrl(wxWindow *parent, Position minPos, Position maxPos, Position pos);
	~PositionCtrl();

	long GetX() const { return x_field->GetIntValue(); }
	long GetY() const { return y_field->GetIntValue(); }
	long GetZ() const { return z_field->GetIntValue(); }
	Position GetPosition() const;

	void SetX(int value) { x_field->SetIntValue(value); }
	void SetY(int value) { y_field->SetIntValue(value); }
	void SetZ(int value) { z_field->SetIntValue(value); }
	void SetPosition(Position pos);

	bool Enable(bool enable = true);

	void OnClipboardText(wxClipboardTextEvent &event);

protected:
	NumberCtrl* x_field;
	NumberCtrl* y_field;
	NumberCtrl* z_field;
};

#endif
