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

#ifndef RME_ACTION_H_
#define RME_ACTION_H_

#include "position.h"
#include "tile.h"

#include <variant>
#include <deque>

enum ActionType {
	ACTION_NONE,
	ACTION_MOVE,
	ACTION_SELECT,
	ACTION_UNSELECT,
	ACTION_DELETE_TILES,
	ACTION_CUT_TILES,
	ACTION_PASTE_TILES,
	ACTION_RANDOMIZE,
	ACTION_BORDERIZE,
	ACTION_DRAW,
	ACTION_ERASE,
	ACTION_SWITCHDOOR,
	ACTION_ROTATE_ITEM,
	ACTION_REPLACE_ITEMS,
	ACTION_CHANGE_PROPERTIES,
};

struct ChangeTile {
	Tile tile;
};

struct ChangeHouseExit {
	uint16_t houseId;
	Position pos;
};

struct ChangeWaypoint {
	std::string name;
	Position pos;
};

using Change = std::variant<
		ChangeTile,
		ChangeHouseExit,
		ChangeWaypoint>;

struct Action {
	ActionType          type = ACTION_NONE;
	bool                commited = false;
	std::vector<Change> changes = {};

	size_t memsize(void) const;
	size_t size(void) const { return changes.size(); }
	bool empty(void) const { return changes.empty(); }

	void changeTile(Tile tile);
	void changeHouseExit(uint16_t houseId, Position pos);
	void changeWaypoint(std::string name, Position pos);
	void commit(void);
	void undo(void);
};

struct ActionGroup {
	ActionType          type = ACTION_NONE;
	time_t              lastUsed = 0;
	std::vector<Action> actions;

	const char *getLabel(void) const;
	size_t size(void) const { return actions.size(); }
	bool empty(void) const { return actions.empty(); }

	// IMPORTANT(fusion): One action at a time...
	Action *createAction(void);

	void commit(void);
	void undo(void);
};

struct ActionQueue {
	size_t                  cursor = 0;
	std::deque<ActionGroup> groups;

	size_t size(void) const { return groups.size(); }
	bool empty(void) const { return groups.empty(); }
	bool canUndo(void) const { return cursor > 0; }
	bool canRedo(void) const { return cursor < groups.size(); }
	bool hasChanges(void) const;
	void resetTimer(void);

	const ActionGroup *getGroup(size_t index) const {
		return (index < groups.size() ? &groups[index] : NULL);
	}

	// IMPORTANT(fusion): One group or action at a time...
	ActionGroup *createGroup(ActionType type, int groupWindow = 0);
	Action *createAction(ActionType type, int groupWindow = 0);

	bool undo(void);
	bool redo(void);
	void clear(void);
};

#endif
