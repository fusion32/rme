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

#include "main_menubar.h"
#include "application.h"
#include "preferences.h"
#include "about_window.h"
#include "minimap_window.h"
#include "dat_debug_view.h"
#include "result_window.h"
#include "find_item_window.h"
#include "duplicated_items_window.h"
#include "settings.h"
#include "items.h"
#include "editor.h"
#include "materials.h"

//#include <wx/chartype.h>

BEGIN_EVENT_TABLE(MainMenuBar, wxEvtHandler)
END_EVENT_TABLE()

MainMenuBar::MainMenuBar(MainFrame *frame) : frame(frame)
{
	using namespace MenuBar;
	checking_programmaticly = false;

#define MAKE_ACTION(id, kind, handler) actions[#id] = new MenuBar::Action(#id, id, kind, wxCommandEventFunction(&MainMenuBar::handler))
#define MAKE_SET_ACTION(id, kind, setting_, handler) actions[#id] = new MenuBar::Action(#id, id, kind, wxCommandEventFunction(&MainMenuBar::handler)); actions[#id].setting = setting_

	MAKE_ACTION(NEW, wxITEM_NORMAL, OnNew);
	MAKE_ACTION(OPEN, wxITEM_NORMAL, OnOpen);
	MAKE_ACTION(SAVE, wxITEM_NORMAL, OnSave);
	MAKE_ACTION(SAVE_AS, wxITEM_NORMAL, OnSaveAs);
	MAKE_ACTION(GENERATE_MAP, wxITEM_NORMAL, OnGenerateMap);
	MAKE_ACTION(CLOSE, wxITEM_NORMAL, OnClose);

	MAKE_ACTION(IMPORT_MAP, wxITEM_NORMAL, OnImportMap);
	MAKE_ACTION(IMPORT_MONSTERS, wxITEM_NORMAL, OnImportMonsterData);
	MAKE_ACTION(IMPORT_MINIMAP, wxITEM_NORMAL, OnImportMinimap);
	MAKE_ACTION(EXPORT_MINIMAP, wxITEM_NORMAL, OnExportMinimap);

	MAKE_ACTION(RELOAD_DATA, wxITEM_NORMAL, OnReloadDataFiles);
	//MAKE_ACTION(RECENT_FILES, wxITEM_NORMAL, OnRecent);
	MAKE_ACTION(PREFERENCES, wxITEM_NORMAL, OnPreferences);
	MAKE_ACTION(EXIT, wxITEM_NORMAL, OnQuit);

	MAKE_ACTION(UNDO, wxITEM_NORMAL, OnUndo);
	MAKE_ACTION(REDO, wxITEM_NORMAL, OnRedo);

	MAKE_ACTION(FIND_ITEM, wxITEM_NORMAL, OnSearchForItem);
	MAKE_ACTION(REPLACE_ITEMS, wxITEM_NORMAL, OnReplaceItems);
	MAKE_ACTION(SEARCH_ON_MAP_EVERYTHING, wxITEM_NORMAL, OnSearchForStuffOnMap);
	MAKE_ACTION(SEARCH_ON_MAP_CONTAINER, wxITEM_NORMAL, OnSearchForContainerOnMap);
	MAKE_ACTION(SEARCH_ON_MAP_WRITEABLE, wxITEM_NORMAL, OnSearchForWritableOnMap);
	MAKE_ACTION(SEARCH_ON_MAP_DUPLICATED_ITEMS, wxITEM_NORMAL, OnSearchForDuplicatedItemsOnMap);
	MAKE_ACTION(SEARCH_ON_SELECTION_EVERYTHING, wxITEM_NORMAL, OnSearchForStuffOnSelection);
	MAKE_ACTION(SEARCH_ON_SELECTION_CONTAINER, wxITEM_NORMAL, OnSearchForContainerOnSelection);
	MAKE_ACTION(SEARCH_ON_SELECTION_WRITEABLE, wxITEM_NORMAL, OnSearchForWritableOnSelection);
	MAKE_ACTION(SEARCH_ON_SELECTION_ITEM, wxITEM_NORMAL, OnSearchForItemOnSelection);
	MAKE_ACTION(SEARCH_ON_SELECTION_DUPLICATED_ITEMS, wxITEM_NORMAL, OnSearchForDuplicatedItemsOnSelection);
	MAKE_ACTION(REPLACE_ON_SELECTION_ITEMS, wxITEM_NORMAL, OnReplaceItemsOnSelection);
	MAKE_ACTION(REMOVE_ON_SELECTION_ITEM, wxITEM_NORMAL, OnRemoveItemOnSelection);
	MAKE_ACTION(SELECT_MODE_COMPENSATE, wxITEM_RADIO, OnSelectionTypeChange);
	MAKE_ACTION(SELECT_MODE_LOWER, wxITEM_RADIO, OnSelectionTypeChange);
	MAKE_ACTION(SELECT_MODE_CURRENT, wxITEM_RADIO, OnSelectionTypeChange);
	MAKE_ACTION(SELECT_MODE_VISIBLE, wxITEM_RADIO, OnSelectionTypeChange);

	MAKE_ACTION(AUTOMAGIC, wxITEM_CHECK, OnToggleAutomagic);
	MAKE_ACTION(BORDERIZE_SELECTION, wxITEM_NORMAL, OnBorderizeSelection);
	MAKE_ACTION(BORDERIZE_MAP, wxITEM_NORMAL, OnBorderizeMap);
	MAKE_ACTION(RANDOMIZE_SELECTION, wxITEM_NORMAL, OnRandomizeSelection);
	MAKE_ACTION(RANDOMIZE_MAP, wxITEM_NORMAL, OnRandomizeMap);
	MAKE_ACTION(GOTO_PREVIOUS_POSITION, wxITEM_NORMAL, OnGotoPreviousPosition);
	MAKE_ACTION(GOTO_POSITION, wxITEM_NORMAL, OnGotoPosition);
	MAKE_ACTION(JUMP_TO_BRUSH, wxITEM_NORMAL, OnJumpToBrush);
	MAKE_ACTION(JUMP_TO_ITEM_BRUSH, wxITEM_NORMAL, OnJumpToItemBrush);

	MAKE_ACTION(CUT, wxITEM_NORMAL, OnCut);
	MAKE_ACTION(COPY, wxITEM_NORMAL, OnCopy);
	MAKE_ACTION(PASTE, wxITEM_NORMAL, OnPaste);

	MAKE_ACTION(EDIT_TOWNS, wxITEM_NORMAL, OnMapEditTowns);
	MAKE_ACTION(EDIT_ITEMS, wxITEM_NORMAL, OnMapEditItems);
	MAKE_ACTION(EDIT_MONSTERS, wxITEM_NORMAL, OnMapEditMonsters);

	MAKE_ACTION(CLEAR_INVALID_HOUSES, wxITEM_NORMAL, OnClearHouseTiles);
	MAKE_ACTION(CLEAR_MODIFIED_STATE, wxITEM_NORMAL, OnClearModifiedState);
	MAKE_ACTION(MAP_REMOVE_ITEMS, wxITEM_NORMAL, OnMapRemoveItems);
	MAKE_ACTION(MAP_REMOVE_CORPSES, wxITEM_NORMAL, OnMapRemoveCorpses);
	MAKE_ACTION(MAP_REMOVE_UNREACHABLE_TILES, wxITEM_NORMAL, OnMapRemoveUnreachable);
	MAKE_ACTION(MAP_REMOVE_EMPTY_SPAWNS, wxITEM_NORMAL, OnMapRemoveEmptySpawns);
	MAKE_ACTION(MAP_CLEANUP, wxITEM_NORMAL, OnMapCleanup);
	MAKE_ACTION(MAP_CLEAN_HOUSE_ITEMS, wxITEM_NORMAL, OnMapCleanHouseItems);
	MAKE_ACTION(MAP_PROPERTIES, wxITEM_NORMAL, OnMapProperties);
	MAKE_ACTION(MAP_STATISTICS, wxITEM_NORMAL, OnMapStatistics);

	MAKE_ACTION(VIEW_TOOLBARS_BRUSHES, wxITEM_CHECK, OnToolbars);
	MAKE_ACTION(VIEW_TOOLBARS_POSITION, wxITEM_CHECK, OnToolbars);
	MAKE_ACTION(VIEW_TOOLBARS_SIZES, wxITEM_CHECK, OnToolbars);
	MAKE_ACTION(VIEW_TOOLBARS_INDICATORS, wxITEM_CHECK, OnToolbars);
	MAKE_ACTION(VIEW_TOOLBARS_STANDARD, wxITEM_CHECK, OnToolbars);
	MAKE_ACTION(NEW_VIEW, wxITEM_NORMAL, OnNewView);
	MAKE_ACTION(TOGGLE_FULLSCREEN, wxITEM_NORMAL, OnToggleFullscreen);

	MAKE_ACTION(ZOOM_IN, wxITEM_NORMAL, OnZoomIn);
	MAKE_ACTION(ZOOM_OUT, wxITEM_NORMAL, OnZoomOut);
	MAKE_ACTION(ZOOM_NORMAL, wxITEM_NORMAL, OnZoomNormal);

	MAKE_ACTION(SHOW_SHADE, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_ALL_FLOORS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(GHOST_ITEMS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(GHOST_HIGHER_FLOORS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(HIGHLIGHT_ITEMS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_EXTRA, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_INGAME_BOX, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_LIGHTS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_GRID, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_CREATURES, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_SPAWNS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_SPECIAL, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_AS_MINIMAP, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_ONLY_COLORS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_ONLY_MODIFIED, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_HOUSES, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_PATHING, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_TOOLTIPS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_PREVIEW, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_WALL_HOOKS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_PICKUPABLES, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_MOVEABLES, wxITEM_CHECK, OnChangeViewSettings);

	MAKE_ACTION(WIN_MINIMAP, wxITEM_NORMAL, OnMinimapWindow);
	MAKE_ACTION(WIN_ACTIONS_HISTORY, wxITEM_NORMAL, OnActionsHistoryWindow);
	MAKE_ACTION(NEW_PALETTE, wxITEM_NORMAL, OnNewPalette);
	MAKE_ACTION(TAKE_SCREENSHOT, wxITEM_NORMAL, OnTakeScreenshot);

	MAKE_ACTION(SELECT_TERRAIN, wxITEM_NORMAL, OnSelectTerrainPalette);
	MAKE_ACTION(SELECT_DOODAD, wxITEM_NORMAL, OnSelectDoodadPalette);
	MAKE_ACTION(SELECT_ITEM, wxITEM_NORMAL, OnSelectItemPalette);
	MAKE_ACTION(SELECT_CREATURE, wxITEM_NORMAL, OnSelectCreaturePalette);
	MAKE_ACTION(SELECT_HOUSE, wxITEM_NORMAL, OnSelectHousePalette);
	MAKE_ACTION(SELECT_WAYPOINT, wxITEM_NORMAL, OnSelectWaypointPalette);
	MAKE_ACTION(SELECT_RAW, wxITEM_NORMAL, OnSelectRawPalette);

	MAKE_ACTION(FLOOR_0, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_1, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_2, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_3, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_4, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_5, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_6, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_7, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_8, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_9, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_10, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_11, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_12, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_13, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_14, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_15, wxITEM_RADIO, OnChangeFloor);

	MAKE_ACTION(DEBUG_VIEW_DAT, wxITEM_NORMAL, OnDebugViewDat);
	MAKE_ACTION(GOTO_WEBSITE, wxITEM_NORMAL, OnGotoWebsite);
	MAKE_ACTION(ABOUT, wxITEM_NORMAL, OnAbout);

	// A deleter, this way the frame does not need
	// to bother deleting us.
	class CustomMenuBar : public wxMenuBar
	{
	public:
		CustomMenuBar(MainMenuBar* mb) : mb(mb) {}
		~CustomMenuBar()
		{
			delete mb;
		}
	private:
		MainMenuBar* mb;
	};

	menubar = newd CustomMenuBar(this);
	frame->SetMenuBar(menubar);

	// Tie all events to this handler!

	for(const auto &[_, action]: actions){
		frame->Connect(MAIN_FRAME_MENU + action->id, wxEVT_COMMAND_MENU_SELECTED,
			(wxObjectEventFunction)(wxEventFunction)(action->handler), nullptr, this);
	}

	for(size_t i = 0; i < 10; ++i) {
		frame->Connect(g_editor.recentFiles.GetBaseId() + i, wxEVT_COMMAND_MENU_SELECTED,
				wxCommandEventHandler(MainMenuBar::OnOpenRecent), nullptr, this);
	}
}

MainMenuBar::~MainMenuBar()
{
	// Don't need to delete menubar, it's owned by the frame

	for(std::map<std::string, MenuBar::Action*>::iterator ai = actions.begin(); ai != actions.end(); ++ai) {
		delete ai->second;
	}
}

namespace OnMapRemoveItems
{
	struct RemoveItemCondition
	{
		RemoveItemCondition(uint16_t itemId) :
			itemId(itemId) { }

		uint16_t itemId;

		bool operator()(Map& map, const Item* item, int64_t removed, int64_t done) {
			if(done % 0x8000 == 0)
				g_editor.SetLoadDone((uint32_t)(100 * done / map.getTileCount()));
			return item->getID() == itemId;
		}
	};
}

void MainMenuBar::EnableItem(MenuBar::ActionID id, bool enable)
{
	std::map<MenuBar::ActionID, std::list<wxMenuItem*> >::iterator fi = items.find(id);
	if(fi == items.end())
		return;

	std::list<wxMenuItem*>& li = fi->second;

	for(std::list<wxMenuItem*>::iterator i = li.begin(); i !=li.end(); ++i)
		(*i)->Enable(enable);
}

void MainMenuBar::CheckItem(MenuBar::ActionID id, bool enable)
{
	std::map<MenuBar::ActionID, std::list<wxMenuItem*> >::iterator fi = items.find(id);
	if(fi == items.end())
		return;

	std::list<wxMenuItem*>& li = fi->second;

	checking_programmaticly = true;
	for(std::list<wxMenuItem*>::iterator i = li.begin(); i !=li.end(); ++i)
		(*i)->Check(enable);
	checking_programmaticly = false;
}

bool MainMenuBar::IsItemChecked(MenuBar::ActionID id) const
{
	std::map<MenuBar::ActionID, std::list<wxMenuItem*> >::const_iterator fi = items.find(id);
	if(fi == items.end())
		return false;

	const std::list<wxMenuItem*>& li = fi->second;

	for(std::list<wxMenuItem*>::const_iterator i = li.begin(); i !=li.end(); ++i)
		if((*i)->IsChecked())
			return true;

	return false;
}

void MainMenuBar::Update()
{
	using namespace MenuBar;
	// This updates all buttons and sets them to proper enabled/disabled state

	bool enable = !g_editor.IsWelcomeDialogShown();
	menubar->Enable(enable);
    if(!enable) {
        return;
	}

	Editor* editor = g_editor.GetCurrentEditor();
	if(editor) {
		EnableItem(UNDO, g_editor.canUndo());
		EnableItem(REDO, editor->canRedo());
		EnableItem(PASTE, editor->copybuffer.canPaste());
	} else {
		EnableItem(UNDO, false);
		EnableItem(REDO, false);
		EnableItem(PASTE, false);
	}

	bool loaded = g_editor.IsProjectOpen();
	bool has_map = editor != nullptr;
	bool has_selection = editor && editor->hasSelection();
	bool is_host = has_map;
	bool is_local = has_map;

	EnableItem(CLOSE, is_local);
	EnableItem(SAVE, is_host);
	EnableItem(SAVE_AS, is_host);
	EnableItem(GENERATE_MAP, false);

	EnableItem(IMPORT_MAP, is_local);
	EnableItem(IMPORT_MONSTERS, is_local);
	EnableItem(IMPORT_MINIMAP, false);
	EnableItem(EXPORT_MINIMAP, is_local);

	EnableItem(FIND_ITEM, is_host);
	EnableItem(REPLACE_ITEMS, is_local);
	EnableItem(SEARCH_ON_MAP_EVERYTHING, is_host);
	EnableItem(SEARCH_ON_MAP_CONTAINER, is_host);
	EnableItem(SEARCH_ON_MAP_WRITEABLE, is_host);
	EnableItem(SEARCH_ON_MAP_DUPLICATED_ITEMS, is_host);
	EnableItem(SEARCH_ON_SELECTION_EVERYTHING, has_selection && is_host);
	EnableItem(SEARCH_ON_SELECTION_CONTAINER, has_selection && is_host);
	EnableItem(SEARCH_ON_SELECTION_WRITEABLE, has_selection && is_host);
	EnableItem(SEARCH_ON_SELECTION_DUPLICATED_ITEMS, has_selection && is_host);
	EnableItem(SEARCH_ON_SELECTION_ITEM, has_selection && is_host);
	EnableItem(REPLACE_ON_SELECTION_ITEMS, has_selection && is_host);
	EnableItem(REMOVE_ON_SELECTION_ITEM, has_selection && is_host);

	EnableItem(CUT, has_map);
	EnableItem(COPY, has_map);

	EnableItem(BORDERIZE_SELECTION, has_map && has_selection);
	EnableItem(BORDERIZE_MAP, is_local);
	EnableItem(RANDOMIZE_SELECTION, has_map && has_selection);
	EnableItem(RANDOMIZE_MAP, is_local);

	EnableItem(GOTO_PREVIOUS_POSITION, has_map);
	EnableItem(GOTO_POSITION, has_map);
	EnableItem(JUMP_TO_BRUSH, loaded);
	EnableItem(JUMP_TO_ITEM_BRUSH, loaded);

	EnableItem(MAP_REMOVE_ITEMS, is_host);
	EnableItem(MAP_REMOVE_CORPSES, is_local);
	EnableItem(MAP_REMOVE_UNREACHABLE_TILES, is_local);
	EnableItem(MAP_REMOVE_EMPTY_SPAWNS, is_local);
	EnableItem(CLEAR_INVALID_HOUSES, is_local);
	EnableItem(CLEAR_MODIFIED_STATE, is_local);

	EnableItem(EDIT_TOWNS, is_local);
	EnableItem(EDIT_ITEMS, false);
	EnableItem(EDIT_MONSTERS, false);

	EnableItem(MAP_CLEANUP, is_local);
	EnableItem(MAP_PROPERTIES, is_local);
	EnableItem(MAP_STATISTICS, is_local);

	EnableItem(NEW_VIEW, has_map);
	EnableItem(ZOOM_IN, has_map);
	EnableItem(ZOOM_OUT, has_map);
	EnableItem(ZOOM_NORMAL, has_map);

	if(has_map)
		CheckItem(SHOW_SPAWNS, g_settings.getBoolean(Config::SHOW_SPAWNS));

	EnableItem(WIN_MINIMAP, loaded);
	EnableItem(NEW_PALETTE, loaded);
	EnableItem(SELECT_TERRAIN, loaded);
	EnableItem(SELECT_DOODAD, loaded);
	EnableItem(SELECT_ITEM, loaded);
	EnableItem(SELECT_HOUSE, loaded);
	EnableItem(SELECT_CREATURE, loaded);
	EnableItem(SELECT_WAYPOINT, loaded);
	EnableItem(SELECT_RAW, loaded);

	EnableItem(DEBUG_VIEW_DAT, loaded);

	UpdateFloorMenu();
	UpdateIndicatorsMenu();
}

void MainMenuBar::LoadValues()
{
	using namespace MenuBar;

	CheckItem(VIEW_TOOLBARS_BRUSHES, g_settings.getBoolean(Config::SHOW_TOOLBAR_BRUSHES));
	CheckItem(VIEW_TOOLBARS_POSITION, g_settings.getBoolean(Config::SHOW_TOOLBAR_POSITION));
	CheckItem(VIEW_TOOLBARS_SIZES, g_settings.getBoolean(Config::SHOW_TOOLBAR_SIZES));
	CheckItem(VIEW_TOOLBARS_INDICATORS, g_settings.getBoolean(Config::SHOW_TOOLBAR_INDICATORS));
	CheckItem(VIEW_TOOLBARS_STANDARD, g_settings.getBoolean(Config::SHOW_TOOLBAR_STANDARD));

	CheckItem(SELECT_MODE_COMPENSATE, g_settings.getBoolean(Config::COMPENSATED_SELECT));

	if(IsItemChecked(MenuBar::SELECT_MODE_CURRENT))
		g_settings.setInteger(Config::SELECTION_TYPE, SELECT_CURRENT_FLOOR);
	else if(IsItemChecked(MenuBar::SELECT_MODE_LOWER))
		g_settings.setInteger(Config::SELECTION_TYPE, SELECT_ALL_FLOORS);
	else if(IsItemChecked(MenuBar::SELECT_MODE_VISIBLE))
		g_settings.setInteger(Config::SELECTION_TYPE, SELECT_VISIBLE_FLOORS);

	switch(g_settings.getInteger(Config::SELECTION_TYPE)) {
		case SELECT_CURRENT_FLOOR:
			CheckItem(SELECT_MODE_CURRENT, true);
			break;
		case SELECT_ALL_FLOORS:
			CheckItem(SELECT_MODE_LOWER, true);
			break;
		default:
		case SELECT_VISIBLE_FLOORS:
			CheckItem(SELECT_MODE_VISIBLE, true);
			break;
	}

	CheckItem(AUTOMAGIC, g_settings.getBoolean(Config::USE_AUTOMAGIC));

	CheckItem(SHOW_SHADE, g_settings.getBoolean(Config::SHOW_SHADE));
	CheckItem(SHOW_INGAME_BOX, g_settings.getBoolean(Config::SHOW_INGAME_BOX));
	CheckItem(SHOW_LIGHTS, g_settings.getBoolean(Config::SHOW_LIGHTS));
	CheckItem(SHOW_ALL_FLOORS, g_settings.getBoolean(Config::SHOW_ALL_FLOORS));
	CheckItem(GHOST_ITEMS, g_settings.getBoolean(Config::TRANSPARENT_ITEMS));
	CheckItem(GHOST_HIGHER_FLOORS, g_settings.getBoolean(Config::TRANSPARENT_FLOORS));
	CheckItem(SHOW_EXTRA, !g_settings.getBoolean(Config::SHOW_EXTRA));
	CheckItem(SHOW_GRID, g_settings.getBoolean(Config::SHOW_GRID));
	CheckItem(HIGHLIGHT_ITEMS, g_settings.getBoolean(Config::HIGHLIGHT_ITEMS));
	CheckItem(SHOW_CREATURES, g_settings.getBoolean(Config::SHOW_CREATURES));
	CheckItem(SHOW_SPAWNS, g_settings.getBoolean(Config::SHOW_SPAWNS));
	CheckItem(SHOW_SPECIAL, g_settings.getBoolean(Config::SHOW_SPECIAL_TILES));
	CheckItem(SHOW_AS_MINIMAP, g_settings.getBoolean(Config::SHOW_AS_MINIMAP));
	CheckItem(SHOW_ONLY_COLORS, g_settings.getBoolean(Config::SHOW_ONLY_TILEFLAGS));
	CheckItem(SHOW_ONLY_MODIFIED, g_settings.getBoolean(Config::SHOW_ONLY_MODIFIED_TILES));
	CheckItem(SHOW_HOUSES, g_settings.getBoolean(Config::SHOW_HOUSES));
	CheckItem(SHOW_PATHING, g_settings.getBoolean(Config::SHOW_BLOCKING));
	CheckItem(SHOW_TOOLTIPS, g_settings.getBoolean(Config::SHOW_TOOLTIPS));
	CheckItem(SHOW_PREVIEW, g_settings.getBoolean(Config::SHOW_PREVIEW));
	CheckItem(SHOW_WALL_HOOKS, g_settings.getBoolean(Config::SHOW_WALL_HOOKS));
	CheckItem(SHOW_PICKUPABLES, g_settings.getBoolean(Config::SHOW_PICKUPABLES));
	CheckItem(SHOW_MOVEABLES, g_settings.getBoolean(Config::SHOW_MOVEABLES));
}

void MainMenuBar::UpdateFloorMenu()
{
	using namespace MenuBar;

	if(!g_editor.IsProjectOpen()) {
		return;
	}

	for(int i = 0; i < rme::MapLayers; ++i)
		CheckItem(static_cast<ActionID>(MenuBar::FLOOR_0 + i), false);

	CheckItem(static_cast<ActionID>(MenuBar::FLOOR_0 + g_editor.GetCurrentFloor()), true);
}

void MainMenuBar::UpdateIndicatorsMenu()
{
	using namespace MenuBar;

	if(!g_editor.IsProjectOpen()) {
		return;
	}

	CheckItem(SHOW_WALL_HOOKS, g_settings.getBoolean(Config::SHOW_WALL_HOOKS));
	CheckItem(SHOW_PICKUPABLES, g_settings.getBoolean(Config::SHOW_PICKUPABLES));
	CheckItem(SHOW_MOVEABLES, g_settings.getBoolean(Config::SHOW_MOVEABLES));
}

bool MainMenuBar::Load(const FileName& path, wxArrayString& warnings, wxString& error)
{
	// Open the XML file
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(path.GetFullPath().mb_str());
	if(!result) {
		error = "Could not open " + path.GetFullName() + " (file not found or syntax error)";
		return false;
	}

	pugi::xml_node node = doc.child("menubar");
	if(!node) {
		error = path.GetFullName() + ": Invalid rootheader.";
		return false;
	}

	// Clear the menu
	while(menubar->GetMenuCount() > 0) {
		menubar->Remove(0);
	}

	// Load succeded
	for(pugi::xml_node menuNode: node.children()){
		// For each child node, load it
		wxObject *i = LoadItem(menuNode, nullptr, warnings, error);
		if (wxMenu *m = dynamic_cast<wxMenu*>(i)) {
			menubar->Append(m, m->GetTitle());
#ifdef __APPLE__
			m->SetTitle(m->GetTitle());
#else
			m->SetTitle("");
#endif
		} else if(i) {
			delete i;
			warnings.push_back(path.GetFullName() + ": Only menus can be subitems of main menu");
		}
	}

#ifdef __LINUX__
	const int count = 42;
	wxAcceleratorEntry entries[count];
	// Edit
	entries[0].Set(wxACCEL_CTRL, (int)'Z', MAIN_FRAME_MENU + MenuBar::UNDO);
	entries[1].Set(wxACCEL_CTRL | wxACCEL_SHIFT, (int)'Z', MAIN_FRAME_MENU + MenuBar::REDO);
	entries[2].Set(wxACCEL_CTRL, (int)'F', MAIN_FRAME_MENU + MenuBar::FIND_ITEM);
	entries[3].Set(wxACCEL_CTRL | wxACCEL_SHIFT, (int)'F', MAIN_FRAME_MENU + MenuBar::REPLACE_ITEMS);
	entries[4].Set(wxACCEL_NORMAL, (int)'A', MAIN_FRAME_MENU + MenuBar::AUTOMAGIC);
	entries[5].Set(wxACCEL_CTRL, (int)'B', MAIN_FRAME_MENU + MenuBar::BORDERIZE_SELECTION);
	entries[6].Set(wxACCEL_NORMAL, (int)'P', MAIN_FRAME_MENU + MenuBar::GOTO_PREVIOUS_POSITION);
	entries[7].Set(wxACCEL_CTRL, (int)'G', MAIN_FRAME_MENU + MenuBar::GOTO_POSITION);
	entries[8].Set(wxACCEL_NORMAL, (int)'J', MAIN_FRAME_MENU + MenuBar::JUMP_TO_BRUSH);
	entries[9].Set(wxACCEL_CTRL, (int)'X', MAIN_FRAME_MENU + MenuBar::CUT);
	entries[10].Set(wxACCEL_CTRL, (int)'C', MAIN_FRAME_MENU + MenuBar::COPY);
	entries[11].Set(wxACCEL_CTRL, (int)'V', MAIN_FRAME_MENU + MenuBar::PASTE);
	// Select
	entries[12].Set(wxACCEL_CTRL, (int)'R', MAIN_FRAME_MENU + MenuBar::SEARCH_ON_SELECTION_DUPLICATED_ITEMS);
	// View
	entries[13].Set(wxACCEL_CTRL, (int)'=', MAIN_FRAME_MENU + MenuBar::ZOOM_IN);
	entries[14].Set(wxACCEL_CTRL, (int)'-', MAIN_FRAME_MENU + MenuBar::ZOOM_OUT);
	entries[15].Set(wxACCEL_CTRL, (int)'0', MAIN_FRAME_MENU + MenuBar::ZOOM_NORMAL);
	entries[16].Set(wxACCEL_NORMAL, (int)'Q', MAIN_FRAME_MENU + MenuBar::SHOW_SHADE);
	entries[17].Set(wxACCEL_CTRL, (int)'W', MAIN_FRAME_MENU + MenuBar::SHOW_ALL_FLOORS);
	entries[18].Set(wxACCEL_NORMAL, (int)'Q', MAIN_FRAME_MENU + MenuBar::GHOST_ITEMS);
	entries[19].Set(wxACCEL_CTRL, (int)'L', MAIN_FRAME_MENU + MenuBar::GHOST_HIGHER_FLOORS);
	entries[20].Set(wxACCEL_SHIFT, (int)'I', MAIN_FRAME_MENU + MenuBar::SHOW_INGAME_BOX);
	entries[21].Set(wxACCEL_SHIFT, (int)'G', MAIN_FRAME_MENU + MenuBar::SHOW_GRID);
	entries[22].Set(wxACCEL_NORMAL, (int)'V', MAIN_FRAME_MENU + MenuBar::HIGHLIGHT_ITEMS);
	entries[23].Set(wxACCEL_NORMAL, (int)'F', MAIN_FRAME_MENU + MenuBar::SHOW_CREATURES);
	entries[24].Set(wxACCEL_NORMAL, (int)'S', MAIN_FRAME_MENU + MenuBar::SHOW_SPAWNS);
	entries[25].Set(wxACCEL_NORMAL, (int)'E', MAIN_FRAME_MENU + MenuBar::SHOW_SPECIAL);
	entries[26].Set(wxACCEL_SHIFT, (int)'E', MAIN_FRAME_MENU + MenuBar::SHOW_AS_MINIMAP);
	entries[27].Set(wxACCEL_CTRL, (int)'E', MAIN_FRAME_MENU + MenuBar::SHOW_ONLY_COLORS);
	entries[28].Set(wxACCEL_CTRL, (int)'M', MAIN_FRAME_MENU + MenuBar::SHOW_ONLY_MODIFIED);
	entries[29].Set(wxACCEL_CTRL, (int)'H', MAIN_FRAME_MENU + MenuBar::SHOW_HOUSES);
	entries[30].Set(wxACCEL_NORMAL, (int)'O', MAIN_FRAME_MENU + MenuBar::SHOW_PATHING);
	entries[31].Set(wxACCEL_NORMAL, (int)'Y', MAIN_FRAME_MENU + MenuBar::SHOW_TOOLTIPS);
	entries[32].Set(wxACCEL_NORMAL, (int)'L', MAIN_FRAME_MENU + MenuBar::SHOW_PREVIEW);
	entries[33].Set(wxACCEL_NORMAL, (int)'K', MAIN_FRAME_MENU + MenuBar::SHOW_WALL_HOOKS);
	// Window
	entries[34].Set(wxACCEL_NORMAL, (int)'M', MAIN_FRAME_MENU + MenuBar::WIN_MINIMAP);
	entries[35].Set(wxACCEL_NORMAL, (int)'T', MAIN_FRAME_MENU + MenuBar::SELECT_TERRAIN);
	entries[36].Set(wxACCEL_NORMAL, (int)'D', MAIN_FRAME_MENU + MenuBar::SELECT_DOODAD);
	entries[37].Set(wxACCEL_NORMAL, (int)'I', MAIN_FRAME_MENU + MenuBar::SELECT_ITEM);
	entries[38].Set(wxACCEL_NORMAL, (int)'H', MAIN_FRAME_MENU + MenuBar::SELECT_HOUSE);
	entries[39].Set(wxACCEL_NORMAL, (int)'C', MAIN_FRAME_MENU + MenuBar::SELECT_CREATURE);
	entries[40].Set(wxACCEL_NORMAL, (int)'W', MAIN_FRAME_MENU + MenuBar::SELECT_WAYPOINT);
	entries[41].Set(wxACCEL_NORMAL, (int)'R', MAIN_FRAME_MENU + MenuBar::SELECT_RAW);

	wxAcceleratorTable accelerator(count, entries);
	frame->SetAcceleratorTable(accelerator);
#endif

	recentFiles.AddFilesToMenu();
	Update();
	LoadValues();
	return true;
}

wxObject* MainMenuBar::LoadItem(pugi::xml_node node, wxMenu* parent, wxArrayString& warnings, wxString& error)
{
	pugi::xml_attribute attribute;

	const std::string& nodeName = as_lower_str(node.name());
	if(nodeName == "menu") {
		if(!(attribute = node.attribute("name"))) {
			return nullptr;
		}

		std::string name = attribute.as_string();
		std::replace(name.begin(), name.end(), '$', '&');

		wxMenu* menu = newd wxMenu;
		if((attribute = node.attribute("special")) && std::string(attribute.as_string()) == "RECENT_FILES") {
			recentFiles.UseMenu(menu);
		} else {
			for(pugi::xml_node menuNode = node.first_child(); menuNode; menuNode = menuNode.next_sibling()) {
				// Load an add each item in order
				LoadItem(menuNode, menu, warnings, error);
			}
		}

		// If we have a parent, add ourselves.
		// If not, we just return the item and the parent function
		// is responsible for adding us to wherever
		if(parent) {
			parent->AppendSubMenu(menu, wxstr(name));
		} else {
			menu->SetTitle((name));
		}
		return menu;
	} else if(nodeName == "item") {
		// We must have a parent when loading items
		if(!parent) {
			return nullptr;
		} else if(!(attribute = node.attribute("name"))) {
			return nullptr;
		}

		std::string name = attribute.as_string();
		std::replace(name.begin(), name.end(), '$', '&');
		if(!(attribute = node.attribute("action"))) {
			return nullptr;
		}

		const std::string& action = attribute.as_string();
		std::string hotkey = node.attribute("hotkey").as_string();
		if(!hotkey.empty()) {
			hotkey = '\t' + hotkey;
		}

		const std::string& help = node.attribute("help").as_string();
		name += hotkey;

		auto it = actions.find(action);
		if(it == actions.end()) {
			warnings.push_back("Invalid action type '" + wxstr(action) + "'.");
			return nullptr;
		}

		const MenuBar::Action& act = *it->second;
		wxAcceleratorEntry* entry = wxAcceleratorEntry::Create(wxstr(hotkey));
		if(entry) {
			delete entry; // accelerators.push_back(entry);
		} else {
			warnings.push_back("Invalid hotkey.");
		}

		wxMenuItem* tmp = parent->Append(
			MAIN_FRAME_MENU + act.id, // ID
			wxstr(name), // Title of button
			wxstr(help), // Help text
			act.kind // Kind of item
		);
		items[MenuBar::ActionID(act.id)].push_back(tmp);
		return tmp;
	} else if(nodeName == "separator") {
		// We must have a parent when loading items
		if(!parent) {
			return nullptr;
		}
		return parent->AppendSeparator();
	}
	return nullptr;
}

void MainMenuBar::OnNew(wxCommandEvent& WXUNUSED(event))
{
	g_editor.NewMap();
}

void MainMenuBar::OnGenerateMap(wxCommandEvent& WXUNUSED(event))
{
	/*
	if(!DoQuerySave()) return;

	std::ostringstream os;
	os << "Untitled-" << untitled_counter << ".otbm";
	++untitled_counter;

	editor.generateMap(wxstr(os.str()));

	g_editor.SetStatusText("Generated newd map");

	g_editor.UpdateTitle();
	g_editor.RefreshPalettes();
	g_editor.UpdateMinimap();
	g_editor.FitViewToMap();
	g_editor.UpdateMenubar();
	Refresh();
	*/
}

void MainMenuBar::OnOpenRecent(wxCommandEvent& event)
{
	FileName fn(recentFiles.GetHistoryFile(event.GetId() - recentFiles.GetBaseId()));
	frame->LoadMap(fn);
}

void MainMenuBar::OnOpen(wxCommandEvent& WXUNUSED(event))
{
	g_editor.OpenMap();
}

void MainMenuBar::OnClose(wxCommandEvent& WXUNUSED(event))
{
	frame->DoQuerySave(true); // It closes the editor too
}

void MainMenuBar::OnSave(wxCommandEvent& WXUNUSED(event))
{
	g_editor.SaveMap();
}

void MainMenuBar::OnSaveAs(wxCommandEvent& WXUNUSED(event))
{
	g_editor.SaveMapAs();
}

void MainMenuBar::OnPreferences(wxCommandEvent& WXUNUSED(event))
{
	PreferencesWindow dialog(frame);
	dialog.ShowModal();
	dialog.Destroy();
}

void MainMenuBar::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	/*
	while(g_editor.IsProjectOpen())
		if(!frame->DoQuerySave(true))
			return;
			*/
	//((Application*)wxTheApp)->Unload();
	g_editor.root->Close();
}

void MainMenuBar::OnImportMap(wxCommandEvent& WXUNUSED(event))
{
	ASSERT(g_editor.GetCurrentEditor());
	wxDialog* importmap = newd ImportMapWindow(frame, *g_editor.GetCurrentEditor());
	importmap->ShowModal();
}

void MainMenuBar::OnImportMonsterData(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog dlg(g_editor.root, "Import monster/npc file", "","","*.xml", wxFD_OPEN | wxFD_MULTIPLE | wxFD_FILE_MUST_EXIST);
	if(dlg.ShowModal() == wxID_OK) {
		wxArrayString paths;
		dlg.GetPaths(paths);
		for(uint32_t i = 0; i < paths.GetCount(); ++i) {
			wxString error;
			wxArrayString warnings;
			bool ok = g_creatures.importXMLFromOT(FileName(paths[i]), error, warnings);
			if(ok)
				g_editor.ListDialog("Monster loader errors", warnings);
			else
				wxMessageBox("Error OT data file \"" + paths[i] + "\".\n" + error, "Error", wxOK | wxICON_INFORMATION, g_editor.root);
		}
	}
}

void MainMenuBar::OnImportMinimap(wxCommandEvent& WXUNUSED(event))
{
	ASSERT(g_editor.IsProjectOpen());
	//wxDialog* importmap = newd ImportMapWindow();
	//importmap->ShowModal();
}

void MainMenuBar::OnExportMinimap(wxCommandEvent& WXUNUSED(event))
{
	if(!g_editor.IsProjectOpen()) {
		return;
	}

	ExportMiniMapWindow dialog(frame, *g_editor.GetCurrentEditor());
	dialog.ShowModal();
}

void MainMenuBar::OnDebugViewDat(wxCommandEvent& WXUNUSED(event))
{
	wxDialog dlg(frame, wxID_ANY, "Debug .dat file", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	new DatDebugView(&dlg);
	dlg.ShowModal();
}

void MainMenuBar::OnReloadDataFiles(wxCommandEvent& WXUNUSED(event))
{
	wxString error;
	wxArrayString warnings;
	g_editor.LoadVersion(error, warnings, true);
	g_editor.PopupDialog("Error", error, wxOK);
	g_editor.ListDialog("Warnings", warnings);
}

void MainMenuBar::OnGotoWebsite(wxCommandEvent& WXUNUSED(event))
{
	::wxLaunchDefaultBrowser("http://www.remeresmapeditor.com/",  wxBROWSER_NEW_WINDOW);
}

void MainMenuBar::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	AboutWindow about(frame);
	about.ShowModal();
}

void MainMenuBar::OnUndo(wxCommandEvent& WXUNUSED(event))
{
	g_editor.DoUndo();
}

void MainMenuBar::OnRedo(wxCommandEvent& WXUNUSED(event))
{
	g_editor.DoRedo();
}

namespace OnSearchForItem
{
	struct Finder
	{
		Finder(uint16_t itemId, uint32_t maxCount) :
			itemId(itemId), maxCount(maxCount) {}

		uint16_t itemId;
		uint32_t maxCount;
		std::vector< std::pair<Tile*, Item*> > result;

		bool limitReached() const { return result.size() >= (size_t)maxCount; }

		void operator()(Map& map, Tile* tile, Item* item, long long done)
		{
			if(result.size() >= (size_t)maxCount)
				return;

			if(done % 0x8000 == 0)
				g_editor.SetLoadDone((unsigned int)(100 * done / map.getTileCount()));

			if(item->getID() == itemId)
				result.push_back(std::make_pair(tile, item));
		}
	};
}

void MainMenuBar::OnSearchForItem(wxCommandEvent& WXUNUSED(event))
{
	if(!g_editor.IsProjectOpen())
		return;

	FindItemDialog dialog(frame, "Search for Item");
	dialog.setSearchMode((FindItemDialog::SearchMode)g_settings.getInteger(Config::FIND_ITEM_MODE));
	if(dialog.ShowModal() == wxID_OK) {
		OnSearchForItem::Finder finder(dialog.getResultID(), (uint32_t)g_settings.getInteger(Config::REPLACE_SIZE));
		g_editor.CreateLoadBar("Searching map...");

		foreach_ItemOnMap(g_editor.GetCurrentMap(), finder, false);
		std::vector< std::pair<Tile*, Item*> >& result = finder.result;

		g_editor.DestroyLoadBar();

		if(finder.limitReached()) {
			wxString msg;
			msg << "The configured limit has been reached. Only " << finder.maxCount << " results will be displayed.";
			g_editor.PopupDialog("Notice", msg, wxOK);
		}

		SearchResultWindow* window = g_editor.ShowSearchWindow();
		window->Clear();
		for(std::vector<std::pair<Tile*, Item*> >::const_iterator iter = result.begin(); iter != result.end(); ++iter) {
			Tile* tile = iter->first;
			Item* item = iter->second;
			window->AddPosition(wxstr(item->getName()), tile->getPosition());
		}

		g_settings.setInteger(Config::FIND_ITEM_MODE, (int)dialog.getSearchMode());
	}
	dialog.Destroy();
}

void MainMenuBar::OnReplaceItems(wxCommandEvent& WXUNUSED(event))
{
	if(!g_editor.IsProjectOpen())
		return;

	if(MapTab* tab = g_editor.GetCurrentMapTab()) {
		if(MapWindow* window = tab->GetView()) {
			window->ShowReplaceItemsDialog(false);
		}
	}
}

namespace OnSearchForStuff
{
	struct Searcher
	{
		// TODO(fusion): Relevant srv flags/attributes instead?
		bool search_container;
		bool search_writable;
		std::vector<std::pair<Tile*, Item*>> found;

		void operator()(Map& map, Tile* tile, Item* item, long long done)
		{
			if(done % 0x8000 == 0) {
				g_editor.SetLoadDone((unsigned int)(100 * done / map.getTileCount()));
			}

			bool add = false;
			if(!add && search_container){
				if((item->getFlag(CONTAINER) || item->getFlag(CHEST)) && item->content != NULL){
					add = true;
				}
			}

			if(!add && search_writable){
				if(item->getFlag(TEXT)){
					const char *text = item->getTextAttribute(TEXTSTRING);
					if(text && strlen(text) > 0){
						add = true;
					}
				}
			}

			if(add){
				found.push_back(std::make_pair(tile, item));
			}
		}

		wxString desc(Item* item)
		{
			wxString label = wxstr(item->getName());

			if(item->getFlag(CONTAINER) || item->getFlag(CHEST)){
				label << " (Container) ";
			}

			if(item->getFlag(TEXT)){
				const char *text = item->getTextAttribute(TEXTSTRING);
				if(text && strlen(text) > 0){
					label << " (Text: " << text << ") ";
				}
			}

			return label;
		}

		void sort()
		{
			// TODO ?
		}
	};
}

void MainMenuBar::OnSearchForStuffOnMap(wxCommandEvent& WXUNUSED(event))
{
	SearchItems(true, true);
}

void MainMenuBar::OnSearchForContainerOnMap(wxCommandEvent& WXUNUSED(event))
{
	SearchItems(true, false);
}

void MainMenuBar::OnSearchForWritableOnMap(wxCommandEvent& WXUNUSED(event))
{
	SearchItems(false, true);
}

void MainMenuBar::OnSearchForDuplicatedItemsOnMap(wxCommandEvent& WXUNUSED(event))
{
	SearchDuplicatedItems(false);
}

void MainMenuBar::OnSearchForStuffOnSelection(wxCommandEvent& WXUNUSED(event))
{
	SearchItems(true, true, true);
}

void MainMenuBar::OnSearchForContainerOnSelection(wxCommandEvent& WXUNUSED(event))
{
	SearchItems(true, false, true);
}

void MainMenuBar::OnSearchForWritableOnSelection(wxCommandEvent& WXUNUSED(event))
{
	SearchItems(false, true, true);
}

void MainMenuBar::OnSearchForItemOnSelection(wxCommandEvent& WXUNUSED(event))
{
	if(!g_editor.IsProjectOpen())
		return;

	FindItemDialog dialog(frame, "Search on Selection");
	dialog.setSearchMode((FindItemDialog::SearchMode)g_settings.getInteger(Config::FIND_ITEM_MODE));
	if(dialog.ShowModal() == wxID_OK) {
		OnSearchForItem::Finder finder(dialog.getResultID(), (uint32_t)g_settings.getInteger(Config::REPLACE_SIZE));
		g_editor.CreateLoadBar("Searching on selected area...");

		foreach_ItemOnMap(g_editor.GetCurrentMap(), finder, true);
		std::vector<std::pair<Tile*, Item*> >& result = finder.result;

		g_editor.DestroyLoadBar();

		if(finder.limitReached()) {
			wxString msg;
			msg << "The configured limit has been reached. Only " << finder.maxCount << " results will be displayed.";
			g_editor.PopupDialog("Notice", msg, wxOK);
		}

		SearchResultWindow* window = g_editor.ShowSearchWindow();
		window->Clear();
		for(std::vector<std::pair<Tile*, Item*> >::const_iterator iter = result.begin(); iter != result.end(); ++iter) {
			Tile* tile = iter->first;
			Item* item = iter->second;
			window->AddPosition(wxstr(item->getName()), tile->getPosition());
		}

		g_settings.setInteger(Config::FIND_ITEM_MODE, (int)dialog.getSearchMode());
	}

	dialog.Destroy();
}

void MainMenuBar::OnSearchForDuplicatedItemsOnSelection(wxCommandEvent& WXUNUSED(event))
{
	SearchDuplicatedItems(true);
}

void MainMenuBar::OnReplaceItemsOnSelection(wxCommandEvent& WXUNUSED(event))
{
	if(!g_editor.IsProjectOpen())
		return;

	if(MapTab* tab = g_editor.GetCurrentMapTab()) {
		if(MapWindow* window = tab->GetView()) {
			window->ShowReplaceItemsDialog(true);
		}
	}
}

void MainMenuBar::OnRemoveItemOnSelection(wxCommandEvent& WXUNUSED(event))
{
	if(!g_editor.IsProjectOpen())
		return;

	FindItemDialog dialog(frame, "Remove Item on Selection");
	if(dialog.ShowModal() == wxID_OK) {
		g_editor.GetCurrentEditor()->clearActions();
		g_editor.CreateLoadBar("Searching item on selection to remove...");
		OnMapRemoveItems::RemoveItemCondition condition(dialog.getResultID());
		int64_t count = RemoveItemOnMap(g_editor.GetCurrentMap(), condition, true);
		g_editor.DestroyLoadBar();

		wxString msg;
		msg << count << " items removed.";
		g_editor.PopupDialog("Remove Item", msg, wxOK);
		g_editor.GetCurrentMap().doChange();
		g_editor.RefreshView();
	}
	dialog.Destroy();
}

void MainMenuBar::OnSelectionTypeChange(wxCommandEvent& WXUNUSED(event))
{
	g_settings.setInteger(Config::COMPENSATED_SELECT, IsItemChecked(MenuBar::SELECT_MODE_COMPENSATE));

	if(IsItemChecked(MenuBar::SELECT_MODE_CURRENT))
		g_settings.setInteger(Config::SELECTION_TYPE, SELECT_CURRENT_FLOOR);
	else if(IsItemChecked(MenuBar::SELECT_MODE_LOWER))
		g_settings.setInteger(Config::SELECTION_TYPE, SELECT_ALL_FLOORS);
	else if(IsItemChecked(MenuBar::SELECT_MODE_VISIBLE))
		g_settings.setInteger(Config::SELECTION_TYPE, SELECT_VISIBLE_FLOORS);
}

void MainMenuBar::OnCopy(wxCommandEvent& WXUNUSED(event))
{
	g_editor.DoCopy();
}

void MainMenuBar::OnCut(wxCommandEvent& WXUNUSED(event))
{
	g_editor.DoCut();
}

void MainMenuBar::OnPaste(wxCommandEvent& WXUNUSED(event))
{
	g_editor.PreparePaste();
}

void MainMenuBar::OnToggleAutomagic(wxCommandEvent& WXUNUSED(event))
{
	g_settings.setInteger(Config::USE_AUTOMAGIC, IsItemChecked(MenuBar::AUTOMAGIC));
	g_settings.setInteger(Config::BORDER_IS_GROUND, IsItemChecked(MenuBar::AUTOMAGIC));
	if(g_settings.getInteger(Config::USE_AUTOMAGIC))
		g_editor.SetStatusText("Automagic enabled.");
	else
		g_editor.SetStatusText("Automagic disabled.");
}

void MainMenuBar::OnBorderizeSelection(wxCommandEvent& WXUNUSED(event))
{
	if(!g_editor.IsProjectOpen())
		return;

	g_editor.GetCurrentEditor()->borderizeSelection();
	g_editor.RefreshView();
}

void MainMenuBar::OnBorderizeMap(wxCommandEvent& WXUNUSED(event))
{
	if(!g_editor.IsProjectOpen())
		return;

	int ret = g_editor.PopupDialog("Borderize Map", "Are you sure you want to borderize the entire map (this action cannot be undone)?", wxYES | wxNO);
	if(ret == wxID_YES)
		g_editor.GetCurrentEditor()->borderizeMap(true);

	g_editor.RefreshView();
}

void MainMenuBar::OnRandomizeSelection(wxCommandEvent& WXUNUSED(event))
{
	if(!g_editor.IsProjectOpen())
		return;

	g_editor.GetCurrentEditor()->randomizeSelection();
	g_editor.RefreshView();
}

void MainMenuBar::OnRandomizeMap(wxCommandEvent& WXUNUSED(event))
{
	if(!g_editor.IsProjectOpen())
		return;

	int ret = g_editor.PopupDialog("Randomize Map", "Are you sure you want to randomize the entire map (this action cannot be undone)?", wxYES | wxNO);
	if(ret == wxID_YES)
		g_editor.GetCurrentEditor()->randomizeMap(true);

	g_editor.RefreshView();
}

void MainMenuBar::OnJumpToBrush(wxCommandEvent& WXUNUSED(event))
{
	if(!g_editor.IsProjectOpen())
		return;

	// Create the jump to dialog
	FindDialog* dlg = newd FindBrushDialog(frame);

	// Display dialog to user
	dlg->ShowModal();

	// Retrieve result, if null user canceled
	const Brush* brush = dlg->getResult();
	if(brush) {
		g_editor.SelectBrush(brush, TILESET_UNKNOWN);
	}
	delete dlg;
}

void MainMenuBar::OnJumpToItemBrush(wxCommandEvent& WXUNUSED(event))
{
	if(!g_editor.IsProjectOpen())
		return;

	// Create the jump to dialog
	FindItemDialog dialog(frame, "Jump to Item");
	dialog.setSearchMode((FindItemDialog::SearchMode)g_settings.getInteger(Config::JUMP_TO_ITEM_MODE));
	if(dialog.ShowModal() == wxID_OK) {
		// Retrieve result, if null user canceled
		const Brush* brush = dialog.getResult();
		if(brush)
			g_editor.SelectBrush(brush, TILESET_RAW);
		g_settings.setInteger(Config::JUMP_TO_ITEM_MODE, (int)dialog.getSearchMode());
	}
	dialog.Destroy();
}

void MainMenuBar::OnGotoPreviousPosition(wxCommandEvent& WXUNUSED(event))
{
	MapTab* mapTab = g_editor.GetCurrentMapTab();
	if(mapTab)
		mapTab->GoToPreviousCenterPosition();
}

void MainMenuBar::OnGotoPosition(wxCommandEvent& WXUNUSED(event))
{
	if(!g_editor.IsProjectOpen())
		return;

	// Display dialog, it also controls the actual jump
	GotoPositionDialog dlg(frame, *g_editor.GetCurrentEditor());
	dlg.ShowModal();
}

void MainMenuBar::OnMapRemoveItems(wxCommandEvent& WXUNUSED(event))
{
	if(!g_editor.IsProjectOpen())
		return;

	FindItemDialog dialog(frame, "Item Type to Remove");
	if(dialog.ShowModal() == wxID_OK) {
		uint16_t itemid = dialog.getResultID();

		g_editor.GetCurrentEditor()->getSelection().clear();
		g_editor.GetCurrentEditor()->clearActions();

		OnMapRemoveItems::RemoveItemCondition condition(itemid);
		g_editor.CreateLoadBar("Searching map for items to remove...");

		int64_t count = RemoveItemOnMap(g_editor.GetCurrentMap(), condition, false);

		g_editor.DestroyLoadBar();

		wxString msg;
		msg << count << " items deleted.";

		g_editor.PopupDialog("Search completed", msg, wxOK);
		g_editor.GetCurrentMap().doChange();
		g_editor.RefreshView();
	}
	dialog.Destroy();
}

namespace OnMapRemoveCorpses
{
	struct condition
	{
		condition() {}

		bool operator()(Map& map, const Item* item, long long removed, long long done){
			if(done % 0x800 == 0)
				g_editor.SetLoadDone((unsigned int)(100 * done / map.getTileCount()));

			return g_materials.isInTileset(item, "Corpses");
		}
	};
}

void MainMenuBar::OnMapRemoveCorpses(wxCommandEvent& WXUNUSED(event))
{
	if(!g_editor.IsProjectOpen())
		return;

	int ok = g_editor.PopupDialog("Remove Corpses", "Do you want to remove all corpses from the map?", wxYES | wxNO);

	if(ok == wxID_YES) {
		g_editor.GetCurrentEditor()->getSelection().clear();
		g_editor.GetCurrentEditor()->clearActions();

		OnMapRemoveCorpses::condition func;
		g_editor.CreateLoadBar("Searching map for items to remove...");

		int64_t count = RemoveItemOnMap(g_editor.GetCurrentMap(), func, false);

		g_editor.DestroyLoadBar();

		wxString msg;
		msg << count << " items deleted.";
		g_editor.PopupDialog("Search completed", msg, wxOK);
		g_editor.GetCurrentMap().doChange();
	}
}

namespace OnMapRemoveUnreachable
{
	struct condition
	{
		condition() {}

		bool isReachable(Tile* tile)
		{
			return tile && !tile->getFlag(UNPASS);
		}

		bool operator()(Map& map, Tile* tile, long long removed, long long done, long long total)
		{
			if(done % 0x1000 == 0)
				g_editor.SetLoadDone((unsigned int)(100 * done / total));

			const Position& pos = tile->getPosition();
			int sx = std::max(pos.x - 10, 0);
			int ex = std::min(pos.x + 10, 65535);
			int sy = std::max(pos.y - 8,  0);
			int ey = std::min(pos.y + 8,  65535);
			int sz, ez;

			if(pos.z < 8) {
				sz = 0;
				ez = 9;
			} else {
				// underground
				sz = std::max(pos.z - 2, rme::MapGroundLayer);
				ez = std::min(pos.z + 2, rme::MapMaxLayer);
			}

			for(int z = sz; z <= ez; ++z) {
				for(int y = sy; y <= ey; ++y) {
					for(int x = sx; x <= ex; ++x) {
						if(isReachable(map.getTile(x, y, z)))
							return false;
					}
				}
			}
			return true;
		}
	};
}

void MainMenuBar::OnMapRemoveUnreachable(wxCommandEvent& WXUNUSED(event))
{
	if(!g_editor.IsProjectOpen())
		return;

	int ok = g_editor.PopupDialog("Remove Unreachable Tiles", "Do you want to remove all unreachable items from the map?", wxYES | wxNO);

	if(ok == wxID_YES) {
		g_editor.GetCurrentEditor()->getSelection().clear();
		g_editor.GetCurrentEditor()->clearActions();

		OnMapRemoveUnreachable::condition func;
		g_editor.CreateLoadBar("Searching map for tiles to remove...");

		long long removed = remove_if_TileOnMap(g_editor.GetCurrentMap(), func);

		g_editor.DestroyLoadBar();

		wxString msg;
		msg << removed << " tiles deleted.";

		g_editor.PopupDialog("Search completed", msg, wxOK);

		g_editor.GetCurrentMap().doChange();
	}
}

void MainMenuBar::OnMapRemoveEmptySpawns(wxCommandEvent& WXUNUSED(event))
{
	if(!g_editor.IsProjectOpen()) {
		return;
	}

	int ok = g_editor.PopupDialog("Remove Empty Spawns", "Do you want to remove all empty spawns from the map?", wxYES | wxNO);
	if(ok == wxID_YES) {
		Editor* editor = g_editor.GetCurrentEditor();
		editor->getSelection().clear();

		g_editor.CreateLoadBar("Searching map for empty spawns to remove...");

		Map& map = g_editor.GetCurrentMap();
		CreatureVector creatures;
		std::vector<Tile*> toDeleteSpawns;
		for(const auto& spawnPosition : map.spawns) {
			Tile* tile = map.getTile(spawnPosition);
			if(!tile || !tile->spawn) {
				continue;
			}

			const int32_t radius = tile->spawn->getSize();

			bool empty = true;
			for(int32_t y = -radius; y <= radius; ++y) {
				for(int32_t x = -radius; x <= radius; ++x) {
					Tile* creature_tile = map.getTile(spawnPosition + Position(x, y, 0));
					if(creature_tile && creature_tile->creature && !creature_tile->creature->isSaved()) {
						creature_tile->creature->save();
						creatures.push_back(creature_tile->creature);
						empty = false;
					}
				}
			}

			if(empty) {
				toDeleteSpawns.push_back(tile);
			}
		}

		for(Creature* creature : creatures) {
			creature->reset();
		}

		BatchAction* batch = editor->createBatch(ACTION_DELETE_TILES);
		Action* action = editor->createAction(batch);

		const size_t count = toDeleteSpawns.size();
		size_t removed = 0;
		for(const auto& tile : toDeleteSpawns) {
			Tile* newtile = tile->deepCopy(map);
			map.removeSpawn(newtile);
			delete newtile->spawn;
			newtile->spawn = nullptr;
			if(++removed % 5 == 0) {
				// update progress bar for each 5 spawns removed
				g_editor.SetLoadDone(100 * removed / count);
			}
			action->addChange(newd Change(newtile));
		}

		batch->addAndCommitAction(action);
		editor->addBatch(batch);

		g_editor.DestroyLoadBar();

		wxString msg;
		msg << removed << " empty spawns removed.";
		g_editor.PopupDialog("Search completed", msg, wxOK);
		g_editor.GetCurrentMap().doChange();
	}
}

void MainMenuBar::OnClearHouseTiles(wxCommandEvent& WXUNUSED(event))
{
	Editor* editor = g_editor.GetCurrentEditor();
	if(!editor)
		return;

	int ret = g_editor.PopupDialog(
		"Clear Invalid House Tiles",
		"Are you sure you want to remove all house tiles that do not belong to a house (this action cannot be undone)?",
		wxYES | wxNO
	);

	if(ret == wxID_YES) {
		// Editor will do the work
		editor->clearInvalidHouseTiles(true);
	}

	g_editor.RefreshView();
}

void MainMenuBar::OnClearModifiedState(wxCommandEvent& WXUNUSED(event))
{
	Editor* editor = g_editor.GetCurrentEditor();
	if(!editor)
		return;

	int ret = g_editor.PopupDialog(
		"Clear Modified State",
		"This will have the same effect as closing the map and opening it again. Do you want to proceed?",
		wxYES | wxNO
	);

	if(ret == wxID_YES) {
		// Editor will do the work
		editor->clearModifiedTileState(true);
	}

	g_editor.RefreshView();
}

void MainMenuBar::OnMapCleanHouseItems(wxCommandEvent& WXUNUSED(event))
{
	Editor* editor = g_editor.GetCurrentEditor();
	if(!editor)
		return;

	int ret = g_editor.PopupDialog(
		"Clear Moveable House Items",
		"Are you sure you want to remove all items inside houses that can be moved (this action cannot be undone)?",
		wxYES | wxNO
	);

	if(ret == wxID_YES) {
		// Editor will do the work
		//editor->removeHouseItems(true);
	}

	g_editor.RefreshView();
}

void MainMenuBar::OnMapEditTowns(wxCommandEvent& WXUNUSED(event))
{
	if(g_editor.GetCurrentEditor()) {
		wxDialog* town_dialog = newd EditTownsDialog(frame, *g_editor.GetCurrentEditor());
		town_dialog->ShowModal();
		town_dialog->Destroy();
	}
}

void MainMenuBar::OnMapEditItems(wxCommandEvent& WXUNUSED(event))
{
	;
}

void MainMenuBar::OnMapEditMonsters(wxCommandEvent& WXUNUSED(event))
{
	;
}

void MainMenuBar::OnMapStatistics(wxCommandEvent& WXUNUSED(event))
{
	if(!g_editor.IsProjectOpen())
		return;

	g_editor.CreateLoadBar("Collecting data...");

	Map* map = &g_editor.GetCurrentMap();

	int load_counter = 0;

	uint64_t tile_count = 0;
	uint64_t detailed_tile_count = 0;
	uint64_t blocking_tile_count = 0;
	uint64_t walkable_tile_count = 0;
	double percent_pathable = 0.0;
	double percent_detailed = 0.0;
	uint64_t spawn_count = 0;
	uint64_t creature_count = 0;
	double creatures_per_spawn = 0.0;

	uint64_t item_count = 0;
	uint64_t loose_item_count = 0;
	uint64_t depot_count = 0;
	uint64_t action_item_count = 0;
	uint64_t unique_item_count = 0;
	uint64_t container_count = 0; // Only includes containers containing more than 1 item

	int town_count = map->towns.count();
	int house_count = map->houses.count();
	std::map<uint32_t, uint32_t> town_sqm_count;
	const Town* largest_town = nullptr;
	uint64_t largest_town_size = 0;
	uint64_t total_house_sqm = 0;
	const House* largest_house = nullptr;
	uint64_t largest_house_size = 0;
	double houses_per_town = 0.0;
	double sqm_per_house = 0.0;
	double sqm_per_town = 0.0;

	for(MapIterator mit = map->begin(); mit != map->end(); ++mit) {
		Tile* tile = (*mit)->get();
		if(load_counter % 8192 == 0) {
			g_editor.SetLoadDone((unsigned int)(int64_t(load_counter) * 95ll / int64_t(map->getTileCount())));
		}

		if(tile->empty())
			continue;

		tile_count += 1;

		// TODO(fusion): Relevant srv flags/attributes instead?
		bool is_detailed = false;
		for(const Item *item = tile->items; item != NULL; item = item->next){
			item_count += 1;
			if(!item->getFlag(BANK) && !item->getFlag(CLIP)) {
				is_detailed = true;
				if(!item->getFlag(UNMOVE)) {
					loose_item_count += 1;
				}
				if(item->getFlag(CONTAINER) || item->getFlag(CHEST)){
					if(item->content != NULL) {
						container_count += 1;
					}
				}
			}
		}

		if(tile->spawn)
			spawn_count += 1;

		if(tile->creature)
			creature_count += 1;

		if(tile->getFlag(UNPASS))
			blocking_tile_count += 1;
		else
			walkable_tile_count += 1;

		if(is_detailed)
			detailed_tile_count += 1;

		load_counter += 1;
	}

	creatures_per_spawn = (spawn_count != 0 ? double(creature_count) / double(spawn_count) : -1.0);
	percent_pathable = 100.0*(tile_count != 0 ? double(walkable_tile_count) / double(tile_count) : -1.0);
	percent_detailed = 100.0*(tile_count != 0 ? double(detailed_tile_count) / double(tile_count) : -1.0);

	load_counter = 0;
	Houses& houses = map->houses;
	for(HouseMap::const_iterator hit = houses.begin(); hit != houses.end(); ++hit) {
		const House* house = hit->second;

		if(load_counter % 64)
			g_editor.SetLoadDone((unsigned int)(95ll + int64_t(load_counter) * 5ll / int64_t(house_count)));

		if(house->size() > largest_house_size) {
			largest_house = house;
			largest_house_size = house->size();
		}
		total_house_sqm += house->size();
		town_sqm_count[house->townid] += house->size();
	}

	houses_per_town = (town_count != 0?  double(house_count) /     double(town_count)  : -1.0);
	sqm_per_house   = (house_count != 0? double(total_house_sqm) / double(house_count) : -1.0);
	sqm_per_town    = (town_count != 0?  double(total_house_sqm) / double(town_count)  : -1.0);

	Towns& towns = map->towns;
	for(std::map<uint32_t, uint32_t>::iterator town_iter = town_sqm_count.begin();
			town_iter != town_sqm_count.end();
			++town_iter)
	{
		// No load bar for this, load is non-existant
		uint32_t town_id = town_iter->first;
		uint32_t town_sqm = town_iter->second;
		Town* town = towns.getTown(town_id);
		if(town && town_sqm > largest_town_size) {
			largest_town = town;
			largest_town_size = town_sqm;
		} else {
			// Non-existant town!
		}
	}

	g_editor.DestroyLoadBar();

	std::ostringstream os;
	os.setf(std::ios::fixed, std::ios::floatfield);
	os.precision(2);
	os << "Map statistics for the map \"" << map->getMapDescription() << "\"\n";
	os << "\tTile data:\n";
	os << "\t\tTotal number of tiles: " << tile_count << "\n";
	os << "\t\tNumber of pathable tiles: " << walkable_tile_count << "\n";
	os << "\t\tNumber of unpathable tiles: " << blocking_tile_count << "\n";
	if(percent_pathable >= 0.0)
		os << "\t\tPercent walkable tiles: " << percent_pathable << "%\n";
	os << "\t\tDetailed tiles: " << detailed_tile_count << "\n";
	if(percent_detailed >= 0.0)
		os << "\t\tPercent detailed tiles: " << percent_detailed << "%\n";

	os << "\tItem data:\n";
	os << "\t\tTotal number of items: " << item_count << "\n";
	os << "\t\tNumber of moveable tiles: " << loose_item_count << "\n";
	os << "\t\tNumber of depots: " << depot_count << "\n";
	os << "\t\tNumber of containers: " << container_count << "\n";
	os << "\t\tNumber of items with Action ID: " << action_item_count << "\n";
	os << "\t\tNumber of items with Unique ID: " << unique_item_count << "\n";

	os << "\tCreature data:\n";
	os << "\t\tTotal creature count: " << creature_count << "\n";
	os << "\t\tTotal spawn count: " << spawn_count << "\n";
	if(creatures_per_spawn >= 0)
		os << "\t\tMean creatures per spawn: " << creatures_per_spawn << "\n";

	os << "\tTown/House data:\n";
	os << "\t\tTotal number of towns: " << town_count << "\n";
	os << "\t\tTotal number of houses: " << house_count << "\n";
	if(houses_per_town >= 0)
		os << "\t\tMean houses per town: " << houses_per_town << "\n";
	os << "\t\tTotal amount of housetiles: " << total_house_sqm << "\n";
	if(sqm_per_house >= 0)
		os << "\t\tMean tiles per house: " << sqm_per_house << "\n";
	if(sqm_per_town >= 0)
		os << "\t\tMean tiles per town: " << sqm_per_town << "\n";

	if(largest_town)
		os << "\t\tLargest Town: \"" << largest_town->getName() << "\" (" << largest_town_size << " sqm)\n";
	if(largest_house)
		os << "\t\tLargest House: \"" << largest_house->name << "\" (" << largest_house_size << " sqm)\n";

	os << "\n";
	os << "Generated by Remere's Map Editor version " + __RME_VERSION__ + "\n";


    wxDialog* dg = newd wxDialog(frame, wxID_ANY, "Map Statistics", wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER | wxCAPTION | wxCLOSE_BOX);
	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);
	wxTextCtrl* text_field = newd wxTextCtrl(dg, wxID_ANY, wxstr(os.str()), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
	text_field->SetMinSize(wxSize(400, 300));
	topsizer->Add(text_field, wxSizerFlags(5).Expand());

	wxSizer* choicesizer = newd wxBoxSizer(wxHORIZONTAL);
	wxButton* export_button = newd wxButton(dg, wxID_OK, "Export as XML");
	choicesizer->Add(export_button, wxSizerFlags(1).Center());
	export_button->Enable(false);
	choicesizer->Add(newd wxButton(dg, wxID_CANCEL, "OK"), wxSizerFlags(1).Center());
	topsizer->Add(choicesizer, wxSizerFlags(1).Center());
	dg->SetSizerAndFit(topsizer);
	dg->Centre(wxBOTH);

	int ret = dg->ShowModal();

	if(ret == wxID_OK) {
		//std::cout << "XML EXPORT";
	} else if(ret == wxID_CANCEL) {
		//std::cout << "OK";
	}
}

void MainMenuBar::OnMapCleanup(wxCommandEvent& WXUNUSED(event))
{
	int ok = g_editor.PopupDialog("Clean map", "Do you want to remove all invalid items from the map?", wxYES | wxNO);

	if(ok == wxID_YES)
		g_editor.GetCurrentMap().cleanInvalidTiles(true);
}

void MainMenuBar::OnMapProperties(wxCommandEvent& WXUNUSED(event))
{
	wxDialog* properties = newd MapPropertiesWindow(
		frame,
		static_cast<MapTab*>(g_editor.GetCurrentTab()),
		*g_editor.GetCurrentEditor());

	if(properties->ShowModal() == 0) {
		// FAIL!
		g_editor.CloseAllEditors();
	}
	properties->Destroy();
}

void MainMenuBar::OnToolbars(wxCommandEvent& event)
{
	using namespace MenuBar;

	ActionID id = static_cast<ActionID>(event.GetId() - (wxID_HIGHEST + 1));
	switch (id) {
		case VIEW_TOOLBARS_BRUSHES:
			g_editor.ShowToolbar(TOOLBAR_BRUSHES, event.IsChecked());
			g_settings.setInteger(Config::SHOW_TOOLBAR_BRUSHES, event.IsChecked());
			break;
		case VIEW_TOOLBARS_POSITION:
			g_editor.ShowToolbar(TOOLBAR_POSITION, event.IsChecked());
			g_settings.setInteger(Config::SHOW_TOOLBAR_POSITION, event.IsChecked());
			break;
		case VIEW_TOOLBARS_SIZES:
			g_editor.ShowToolbar(TOOLBAR_SIZES, event.IsChecked());
			g_settings.setInteger(Config::SHOW_TOOLBAR_SIZES, event.IsChecked());
			break;
		case VIEW_TOOLBARS_INDICATORS:
			g_editor.ShowToolbar(TOOLBAR_INDICATORS, event.IsChecked());
			g_settings.setInteger(Config::SHOW_TOOLBAR_INDICATORS, event.IsChecked());
			break;
		case VIEW_TOOLBARS_STANDARD:
			g_editor.ShowToolbar(TOOLBAR_STANDARD, event.IsChecked());
			g_settings.setInteger(Config::SHOW_TOOLBAR_STANDARD, event.IsChecked());
			break;
	    default:
	        break;
	}
}

void MainMenuBar::OnNewView(wxCommandEvent& WXUNUSED(event))
{
	g_editor.NewMapView();
}

void MainMenuBar::OnToggleFullscreen(wxCommandEvent& WXUNUSED(event))
{
	if(frame->IsFullScreen())
		frame->ShowFullScreen(false);
	else
		frame->ShowFullScreen(true, wxFULLSCREEN_NOBORDER | wxFULLSCREEN_NOCAPTION);
}

void MainMenuBar::OnTakeScreenshot(wxCommandEvent& WXUNUSED(event))
{
	wxString path = wxstr(g_settings.getString(Config::SCREENSHOT_DIRECTORY));
	if(path.size() > 0 && (path.Last() == '/' || path.Last() == '\\'))
		path = path + "/";

	g_editor.GetCurrentMapTab()->GetView()->GetCanvas()->TakeScreenshot(
		path, wxstr(g_settings.getString(Config::SCREENSHOT_FORMAT))
	);

}

void MainMenuBar::OnZoomIn(wxCommandEvent& event)
{
	double zoom = g_editor.GetCurrentZoom();
	g_editor.SetCurrentZoom(zoom - 0.1);
}

void MainMenuBar::OnZoomOut(wxCommandEvent& event)
{
	double zoom = g_editor.GetCurrentZoom();
	g_editor.SetCurrentZoom(zoom + 0.1);
}

void MainMenuBar::OnZoomNormal(wxCommandEvent& event)
{
	g_editor.SetCurrentZoom(1.0);
}

void MainMenuBar::OnChangeViewSettings(wxCommandEvent& event)
{
	g_settings.setInteger(Config::SHOW_ALL_FLOORS, IsItemChecked(MenuBar::SHOW_ALL_FLOORS));
	if(IsItemChecked(MenuBar::SHOW_ALL_FLOORS)) {
		EnableItem(MenuBar::SELECT_MODE_VISIBLE, true);
		EnableItem(MenuBar::SELECT_MODE_LOWER, true);
	} else {
		EnableItem(MenuBar::SELECT_MODE_VISIBLE, false);
		EnableItem(MenuBar::SELECT_MODE_LOWER, false);
		CheckItem(MenuBar::SELECT_MODE_CURRENT, true);
		g_settings.setInteger(Config::SELECTION_TYPE, SELECT_CURRENT_FLOOR);
	}
	g_settings.setInteger(Config::TRANSPARENT_FLOORS, IsItemChecked(MenuBar::GHOST_HIGHER_FLOORS));
	g_settings.setInteger(Config::TRANSPARENT_ITEMS, IsItemChecked(MenuBar::GHOST_ITEMS));
	g_settings.setInteger(Config::SHOW_INGAME_BOX, IsItemChecked(MenuBar::SHOW_INGAME_BOX));
	g_settings.setInteger(Config::SHOW_LIGHTS, IsItemChecked(MenuBar::SHOW_LIGHTS));
	g_settings.setInteger(Config::SHOW_GRID, IsItemChecked(MenuBar::SHOW_GRID));
	g_settings.setInteger(Config::SHOW_EXTRA, !IsItemChecked(MenuBar::SHOW_EXTRA));

	g_settings.setInteger(Config::SHOW_SHADE, IsItemChecked(MenuBar::SHOW_SHADE));
	g_settings.setInteger(Config::SHOW_SPECIAL_TILES, IsItemChecked(MenuBar::SHOW_SPECIAL));
	g_settings.setInteger(Config::SHOW_AS_MINIMAP, IsItemChecked(MenuBar::SHOW_AS_MINIMAP));
	g_settings.setInteger(Config::SHOW_ONLY_TILEFLAGS, IsItemChecked(MenuBar::SHOW_ONLY_COLORS));
	g_settings.setInteger(Config::SHOW_ONLY_MODIFIED_TILES, IsItemChecked(MenuBar::SHOW_ONLY_MODIFIED));
	g_settings.setInteger(Config::SHOW_CREATURES, IsItemChecked(MenuBar::SHOW_CREATURES));
	g_settings.setInteger(Config::SHOW_SPAWNS, IsItemChecked(MenuBar::SHOW_SPAWNS));
	g_settings.setInteger(Config::SHOW_HOUSES, IsItemChecked(MenuBar::SHOW_HOUSES));
	g_settings.setInteger(Config::HIGHLIGHT_ITEMS, IsItemChecked(MenuBar::HIGHLIGHT_ITEMS));
	g_settings.setInteger(Config::SHOW_BLOCKING, IsItemChecked(MenuBar::SHOW_PATHING));
	g_settings.setInteger(Config::SHOW_TOOLTIPS, IsItemChecked(MenuBar::SHOW_TOOLTIPS));
	g_settings.setInteger(Config::SHOW_PREVIEW, IsItemChecked(MenuBar::SHOW_PREVIEW));
	g_settings.setInteger(Config::SHOW_WALL_HOOKS, IsItemChecked(MenuBar::SHOW_WALL_HOOKS));
	g_settings.setInteger(Config::SHOW_PICKUPABLES, IsItemChecked(MenuBar::SHOW_PICKUPABLES));
	g_settings.setInteger(Config::SHOW_MOVEABLES, IsItemChecked(MenuBar::SHOW_MOVEABLES));

	g_editor.RefreshView();
	g_editor.toolbar->UpdateIndicators();
}

void MainMenuBar::OnChangeFloor(wxCommandEvent& event)
{
	// Workaround to stop events from looping
	if(checking_programmaticly)
		return;

	for(int i = 0; i < 16; ++i) {
		if(IsItemChecked(MenuBar::ActionID(MenuBar::FLOOR_0 + i))) {
			g_editor.ChangeFloor(i);
		}
	}
}

void MainMenuBar::OnMinimapWindow(wxCommandEvent& event)
{
	g_editor.CreateMinimap();
}

void MainMenuBar::OnActionsHistoryWindow(wxCommandEvent& WXUNUSED(event))
{
	g_editor.ShowActionsWindow();
}

void MainMenuBar::OnNewPalette(wxCommandEvent& event)
{
	g_editor.NewPalette();
}

void MainMenuBar::OnSelectTerrainPalette(wxCommandEvent& WXUNUSED(event))
{
	g_editor.SelectPalettePage(TILESET_TERRAIN);
}

void MainMenuBar::OnSelectDoodadPalette(wxCommandEvent& WXUNUSED(event))
{
	g_editor.SelectPalettePage(TILESET_DOODAD);
}

void MainMenuBar::OnSelectItemPalette(wxCommandEvent& WXUNUSED(event))
{
	g_editor.SelectPalettePage(TILESET_ITEM);
}

void MainMenuBar::OnSelectHousePalette(wxCommandEvent& WXUNUSED(event))
{
	g_editor.SelectPalettePage(TILESET_HOUSE);
}

void MainMenuBar::OnSelectCreaturePalette(wxCommandEvent& WXUNUSED(event))
{
	g_editor.SelectPalettePage(TILESET_CREATURE);
}

void MainMenuBar::OnSelectWaypointPalette(wxCommandEvent& WXUNUSED(event))
{
	g_editor.SelectPalettePage(TILESET_WAYPOINT);
}

void MainMenuBar::OnSelectRawPalette(wxCommandEvent& WXUNUSED(event))
{
	g_editor.SelectPalettePage(TILESET_RAW);
}

void MainMenuBar::SearchItems(bool container, bool writable, bool onSelection/* = false*/)
{
	if(!container && !writable)
		return;

	if(!g_editor.IsProjectOpen())
		return;

	if(onSelection)
		g_editor.CreateLoadBar("Searching on selected area...");
	else
		g_editor.CreateLoadBar("Searching on map...");

	OnSearchForStuff::Searcher searcher;
	searcher.search_container = container;
	searcher.search_writable = writable;

	foreach_ItemOnMap(g_editor.GetCurrentMap(), searcher, onSelection);
	searcher.sort();

	g_editor.DestroyLoadBar();

	SearchResultWindow* result = g_editor.ShowSearchWindow();
	result->Clear();

	for(const auto &p: searcher.found){
		result->AddPosition(searcher.desc(p.second), p.first->getPosition());
	}
}

void MainMenuBar::SearchDuplicatedItems(bool selection)
{
	if(!g_editor.IsProjectOpen()) {
		return;
	}

	auto dialog = g_editor.ShowDuplicatedItemsWindow();
	dialog->StartSearch(g_editor.GetCurrentMapTab(), selection);
}
