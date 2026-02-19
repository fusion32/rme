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

#ifndef RME_COPYBUFFER_H_
#define RME_COPYBUFFER_H_

#include <wx/dataobj.h>

#include "position.h"
#include "map.h"

class CopyBuffer
{
public:
	CopyBuffer(void) = default;
	~CopyBuffer(void);

	void clear(void);
	int getTileCount(void) const;
	void copy(int floor);
	void cut(int floor);
	void paste(const Position& toPosition);

	bool canPaste(void) const { return getTileCount() > 0; }
	Position getPosition(void) const { return copyPos; }
	Map *getBufferMap(void) { return buffer; }

private:
	Position copyPos = {};
	Map      *buffer = NULL;
};

#endif
