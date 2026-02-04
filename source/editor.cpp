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
#include "editor.h"

#include <wx/display.h>

#include "actions_history_window.h"
#include "application.h"
#include "brush.h"
#include "common_windows.h"
#include "doodad_brush.h"
#include "duplicated_items_window.h"
#include "ground_brush.h"
#include "gui_ids.h"
#include "house_exit_brush.h"
#include "main_menubar.h"
#include "map.h"
#include "map_display.h"
#include "map_window.h"
#include "materials.h"
#include "minimap_window.h"
#include "palette_window.h"
#include "result_window.h"
#include "spawn_brush.h"
#include "sprites.h"
#include "waypoint_brush.h"
#include "welcome_dialog.h"

#ifdef __WXOSX__
#include <AGL/agl.h>
#endif

const wxEventType EVT_UPDATE_MENUS = wxNewEventType();
const wxEventType EVT_UPDATE_ACTIONS = wxNewEventType();

Editor g_editor;

Editor::Editor()
{
	actionQueue = newd ActionQueue();
	doodad_buffer_map = newd BaseMap();
}

Editor::~Editor()
{
	// NOTE(fusion): This is a "singleton" anyways, so this only gets executed
	// at exit. Let the OS reclaim any resources, any persistent data should
	// already have been saved to disk.
}

wxGLContext &Editor::GetGLContext(wxGLCanvas* win)
{
	if(OGLContext == nullptr) {
		OGLContext = newd wxGLContext(win);
    }

	return *OGLContext;
}

void Editor::LoadRecentFiles()
{
	recentFiles.Load(g_settings.getConfigObject());
}

void Editor::SaveRecentFiles()
{
	recentFiles.Save(g_settings.getConfigObject());
}

void Editor::AddRecentFile(FileName file)
{
	recentFiles.AddFileToHistory(file.GetFullPath());
}

std::vector<wxString> Editor::GetRecentFiles()
{
    std::vector<wxString> files(recentFiles.GetCount());
    for(size_t i = 0; i < recentFiles.GetCount(); ++i) {
        files[i] = recentFiles.GetHistoryFile(i);
    }
    return files;
}

void Editor::NewProject(void)
{
	// TODO(fusion): Create new project? Which doesn't make a lot of sense if
	// a project also includes objects, materials, etc... so we might want to
	// have some kind of project template?
	OpenProject();
}

void Editor::OpenProject(void)
{
	wxDirDialog openDialog(root, "Select project directory...", wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	if(openDialog.ShowModal() == wxID_OK){
		OpenProject(openDialog.GetPath());
	}
}

void Editor::OpenProject(FileName filename)
{
	if(IsProjectOpen()){
		// Prompt for a save, then close?
	}

	FinishWelcomeDialog();

	//Load everything here!
	//CreateLoadBar();
	//LoadItemTypes();
	//LoadSprites();
	//LoadMap();
	//...

	SetStatusText("");
	UpdateTitle();
	RefreshPalettes();
	UpdateMenubar();
	root->Refresh();
}

void Editor::CloseProject(void)
{
	if(!IsProjectOpen()){
		return;
	}
}

void Editor::SaveProject(void)
{
	if(!IsProjectOpen()){
		return;
	}
}

void Editor::SaveProjectAs(void)
{
	if(!IsProjectOpen()){
		return;
	}
}

bool Editor::IsProjectOpen(void) const {
	return false;
}

bool Editor::IsProjectDirty(void) const {
	return false;
}

#if 0
bool Editor::LoadDataFiles(wxString& error, wxArrayString& warnings)
{
	// TODO(fusion): We want to move all this loading to when the map is loaded.
	// This should also involve making the editor load a single map at a time,
	// with all the loaded assets being related to it. And I'm still not sure
	// what every asset here means, so we might still want to cull/merge some
	// of them as well. Maybe have a single `editor.xml` that can be placed in
	// the root directory of the project and loaded along with srv and sec files.

	//wxString projectPath = ?.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
	wxString projectPath = ".";

	CreateLoadBar("Loading assets...");

	SetLoadDone(0, "Loading DAT file...");
	wxFileName metadata_path = gfx.getMetadataFileName();
	if(!gfx.loadSpriteMetadata(metadata_path, error, warnings)) {
		error = "Couldn't load metadata: " + error;
		DestroyLoadBar();
		UnloadVersion();
		return false;
	}

	SetLoadDone(10, "Loading SPR file...");
	wxFileName sprites_path = gfx.getSpritesFileName();
	if(!gfx.loadSpriteData(sprites_path.GetFullPath(), error, warnings)) {
		error = "Couldn't load sprites: " + error;
		DestroyLoadBar();
		UnloadVersion();
		return false;
	}

	SetLoadDone(25, "Loading objects.srv file...");
	if(!LoadItemTypes(projectPath + "objects.srv", error, warnings)){
		error = "Couldn't load items.otb: " + error;
		DestroyLoadBar();
		UnloadVersion();
		return false;
	}

	SetLoadDone(45, "Loading creatures.xml ...");
	if(!g_creatures.loadFromXML(projectPath + "creatures.xml", true, error, warnings)) {
		warnings.push_back("Couldn't load creatures.xml: " + error);
	}

	SetLoadDone(50, "Loading materials.xml ...");
	if(!g_materials.loadMaterials(projectPath + "materials.xml", error, warnings)) {
		warnings.push_back("Couldn't load materials.xml: " + error);
	}

	SetLoadDone(70, "Finishing...");
	g_brushes.init();
	g_materials.createOtherTileset();

	DestroyLoadBar();
	return true;
}

bool Editor::LoadVersion(wxString& error, wxArrayString& warnings, bool force)
{
	if(ClientVersion::get(version) == nullptr) {
		error = "Unsupported client version! (8)";
		return false;
	}

	if(version != loaded_version || force) {
		if(getLoadedVersion() != nullptr)
			// There is another version loaded right now, save window layout
			SavePerspective();

		// Disable all rendering so the data is not accessed while reloading
		UnnamedRenderingLock();
		DestroyPalettes();
		DestroyMinimap();

		// Destroy the previous version
		UnloadVersion();

		loaded_version = version;
		if(!getLoadedVersion()->hasValidPaths()) {
			if(!getLoadedVersion()->loadValidPaths()) {
				error = "Couldn't load relevant asset files";
				loaded_version = CLIENT_VERSION_NONE;
				return false;
			}
		}

		bool ret = LoadDataFiles(error, warnings);
		if(ret)
			LoadPerspective();
		else
			loaded_version = CLIENT_VERSION_NONE;

		return ret;
	}
	return true;
}

void Editor::UnloadVersion()
{
	UnnamedRenderingLock();
	gfx.clear();
	current_brush = nullptr;
	previous_brush = nullptr;

	house_brush = nullptr;
	house_exit_brush = nullptr;
	waypoint_brush = nullptr;
	optional_brush = nullptr;
	eraser = nullptr;
	normal_door_brush = nullptr;
	locked_door_brush = nullptr;
	magic_door_brush = nullptr;
	quest_door_brush = nullptr;
	hatch_door_brush = nullptr;
	window_door_brush = nullptr;

	if(loaded_version != CLIENT_VERSION_NONE) {
		//UnloadVersion();
		g_materials.clear();
		g_brushes.clear();
		ClearItemTypes();
		gfx.clear();

		loaded_version = CLIENT_VERSION_NONE;
	}
}

void Editor::SaveCurrentMap(FileName filename, bool showdialog)
{
	MapTab* mapTab = GetCurrentMapTab();
	if(mapTab) {
		Editor* editor = mapTab->GetEditor();
		if(editor) {
			editor->saveMap(filename, showdialog);

			const std::string& filename = editor->getMap().getFilename();
			const Position& position = mapTab->GetScreenCenterPosition();
			std::ostringstream stream;
			stream << position;
			g_settings.setString(Config::RECENT_EDITED_MAP_PATH, filename);
			g_settings.setString(Config::RECENT_EDITED_MAP_POSITION, stream.str());
		}
	}

	UpdateTitle();
	UpdateMenubar();
	root->Refresh();
}

void Editor::FitViewToMap(MapTab* mt)
{
	for(int index = 0; index < tabbook->GetTabCount(); ++index) {
		if(auto *tab = dynamic_cast<MapTab*>(tabbook->GetTab(index))) {
			if(tab->HasSameReference(mt)) {
				tab->GetView()->FitToMap();
			}
		}
	}
}

bool Editor::NewMap()
{
    FinishWelcomeDialog();

	Editor* editor;
	try
	{
		editor = newd Editor(copybuffer);
	}
	catch(std::runtime_error& e)
	{
		PopupDialog(root, "Error!", wxString(e.what(), wxConvUTF8), wxOK);
		return false;
	}

	auto *mapTab = newd MapTab(tabbook, editor);
	mapTab->OnSwitchEditorMode(mode);
    editor->clearChanges();

	SetStatusText("Created new map");
	UpdateTitle();
	RefreshPalettes();
	UpdateMenubar();
	root->Refresh();
	return true;
}

void Editor::OpenMap()
{
	wxString wildcard = g_settings.getInteger(Config::USE_OTGZ) != 0 ? MAP_LOAD_FILE_WILDCARD_OTGZ : MAP_LOAD_FILE_WILDCARD;
	wxFileDialog dialog(root, "Open map file", wxEmptyString, wxEmptyString, wildcard, wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if(dialog.ShowModal() == wxID_OK)
		LoadMap(dialog.GetPath());
}

void Editor::SaveMap()
{
	if(!IsProjectOpen())
		return;

	if(GetCurrentMap().hasFile()) {
		SaveCurrentMap(true);
	} else {
		wxString wildcard = g_settings.getInteger(Config::USE_OTGZ) != 0 ? MAP_SAVE_FILE_WILDCARD_OTGZ : MAP_SAVE_FILE_WILDCARD;
		wxFileDialog dialog(root, "Save...", wxEmptyString, wxEmptyString, wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

		if(dialog.ShowModal() == wxID_OK)
			SaveCurrentMap(dialog.GetPath(), true);
	}
}

void Editor::SaveMapAs()
{
	if(!IsProjectOpen())
		return;

	wxString wildcard = g_settings.getInteger(Config::USE_OTGZ) != 0 ? MAP_SAVE_FILE_WILDCARD_OTGZ : MAP_SAVE_FILE_WILDCARD;
	wxFileDialog dialog(root, "Save As...", "", "", wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if(dialog.ShowModal() == wxID_OK) {
		SaveCurrentMap(dialog.GetPath(), true);
		UpdateTitle();
		AddRecentFile(dialog.GetPath());
		UpdateMenubar();
	}
}

bool Editor::LoadMap(const FileName& fileName)
{
    FinishWelcomeDialog();

	if(GetCurrentEditor() && !GetCurrentMap().hasChanged() && !GetCurrentMap().hasFile())
		CloseCurrentEditor();

	Editor* editor;
	try
	{
		editor = newd Editor(copybuffer, fileName);
	}
	catch(std::runtime_error& e)
	{
		PopupDialog(root, "Error!", wxString(e.what(), wxConvUTF8), wxOK);
		return false;
	}

	auto *mapTab = newd MapTab(tabbook, editor);
	mapTab->OnSwitchEditorMode(mode);

	AddRecentFile(fileName);

	mapTab->GetView()->FitToMap();
	UpdateTitle();
	ListDialog("Map loader errors", mapTab->GetMap()->getWarnings());
	root->DoQueryImportCreatures();

	FitViewToMap(mapTab);
	UpdateMenubar();

	std::string path = g_settings.getString(Config::RECENT_EDITED_MAP_PATH);
	if(!path.empty()) {
		FileName file(path);
		if(file == fileName) {
			std::istringstream stream(g_settings.getString(Config::RECENT_EDITED_MAP_POSITION));
			Position position;
			stream >> position;
			mapTab->SetScreenCenterPosition(position);
		}
	}
	return true;
}

bool Editor::ShouldSave()
{
	Editor* editor = GetCurrentEditor();
	ASSERT(editor);
	return editor->hasChanges();
}


bool Editor::hasChanges() const
{
	if(map.hasChanged()) {
		if(map.getTileCount() == 0) {
			return actionQueue->hasChanges();
		}
		return true;
	}
	return false;
}

void Editor::clearChanges()
{
	map.clearChanges();
}

void Editor::saveMap(FileName filename, bool showdialog)
{
	std::string savefile = filename.GetFullPath().mb_str(wxConvUTF8).data();
	bool save_as = false;

	if(savefile.empty()) {
		savefile = map.filename;

		FileName c1(wxstr(savefile));
		FileName c2(wxstr(map.filename));
		save_as = c1 != c2;
	}

	// If not named yet, propagate the file name to the auxilliary files
	if(map.unnamed) {
		FileName _name(filename);
		_name.SetExt("xml");

		_name.SetName(filename.GetName() + "-spawn");
		map.spawnfile = nstr(_name.GetFullName());
		_name.SetName(filename.GetName() + "-house");
		map.housefile = nstr(_name.GetFullName());

		map.unnamed = false;
	}

	// File object to convert between local paths etc.
	FileName converter;
	converter.Assign(wxstr(savefile));
	std::string map_path = nstr(converter.GetPath(wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME));

	// Make temporary backups
	//converter.Assign(wxstr(savefile));
	std::string backup_otbm, backup_house, backup_spawn;

	if(converter.FileExists()) {
		backup_otbm = map_path + nstr(converter.GetName()) + ".otbm~";
		std::remove(backup_otbm.c_str());
		std::rename(savefile.c_str(), backup_otbm.c_str());
	}

	converter.SetFullName(wxstr(map.housefile));
	if(converter.FileExists()) {
		backup_house = map_path + nstr(converter.GetName()) + ".xml~";
		std::remove(backup_house.c_str());
		std::rename((map_path + map.housefile).c_str(), backup_house.c_str());
	}

	converter.SetFullName(wxstr(map.spawnfile));
	if(converter.FileExists()) {
		backup_spawn = map_path + nstr(converter.GetName()) + ".xml~";
		std::remove(backup_spawn.c_str());
		std::rename((map_path + map.spawnfile).c_str(), backup_spawn.c_str());
	}

	// Save the map
	{
		std::string n = nstr(GetLocalDataDirectory()) + ".saving.txt";
		std::ofstream f(n.c_str(), std::ios::trunc | std::ios::out);
		f <<
			backup_otbm << std::endl <<
			backup_house << std::endl <<
			backup_spawn << std::endl;
	}

	{
		// Set up the Map paths
		wxFileName fn = wxstr(savefile);
		map.filename = fn.GetFullPath().mb_str(wxConvUTF8);
		map.name = fn.GetFullName().mb_str(wxConvUTF8);

		if(showdialog)
			CreateLoadBar("Saving OTBM map...");

		// Perform the actual save
		IOMapOTBM mapsaver(map.getVersion());
		bool success = mapsaver.saveMap(map, fn);

		if(showdialog)
			DestroyLoadBar();

		// Check for errors...
		if(!success) {
			// Rename the temporary backup files back to their previous names
			if(!backup_otbm.empty()) {
				converter.SetFullName(wxstr(savefile));
				std::string otbm_filename = map_path + nstr(converter.GetName());
				std::rename(backup_otbm.c_str(), std::string(otbm_filename + ".otbm").c_str());
			}

			if(!backup_house.empty()) {
				converter.SetFullName(wxstr(map.housefile));
				std::string house_filename = map_path + nstr(converter.GetName());
				std::rename(backup_house.c_str(), std::string(house_filename + ".xml").c_str());
			}

			if(!backup_spawn.empty()) {
				converter.SetFullName(wxstr(map.spawnfile));
				std::string spawn_filename = map_path + nstr(converter.GetName());
				std::rename(backup_spawn.c_str(), std::string(spawn_filename + ".xml").c_str());
			}

			// Display the error
			PopupDialog("Error", "Could not save, unable to open target for writing.", wxOK);
		}

		// Remove temporary save runfile
		{
			std::string n = nstr(GetLocalDataDirectory()) + ".saving.txt";
			std::remove(n.c_str());
		}

		if(!success)
			return;
	}

	// Move to permanent backup
	if(!save_as && g_settings.getInteger(Config::ALWAYS_MAKE_BACKUP)) {
		// Move temporary backups to their proper files
		time_t t = time(nullptr);
		tm* current_time = localtime(&t);
		ASSERT(current_time);

		std::ostringstream date;
		date << (1900 + current_time->tm_year);
		if(current_time->tm_mon < 9)
			date << "-" << "0" << current_time->tm_mon+1;
		else
			date << "-" << current_time->tm_mon+1;
		date << "-" << current_time->tm_mday;
		date << "-" << current_time->tm_hour;
		date << "-" << current_time->tm_min;
		date << "-" << current_time->tm_sec;

		if(!backup_otbm.empty()) {
			converter.SetFullName(wxstr(savefile));
			std::string otbm_filename = map_path + nstr(converter.GetName());
			std::rename(backup_otbm.c_str(), std::string(otbm_filename + "." + date.str() + ".otbm").c_str());
		}

		if(!backup_house.empty()) {
			converter.SetFullName(wxstr(map.housefile));
			std::string house_filename = map_path + nstr(converter.GetName());
			std::rename(backup_house.c_str(), std::string(house_filename + "." + date.str() + ".xml").c_str());
		}

		if(!backup_spawn.empty()) {
			converter.SetFullName(wxstr(map.spawnfile));
			std::string spawn_filename = map_path + nstr(converter.GetName());
			std::rename(backup_spawn.c_str(), std::string(spawn_filename + "." + date.str() + ".xml").c_str());
		}
	} else {
		// Delete the temporary files
		std::remove(backup_otbm.c_str());
		std::remove(backup_house.c_str());
		std::remove(backup_spawn.c_str());
	}

	clearChanges();
}
#endif

double Editor::GetCurrentZoom()
{
	return mapWindow->GetCanvas()->GetZoom();
}

void Editor::SetCurrentZoom(double zoom)
{
	mapWindow->GetCanvas()->SetZoom(zoom);
}

void Editor::FitViewToMap()
{
	mapWindow->FitToMap();
}

void Editor::AddPendingMapEvent(wxEvent &event)
{
	mapWindow->GetEventHandler()->AddPendingEvent(event);
}

void Editor::AddPendingCanvasEvent(wxEvent &event)
{
	mapWindow->GetCanvas()->GetEventHandler()->AddPendingEvent(event);
}

void Editor::LoadPerspective()
{
	std::vector<std::string> palette_list;
	{
		std::string tmp;
		std::string layout = g_settings.getString(Config::PALETTE_LAYOUT);
		for(char c : layout) {
			if(c == '|') {
				palette_list.push_back(tmp);
				tmp.clear();
			} else {
				tmp.push_back(c);
			}
		}

		if(!tmp.empty()) {
			palette_list.push_back(tmp);
		}
	}

	for(const std::string& name : palette_list) {
		PaletteWindow* palette = CreatePalette();
		wxAuiPaneInfo &pane = aui_manager->GetPane(palette);
		aui_manager->LoadPaneInfo(wxstr(name), pane);
		if(pane.IsFloatable()) {
			bool offscreen = true;
			for(uint32_t index = 0; index < wxDisplay::GetCount(); ++index) {
				wxDisplay display(index);
				wxRect rect = display.GetClientArea();
				if(rect.Contains(pane.floating_pos)) {
					offscreen = false;
					break;
				}
			}

			if(offscreen) {
				pane.Dock();
			}
		}
	}

	if(g_settings.getInteger(Config::MINIMAP_VISIBLE)) {
		wxString layout = wxstr(g_settings.getString(Config::MINIMAP_LAYOUT));
		if(!minimap) {
			wxAuiPaneInfo pane;
			aui_manager->LoadPaneInfo(layout, pane);
			minimap = newd MinimapWindow(root);
			aui_manager->AddPane(minimap, pane);
		} else {
			wxAuiPaneInfo &pane = aui_manager->GetPane(minimap);
			aui_manager->LoadPaneInfo(layout, pane);
		}

		wxAuiPaneInfo &pane = aui_manager->GetPane(minimap);
		if(pane.IsFloatable()) {
			bool offscreen = true;
			for(uint32_t index = 0; index < wxDisplay::GetCount(); ++index) {
				wxDisplay display(index);
				wxRect rect = display.GetClientArea();
				if(rect.Contains(pane.floating_pos)) {
					offscreen = false;
					break;
				}
			}

			if(offscreen) {
				pane.Dock();
			}
		}
	}

	if(g_settings.getInteger(Config::ACTIONS_HISTORY_VISIBLE)) {
		wxString layout = wxstr(g_settings.getString(Config::ACTIONS_HISTORY_LAYOUT));
		if(!actions_history_window) {
			wxAuiPaneInfo pane;
			aui_manager->LoadPaneInfo(layout, pane);
			actions_history_window = new ActionsHistoryWindow(root);
			aui_manager->AddPane(actions_history_window, pane);
		} else {
			wxAuiPaneInfo &pane = aui_manager->GetPane(actions_history_window);
			aui_manager->LoadPaneInfo(layout, pane);
		}

		wxAuiPaneInfo& pane = aui_manager->GetPane(actions_history_window);
		if(pane.IsFloatable()) {
			bool offscreen = true;
			for(uint32_t index = 0; index < wxDisplay::GetCount(); ++index) {
				wxDisplay display(index);
				wxRect rect = display.GetClientArea();
				if(rect.Contains(pane.floating_pos)) {
					offscreen = false;
					break;
				}
			}

			if(offscreen) {
				pane.Dock();
			}
		}
	}

	aui_manager->Update();
	UpdateMenubar();
	toolbar->LoadPerspective();
}

void Editor::SavePerspective()
{
	g_settings.setInteger(Config::WINDOW_MAXIMIZED, root->IsMaximized());
	g_settings.setInteger(Config::WINDOW_WIDTH, root->GetSize().GetWidth());
	g_settings.setInteger(Config::WINDOW_HEIGHT, root->GetSize().GetHeight());
	g_settings.setInteger(Config::MINIMAP_VISIBLE, minimap? 1: 0);
	g_settings.setInteger(Config::ACTIONS_HISTORY_VISIBLE, actions_history_window ? 1 : 0);

	{
		wxString layout;
		for(auto &palette : palettes) {
			if(aui_manager->GetPane(palette).IsShown())
				layout << aui_manager->SavePaneInfo(aui_manager->GetPane(palette)) << "|";
		}
		g_settings.setString(Config::PALETTE_LAYOUT, nstr(layout));
	}

	if(minimap) {
		wxString layout = aui_manager->SavePaneInfo(aui_manager->GetPane(minimap));
		g_settings.setString(Config::MINIMAP_LAYOUT, nstr(layout));
	}

	if(actions_history_window) {
		wxString layout = aui_manager->SavePaneInfo(aui_manager->GetPane(actions_history_window));
		g_settings.setString(Config::ACTIONS_HISTORY_LAYOUT, nstr(layout));
	}

	toolbar->SavePerspective();
}

void Editor::HideSearchWindow()
{
	if(search_result_window) {
		aui_manager->GetPane(search_result_window).Show(false);
		aui_manager->Update();
	}
}

SearchResultWindow* Editor::ShowSearchWindow()
{
	if(search_result_window == nullptr) {
		search_result_window = newd SearchResultWindow(root);
		aui_manager->AddPane(search_result_window, wxAuiPaneInfo().Caption("Search Results"));
	} else {
		aui_manager->GetPane(search_result_window).Show();
	}
	aui_manager->Update();
	return search_result_window;
}

DuplicatedItemsWindow* Editor::ShowDuplicatedItemsWindow()
{
	if(!duplicated_items_window) {
		duplicated_items_window = new DuplicatedItemsWindow(root);
		aui_manager->AddPane(duplicated_items_window, wxAuiPaneInfo().Caption("Duplicated Items"));
	} else {
		aui_manager->GetPane(duplicated_items_window).Show();
	}
	aui_manager->Update();
	return duplicated_items_window;
}

void Editor::HideDuplicatedItemsWindow()
{
	if(duplicated_items_window) {
		aui_manager->GetPane(duplicated_items_window).Show(false);
		aui_manager->Update();
	}
}

ActionsHistoryWindow* Editor::ShowActionsWindow()
{
	if(!actions_history_window) {
		actions_history_window = new ActionsHistoryWindow(root);
		aui_manager->AddPane(actions_history_window, wxAuiPaneInfo().Caption("Actions History"));
	} else {
		aui_manager->GetPane(actions_history_window).Show();
	}

	aui_manager->Update();
	actions_history_window->RefreshActions();
	return actions_history_window;
}

void Editor::HideActionsWindow()
{
	if(actions_history_window) {
		aui_manager->GetPane(actions_history_window).Show(false);
		aui_manager->Update();
	}
}

//=============================================================================
// Palette Window Interface implementation

PaletteWindow* Editor::GetPalette()
{
	if(palettes.empty())
		return nullptr;
	return palettes.front();
}

PaletteWindow* Editor::NewPalette()
{
	return CreatePalette();
}

void Editor::RefreshPalettes()
{
	for(auto&palette : palettes) {
		palette->OnUpdate(&map);
	}
	SelectBrush();

	if(duplicated_items_window) {
		duplicated_items_window->UpdateButtons();
	}

	RefreshActions();
}

void Editor::RefreshOtherPalettes(PaletteWindow* p)
{
	for(auto &palette : palettes) {
		if(palette != p)
			palette->OnUpdate(&map);
	}
	SelectBrush();
}

PaletteWindow* Editor::CreatePalette()
{
	if(!IsProjectOpen())
		return nullptr;

	auto *palette = newd PaletteWindow(root, g_materials.tilesets);
	aui_manager->AddPane(palette, wxAuiPaneInfo().Caption("Palette").TopDockable(false).BottomDockable(false));
	aui_manager->Update();

	// Make us the active palette
	palettes.push_front(palette);
	// Select brush from this palette
	SelectBrushInternal(palette->GetSelectedBrush());

	return palette;
}

void Editor::ActivatePalette(PaletteWindow* p)
{
	palettes.erase(std::find(palettes.begin(), palettes.end(), p));
	palettes.push_front(p);
}

void Editor::DestroyPalettes()
{
	for(auto palette : palettes) {
		aui_manager->DetachPane(palette);
		palette->Destroy();
		palette = nullptr;
	}
	palettes.clear();
	aui_manager->Update();
}

void Editor::RebuildPalettes()
{
	// Palette lits might be modified due to active palette changes
	// Use a temporary list for iterating
	std::list<PaletteWindow*> tmp = palettes;
	for(auto &piter : tmp) {
		piter->ReloadSettings(&map);
	}
	aui_manager->Update();
}

void Editor::ShowPalette()
{
	if(palettes.empty())
		return;

	for(auto &palette : palettes) {
		if(aui_manager->GetPane(palette).IsShown())
			return;
	}

	aui_manager->GetPane(palettes.front()).Show(true);
	aui_manager->Update();
}

void Editor::SelectPalettePage(PaletteType pt)
{
	if(palettes.empty())
		CreatePalette();
	PaletteWindow* p = GetPalette();
	if(!p)
		return;

	ShowPalette();
	p->SelectPage(pt);
	aui_manager->Update();
	SelectBrushInternal(p->GetSelectedBrush());
}

//=============================================================================
// Minimap Window Interface Implementation

void Editor::CreateMinimap()
{
	if(!IsProjectOpen())
		return;

	if(minimap) {
		aui_manager->GetPane(minimap).Show(true);
	} else {
		minimap = newd MinimapWindow(root);
		minimap->Show(true);
		aui_manager->AddPane(minimap, wxAuiPaneInfo().Caption("Minimap"));
	}
	aui_manager->Update();
}

void Editor::HideMinimap()
{
	if(minimap) {
		aui_manager->GetPane(minimap).Show(false);
		aui_manager->Update();
	}
}

void Editor::DestroyMinimap()
{
	if(minimap) {
		aui_manager->DetachPane(minimap);
		aui_manager->Update();
		minimap->Destroy();
		minimap = nullptr;
	}
}

void Editor::UpdateMinimap(bool immediate)
{
	if(IsMinimapVisible()) {
		if(immediate) {
			minimap->Refresh();
		} else {
			minimap->DelayedUpdate();
		}
	}
}

bool Editor::IsMinimapVisible() const
{
	if(minimap) {
		const wxAuiPaneInfo& pi = aui_manager->GetPane(minimap);
		if(pi.IsShown()) {
			return true;
		}
	}
	return false;
}

//=============================================================================

void Editor::RefreshView()
{
	mapWindow->Refresh();
}

void Editor::CreateLoadBar(wxString message, bool canCancel /* = false */ )
{
	progressText = message;

	progressFrom = 0;
	progressTo = 100;
	currentProgress = -1;

	progressBar = newd wxGenericProgressDialog("Loading", progressText + " (0%)", 100, root,
		wxPD_APP_MODAL | wxPD_SMOOTH | (canCancel ? wxPD_CAN_ABORT : 0)
	);
	progressBar->SetSize(280, -1);
	progressBar->Show(true);
	progressBar->Update(0);
}

void Editor::SetLoadScale(int32_t from, int32_t to)
{
	progressFrom = from;
	progressTo = to;
}

bool Editor::SetLoadDone(int32_t done, const wxString& newMessage)
{
	if(done == 100) {
		DestroyLoadBar();
		return true;
	} else if(done == currentProgress) {
		return true;
	}

	if(!newMessage.empty()) {
		progressText = newMessage;
	}

	int32_t newProgress = progressFrom + static_cast<int32_t>((done / 100.f) * (progressTo - progressFrom));
	newProgress = std::max<int32_t>(0, std::min<int32_t>(100, newProgress));

	bool skip = false;
	if(progressBar) {
		progressBar->Update(
			newProgress,
			wxString::Format("%s (%d%%)", progressText, newProgress),
			&skip
		);
		currentProgress = newProgress;
	}

	return skip;
}

void Editor::DestroyLoadBar()
{
	if(progressBar) {
		progressBar->Show(false);
		currentProgress = -1;

		progressBar->Destroy();
		progressBar = nullptr;

		if(root->IsActive()) {
			root->Raise();
		} else {
			root->RequestUserAttention();
		}
	}
}

void Editor::ShowWelcomeDialog(const wxBitmap &icon) {
    std::vector<wxString> recent_files = GetRecentFiles();
    welcomeDialog = newd WelcomeDialog(__W_RME_APPLICATION_NAME__, "Version " + __W_RME_VERSION__, FROM_DIP(root, wxSize(800, 480)), icon, recent_files);
    welcomeDialog->Bind(wxEVT_CLOSE_WINDOW, &Editor::OnWelcomeDialogClosed, this);
    welcomeDialog->Bind(WELCOME_DIALOG_ACTION, &Editor::OnWelcomeDialogAction, this);
    welcomeDialog->Show();
    UpdateMenubar();
}

void Editor::FinishWelcomeDialog() {
    if(welcomeDialog != nullptr) {
        welcomeDialog->Hide();
		root->Show();
        welcomeDialog->Destroy();
        welcomeDialog = nullptr;
    }
}

bool Editor::IsWelcomeDialogShown() {
    return welcomeDialog != nullptr && welcomeDialog->IsShown();
}

void Editor::OnWelcomeDialogClosed(wxCloseEvent &event)
{
    welcomeDialog->Destroy();
    root->Close();
}

void Editor::OnWelcomeDialogAction(wxCommandEvent &event)
{
    if(event.GetId() == wxID_NEW) {
        NewProject();
    } else if(event.GetId() == wxID_OPEN) {
        OpenProject(FileName(event.GetString()));
    }
}

void Editor::UpdateMenubar()
{
	menubar->Update();
	toolbar->UpdateButtons();
}

void Editor::SetScreenCenterPosition(const Position& position, bool showIndicator)
{
	mapWindow->SetScreenCenterPosition(position, showIndicator);
}

void Editor::DoCut()
{
	if(!IsSelectionMode())
		return;

	copybuffer.cut(GetCurrentFloor());
	RefreshView();
	UpdateMenubar();
}

void Editor::DoCopy()
{
	if(!IsSelectionMode())
		return;

	copybuffer.copy(GetCurrentFloor());
	RefreshView();
	UpdateMenubar();
}

void Editor::DoPaste()
{
	copybuffer.paste(mapWindow->GetCanvas()->GetCursorPosition());
}

void Editor::PreparePaste()
{
	SetSelectionMode();
	selection.start();
	selection.clear();
	selection.finish();
	StartPasting();
	RefreshView();
}

void Editor::StartPasting()
{
	pasting = true;
	secondary_map = &copybuffer.getBufferMap();
}

void Editor::EndPasting()
{
	if(pasting) {
		pasting = false;
		secondary_map = nullptr;
	}
}

void Editor::DoUndo(int numActions /*= 1*/)
{
	if(canUndo() && numActions > 0){
		undo(numActions);
		if(hasSelection())
			SetSelectionMode();
		SetStatusText("Undo action");
		UpdateMinimap();
		UpdateMenubar();
		root->Refresh();
	}
}

void Editor::DoRedo(int numActions /*= 1*/)
{
	if(canRedo() && numActions > 0) {
		redo(numActions);
		if(hasSelection())
			SetSelectionMode();
		SetStatusText("Redo action");
		UpdateMinimap();
		UpdateMenubar();
		root->Refresh();
	}
}

int Editor::GetCurrentFloor()
{
	return mapWindow->GetCanvas()->GetFloor();
}

void Editor::ChangeFloor(int new_floor)
{
	int old_floor = GetCurrentFloor();
	if(new_floor < rme::MapMinLayer || new_floor > rme::MapMaxLayer)
		return;

	if(old_floor != new_floor)
		mapWindow->GetCanvas()->ChangeFloor(new_floor);
}

void Editor::SetStatusText(wxString text)
{
	root->SetStatusText(text, 0);
}

void Editor::SetTitle(wxString title)
{
	if(root == nullptr)
		return;

	if(!title.empty()) {
		root->SetTitle(title << " - Remere's Map Editor");
	} else {
		root->SetTitle(wxString("Remere's Map Editor"));
	}
}

void Editor::UpdateTitle()
{
	SetTitle(projectDir);
}

void Editor::UpdateMenus()
{
	wxCommandEvent evt(EVT_UPDATE_MENUS);
	root->AddPendingEvent(evt);
}

void Editor::UpdateActions()
{
	wxCommandEvent evt(EVT_UPDATE_ACTIONS);
	root->AddPendingEvent(evt);
}

void Editor::RefreshActions()
{
	if(actions_history_window)
		actions_history_window->RefreshActions();
}

void Editor::ShowToolbar(ToolBarID id, bool show)
{
	if(toolbar) // ?
		toolbar->Show(id, show);
}

void Editor::SwitchMode()
{
	if(mode == DRAWING_MODE) {
		SetSelectionMode();
	} else {
		SetDrawingMode();
	}
}

void Editor::SetSelectionMode()
{
	if(mode == SELECTION_MODE)
		return;

	if(current_brush && current_brush->isDoodad()) {
		secondary_map = nullptr;
	}

	mapWindow->OnSwitchEditorMode(SELECTION_MODE);
	mode = SELECTION_MODE;
}

void Editor::SetDrawingMode()
{
	if(mode == DRAWING_MODE)
		return;

	if(current_brush && current_brush->isDoodad()) {
		secondary_map = doodad_buffer_map;
	} else {
		secondary_map = nullptr;
	}

	selection.start(Selection::NONE, ACTION_UNSELECT);
	selection.clear();
	selection.finish();
	selection.updateSelectionCount();
	mapWindow->OnSwitchEditorMode(DRAWING_MODE);
	mode = DRAWING_MODE;
}

void Editor::SetBrushSizeInternal(int nz)
{
	if(nz != brush_size && current_brush && current_brush->isDoodad() && !current_brush->oneSizeFitsAll()) {
		brush_size = nz;
		FillDoodadPreviewBuffer();
		secondary_map = doodad_buffer_map;
	} else {
		brush_size = nz;
	}
}

void Editor::SetBrushSize(int nz)
{
	SetBrushSizeInternal(nz);

	for(auto &palette : palettes) {
		palette->OnUpdateBrushSize(brush_shape, brush_size);
	}

	toolbar->UpdateBrushSize(brush_shape, brush_size);
}

void Editor::SetBrushVariation(int nz)
{
	if(nz != brush_variation && current_brush && current_brush->isDoodad()) {
		// Monkey!
		brush_variation = nz;
		FillDoodadPreviewBuffer();
		secondary_map = doodad_buffer_map;
	}
}

void Editor::SetBrushShape(BrushShape bs)
{
	if(bs != brush_shape && current_brush && current_brush->isDoodad() && !current_brush->oneSizeFitsAll()) {
		// Donkey!
		brush_shape = bs;
		FillDoodadPreviewBuffer();
		secondary_map = doodad_buffer_map;
	}
	brush_shape = bs;

	for(auto &palette : palettes) {
		palette->OnUpdateBrushSize(brush_shape, brush_size);
	}

	toolbar->UpdateBrushSize(brush_shape, brush_size);
}

void Editor::SetBrushThickness(bool on, int x, int y)
{
	use_custom_thickness = on;

	if(x != -1 || y != -1) {
		custom_thickness_mod = std::max<float>(x, 1.f) / std::max<float>(y, 1.f);
	}

	if(current_brush && current_brush->isDoodad()) {
		FillDoodadPreviewBuffer();
	}

	RefreshView();
}

void Editor::SetBrushThickness(int low, int ceil)
{
	custom_thickness_mod = std::max<float>(low, 1.f) / std::max<float>(ceil, 1.f);

	if(use_custom_thickness && current_brush && current_brush->isDoodad()) {
		FillDoodadPreviewBuffer();
	}

	RefreshView();
}

void Editor::DecreaseBrushSize(bool wrap)
{
	switch(brush_size) {
		case 0: {
			if(wrap) {
				SetBrushSize(11);
			}
			break;
		}
		case 1: {
			SetBrushSize(0);
			break;
		}
		case 2:
		case 3: {
			SetBrushSize(1);
			break;
		}
		case 4:
		case 5: {
			SetBrushSize(2);
			break;
		}
		case 6:
		case 7: {
			SetBrushSize(4);
			break;
		}
		case 8:
		case 9:
		case 10: {
			SetBrushSize(6);
			break;
		}
		case 11:
		default: {
			SetBrushSize(8);
			break;
		}
	}
}

void Editor::IncreaseBrushSize(bool wrap)
{
	switch(brush_size) {
		case 0: {
			SetBrushSize(1);
			break;
		}
		case 1: {
			SetBrushSize(2);
			break;
		}
		case 2:
		case 3: {
			SetBrushSize(4);
			break;
		}
		case 4:
		case 5: {
			SetBrushSize(6);
			break;
		}
		case 6:
		case 7: {
			SetBrushSize(8);
			break;
		}
		case 8:
		case 9:
		case 10: {
			SetBrushSize(11);
			break;
		}
		case 11:
		default: {
			if(wrap) {
				SetBrushSize(0);
			}
			break;
		}
	}
}

Brush* Editor::GetCurrentBrush() const
{
	return current_brush;
}

BrushShape Editor::GetBrushShape() const
{
	if(current_brush == spawn_brush)
		return BRUSHSHAPE_SQUARE;

	return brush_shape;
}

int Editor::GetBrushSize() const
{
	return brush_size;
}

int Editor::GetBrushVariation() const
{
	return brush_variation;
}

int Editor::GetSpawnTime() const
{
	return creature_spawntime;
}

void Editor::SelectBrush()
{
	if(palettes.empty())
		return;

	SelectBrushInternal(palettes.front()->GetSelectedBrush());

	RefreshView();
}

bool Editor::SelectBrush(const Brush* whatbrush, PaletteType primary)
{
	if(palettes.empty())
		if(!CreatePalette())
			return false;

	if(!palettes.front()->OnSelectBrush(whatbrush, primary))
		return false;

	SelectBrushInternal(const_cast<Brush*>(whatbrush));
	toolbar->UpdateBrushButtons();
	return true;
}

void Editor::SelectBrushInternal(Brush* brush)
{
	// Fear no evil don't you say no evil
	if(current_brush != brush && brush)
		previous_brush = current_brush;

	current_brush = brush;
	if(!current_brush)
		return;

	brush_variation = std::min(brush_variation, brush->getMaxVariation());
	FillDoodadPreviewBuffer();
	if(brush->isDoodad())
		secondary_map = doodad_buffer_map;

	SetDrawingMode();
	RefreshView();
}

void Editor::SelectPreviousBrush()
{
	if(previous_brush)
		SelectBrush(previous_brush);
}

void Editor::FillDoodadPreviewBuffer()
{
	if(!current_brush || !current_brush->isDoodad())
		return;

	doodad_buffer_map->clear();

	DoodadBrush* brush = current_brush->asDoodad();
	if(brush->isEmpty(GetBrushVariation()))
		return;

	int object_count = 0;
	int area;
	if(GetBrushShape() == BRUSHSHAPE_SQUARE) {
		area = 2*GetBrushSize();
		area = area*area + 1;
	} else {
		if(GetBrushSize() == 1) {
			// There is a huge deviation here with the other formula.
			area = 5;
		} else {
			area = int(0.5 + GetBrushSize() * GetBrushSize() * rme::PI);
		}
	}
	const int object_range = (use_custom_thickness ? int(area*custom_thickness_mod) : brush->getThickness() * area / std::max(1, brush->getThicknessCeiling()));
	const int final_object_count = std::max(1, object_range + random(object_range));

	Position center_pos(0x8000, 0x8000, 0x8);

	if(brush_size > 0 && !brush->oneSizeFitsAll()) {
		while(object_count < final_object_count) {
			int retries = 0;
			bool exit = false;

			// Try to place objects 5 times
			while(retries < 5 && !exit) {

				int pos_retries = 0;
				int xpos = 0, ypos = 0;
				bool found_pos = false;
				if(GetBrushShape() == BRUSHSHAPE_CIRCLE) {
					while(pos_retries < 5 && !found_pos) {
						xpos = random(-brush_size, brush_size);
						ypos = random(-brush_size, brush_size);
						float distance = sqrt(float(xpos*xpos) + float(ypos*ypos));
						if(distance < GetBrushSize() + 0.005) {
							found_pos = true;
						} else {
							++pos_retries;
						}
					}
				} else {
					found_pos = true;
					xpos = random(-brush_size, brush_size);
					ypos = random(-brush_size, brush_size);
				}

				if(!found_pos) {
					++retries;
					continue;
				}

				// Decide whether the zone should have a composite or several single objects.
				bool fail = false;
				if(random(brush->getTotalChance(GetBrushVariation())) <= brush->getCompositeChance(GetBrushVariation())) {
					// Composite
					const auto &composites = brush->getComposite(GetBrushVariation());

					// Figure out if the placement is valid
					for(const auto &composite: composites) {
						Position pos = center_pos + composite.offset + Position(xpos, ypos, 0);
						if(Tile *tile = doodad_buffer_map->getTile(pos)) {
							if(!tile->empty()) {
								fail = true;
								break;
							}
						}
					}
					if(fail) {
						++retries;
						break;
					}

					// Transfer items to the stack
					for(const auto &composite: composites) {
						Position pos = center_pos + composite.offset + Position(xpos, ypos, 0);
						Tile* tile = doodad_buffer_map->getTile(pos);
						if(!tile)
							tile = doodad_buffer_map->allocator(doodad_buffer_map->createTileL(pos));

						for(const Item *item = composite.first; item != NULL; item = item->next){
							tile->addItem(item->deepCopy());
						}

						doodad_buffer_map->setTile(tile->getPosition(), tile);
					}
					exit = true;
				} else if(brush->hasSingleObjects(GetBrushVariation())) {
					Position pos = center_pos + Position(xpos, ypos, 0);
					Tile* tile = doodad_buffer_map->getTile(pos);
					if(tile) {
						if(!tile->empty()) {
							fail = true;
							break;
						}
					} else {
						tile = doodad_buffer_map->allocator(doodad_buffer_map->createTileL(pos));
					}
					int variation = GetBrushVariation();
					brush->draw(doodad_buffer_map, tile, &variation);
					//std::cout << "\tpos: " << tile->getPosition() << std::endl;
					doodad_buffer_map->setTile(tile->getPosition(), tile);
					exit = true;
				}
				if(fail) {
					++retries;
					break;
				}
			}
			++object_count;
		}
	} else {
		if(brush->hasCompositeObjects(GetBrushVariation()) &&
				random(brush->getTotalChance(GetBrushVariation())) <= brush->getCompositeChance(GetBrushVariation())) {
			// All placement is valid...
			// Transfer items to the buffer
			for(const auto &composite : brush->getComposite(GetBrushVariation())) {
				Position pos = center_pos + composite.offset;
				Tile* tile = doodad_buffer_map->allocator(doodad_buffer_map->createTileL(pos));
				//std::cout << pos << " = " << center_pos << " + " << buffer_tile->getPosition() << std::endl;

				for(const Item *item = composite.first; item != NULL; item = item->next){
					tile->addItem(item->deepCopy());
				}

				doodad_buffer_map->setTile(tile->getPosition(), tile);
			}
		} else if(brush->hasSingleObjects(GetBrushVariation())) {
			Tile* tile = doodad_buffer_map->allocator(doodad_buffer_map->createTileL(center_pos));
			int variation = GetBrushVariation();
			brush->draw(doodad_buffer_map, tile, &variation);
			doodad_buffer_map->setTile(center_pos, tile);
		}
	}
}

long Editor::PopupDialog(wxWindow* parent, wxString title, wxString text, long style, wxString confisavename, uint32_t configsavevalue)
{
	if(text.empty())
		return wxID_ANY;

	wxMessageDialog dlg(parent, text, title, style);
	return dlg.ShowModal();
}

long Editor::PopupDialog(wxString title, wxString text, long style, wxString configsavename, uint32_t configsavevalue)
{
	return PopupDialog(root, title, text, style, configsavename, configsavevalue);
}

void Editor::ListDialog(wxWindow* parent, wxString title, const wxArrayString& param_items)
{
	if(param_items.empty())
		return;

	wxArrayString list_items(param_items);

	// Create the window
	wxDialog* dlg = newd wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER | wxCAPTION | wxCLOSE_BOX);

	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);
	wxListBox* item_list = newd wxListBox(dlg, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_SINGLE);
	item_list->SetMinSize(wxSize(500, 300));

	for(size_t i = 0; i != list_items.GetCount();) {
		wxString str = list_items[i];
		size_t pos = str.find("\n");
		if(pos != wxString::npos) {
			// Split string!
			item_list->Append(str.substr(0, pos));
			list_items[i] = str.substr(pos+1);
			continue;
		}
		item_list->Append(list_items[i]);
		++i;
	}
	sizer->Add(item_list, 1, wxEXPAND);

	wxSizer* stdsizer = newd wxBoxSizer(wxHORIZONTAL);
	stdsizer->Add(newd wxButton(dlg, wxID_OK, "OK"), wxSizerFlags(1).Center());
	sizer->Add(stdsizer, wxSizerFlags(0).Center());

	dlg->SetSizerAndFit(sizer);

	// Show the window
	dlg->ShowModal();
	delete dlg;
}

void Editor::ShowTextBox(wxWindow* parent, wxString title, wxString content)
{
	wxDialog* dlg = newd wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER | wxCAPTION | wxCLOSE_BOX);
	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);
	wxTextCtrl* text_field = newd wxTextCtrl(dlg, wxID_ANY, content, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
	text_field->SetMinSize(wxSize(400, 550));
	topsizer->Add(text_field, wxSizerFlags(5).Expand());

	wxSizer* choicesizer = newd wxBoxSizer(wxHORIZONTAL);
	choicesizer->Add(newd wxButton(dlg, wxID_CANCEL, "OK"), wxSizerFlags(1).Center());
	topsizer->Add(choicesizer, wxSizerFlags(0).Center());
	dlg->SetSizerAndFit(topsizer);

	dlg->ShowModal();
}

void Editor::SetHotkey(int index, Hotkey& hotkey)
{
	ASSERT(index >= 0 && index <= 9);
	hotkeys[index] = hotkey;
	SetStatusText("Set hotkey " + i2ws(index) + ".");
}

const Hotkey& Editor::GetHotkey(int index) const
{
	ASSERT(index >= 0 && index <= 9);
	return hotkeys[index];
}

void Editor::SaveHotkeys() const
{
	std::ostringstream os;
	for(const auto &hotkey : hotkeys) {
		os << hotkey << '\n';
	}
	g_settings.setString(Config::NUMERICAL_HOTKEYS, os.str());
}

void Editor::LoadHotkeys()
{
	std::istringstream is;
	is.str(g_settings.getString(Config::NUMERICAL_HOTKEYS));

	std::string line;
	int index = 0;
	while(getline(is, line)) {
		std::istringstream line_is;
		line_is.str(line);
		line_is >> hotkeys[index];

		++index;
	}
}

Action* Editor::createAction(ActionIdentifier type)
{
	return actionQueue->createAction(type);
}

Action* Editor::createAction(BatchAction* parent)
{
	return actionQueue->createAction(parent);
}

BatchAction* Editor::createBatch(ActionIdentifier type)
{
	return actionQueue->createBatch(type);
}

void Editor::addBatch(BatchAction* action, int stacking_delay)
{
	actionQueue->addBatch(action, stacking_delay);
}

void Editor::addAction(Action* action, int stacking_delay )
{
	actionQueue->addAction(action, stacking_delay);
}

bool Editor::canUndo() const
{
	return actionQueue->canUndo();
}

bool Editor::canRedo() const
{
	return actionQueue->canRedo();
}

void Editor::undo(int indexes)
{
	if(indexes <= 0 || !actionQueue->canUndo())
		return;

	while(indexes > 0) {
		if(!actionQueue->undo())
			break;
		indexes--;
	}
	UpdateActions();
	RefreshView();
}

void Editor::redo(int indexes)
{
	if(indexes <= 0 || !actionQueue->canRedo())
		return;

	while(indexes > 0) {
		if(!actionQueue->redo())
			break;
		indexes--;
	}
	UpdateActions();
	RefreshView();
}

void Editor::updateActions()
{
	actionQueue->generateLabels();
	UpdateMenus();
	UpdateActions();
}

void Editor::resetActionsTimer()
{
	actionQueue->resetTimer();
}

void Editor::clearActions()
{
	actionQueue->clear();
	UpdateActions();
}

bool Editor::importMiniMap(FileName filename, int import, int import_x_offset, int import_y_offset, int import_z_offset)
{
	return false;
}

bool Editor::importMap(FileName filename, int import_x_offset, int import_y_offset, int import_z_offset, ImportType house_import_type, ImportType spawn_import_type)
{
	selection.clear();
	actionQueue->clear();

	Map imported_map;
	bool loaded = imported_map.open(nstr(filename.GetFullPath()));

	if(!loaded) {
		PopupDialog("Error", "Error loading map!\n" + imported_map.getError(), wxOK | wxICON_INFORMATION);
		return false;
	}
	ListDialog("Warning", imported_map.getWarnings());

	Position offset(import_x_offset, import_y_offset, import_z_offset);

	bool resizemap = false;
	bool resize_asked = false;
	int newsize_x = map.getWidth(), newsize_y = map.getHeight();
	int discarded_tiles = 0;

	CreateLoadBar("Merging maps...");

	std::map<uint32_t, uint32_t> town_id_map;
	std::map<uint32_t, uint32_t> house_id_map;

	if(house_import_type != IMPORT_DONT) {
		for(TownMap::iterator tit = imported_map.towns.begin(); tit != imported_map.towns.end();) {
			Town* imported_town = tit->second;
			Town* current_town = map.towns.getTown(imported_town->getID());

			Position oldexit = imported_town->getTemplePosition();
			Position newexit = oldexit + offset;
			if(newexit.isValid()) {
				imported_town->setTemplePosition(newexit);
			}

			switch(house_import_type) {
				case IMPORT_MERGE: {
					town_id_map[imported_town->getID()] = imported_town->getID();
					if(current_town) {
						++tit;
						continue;
					}
					break;
				}
				case IMPORT_SMART_MERGE: {
					if(current_town) {
						// Compare and insert/merge depending on parameters
						if(current_town->getName() == imported_town->getName() && current_town->getID() == imported_town->getID()) {
							// Just add to map
							town_id_map[imported_town->getID()] = current_town->getID();
							++tit;
							continue;
						} else {
							// Conflict! Find a newd id and replace old
							uint32_t new_id = map.towns.getEmptyID();
							imported_town->setID(new_id);
							town_id_map[imported_town->getID()] = new_id;
						}
					} else {
						town_id_map[imported_town->getID()] = imported_town->getID();
					}
					break;
				}
				case IMPORT_INSERT: {
					// Find a newd id and replace old
					uint32_t new_id = map.towns.getEmptyID();
					imported_town->setID(new_id);
					town_id_map[imported_town->getID()] = new_id;
					break;
				}
				case IMPORT_DONT: {
					++tit;
					continue; // Should never happend..?
					break; // Continue or break ?
				}
			}

			map.towns.addTown(imported_town);

#ifdef __VISUALC__ // C++0x compliance to some degree :)
			tit = imported_map.towns.erase(tit);
#else // Bulky, slow way
			TownMap::iterator tmp_iter = tit;
			++tmp_iter;
			uint32_t next_key = 0;
			if(tmp_iter != imported_map.towns.end()) {
				next_key = tmp_iter->first;
			}
			imported_map.towns.erase(tit);
			if(next_key != 0) {
				tit = imported_map.towns.find(next_key);
			} else {
				tit = imported_map.towns.end();
			}
#endif
		}

		for(HouseMap::iterator hit = imported_map.houses.begin(); hit != imported_map.houses.end();) {
			House* imported_house = hit->second;
			House* current_house = map.houses.getHouse(imported_house->id);
			imported_house->townid = town_id_map[imported_house->townid];

			const Position& oldexit = imported_house->getExit();
			imported_house->setExit(nullptr, Position()); // Reset it

			switch(house_import_type) {
				case IMPORT_MERGE: {
					house_id_map[imported_house->id] = imported_house->id;
					if(current_house) {
						++hit;
						Position newexit = oldexit + offset;
						if(newexit.isValid()) current_house->setExit(&map, newexit);
						continue;
					}
					break;
				}
				case IMPORT_SMART_MERGE: {
					if(current_house) {
						// Compare and insert/merge depending on parameters
						if(current_house->name == imported_house->name && current_house->townid == imported_house->townid) {
							// Just add to map
							house_id_map[imported_house->id] = current_house->id;
							++hit;
							Position newexit = oldexit + offset;
							if(newexit.isValid()) imported_house->setExit(&map, newexit);
							continue;
						} else {
							// Conflict! Find a newd id and replace old
							uint32_t new_id = map.houses.getEmptyID();
							house_id_map[imported_house->id] = new_id;
							imported_house->id = new_id;
						}
					} else {
						house_id_map[imported_house->id] = imported_house->id;
					}
					break;
				}
				case IMPORT_INSERT: {
					// Find a newd id and replace old
					uint32_t new_id = map.houses.getEmptyID();
					house_id_map[imported_house->id] = new_id;
					imported_house->id = new_id;
					break;
				}
				case IMPORT_DONT: {
					++hit;
					Position newexit = oldexit + offset;
					if(newexit.isValid()) imported_house->setExit(&map, newexit);
						continue; // Should never happend..?
					break;
				}
			}

			Position newexit = oldexit + offset;
			if(newexit.isValid()) imported_house->setExit(&map, newexit);
			map.houses.addHouse(imported_house);

#ifdef __VISUALC__ // C++0x compliance to some degree :)
			hit = imported_map.houses.erase(hit);
#else // Bulky, slow way
			HouseMap::iterator tmp_iter = hit;
			++tmp_iter;
			uint32_t next_key = 0;
			if(tmp_iter != imported_map.houses.end()) {
				next_key = tmp_iter->first;
			}
			imported_map.houses.erase(hit);
			if(next_key != 0) {
				hit = imported_map.houses.find(next_key);
			} else {
				hit = imported_map.houses.end();
			}
#endif
		}
	}

	std::map<Position, Spawn*> spawn_map;
	if(spawn_import_type != IMPORT_DONT) {
		for(SpawnPositionList::iterator siter = imported_map.spawns.begin(); siter != imported_map.spawns.end();) {
			Position old_spawn_pos = *siter;
			Position new_spawn_pos = *siter + offset;
			switch(spawn_import_type) {
				case IMPORT_SMART_MERGE:
				case IMPORT_INSERT:
				case IMPORT_MERGE: {
					Tile* imported_tile = imported_map.getTile(old_spawn_pos);
					if(imported_tile) {
						ASSERT(imported_tile->spawn);
						spawn_map[new_spawn_pos] = imported_tile->spawn;

						SpawnPositionList::iterator next = siter;
						bool cont = true;
						Position next_spawn;

						++next;
						if(next == imported_map.spawns.end())
							cont = false;
						else
							next_spawn = *next;
						imported_map.spawns.erase(siter);
						if(cont)
							siter = imported_map.spawns.find(next_spawn);
						else
							siter = imported_map.spawns.end();
					}
					break;
				}
				case IMPORT_DONT: {
					++siter;
					break;
				}
			}
		}
	}

	// Plain merge of waypoints, very simple! :)
	for(WaypointMap::iterator iter = imported_map.waypoints.begin(); iter != imported_map.waypoints.end(); ++iter) {
		iter->second->pos += offset;
	}

	map.waypoints.waypoints.insert(imported_map.waypoints.begin(), imported_map.waypoints.end());
	imported_map.waypoints.waypoints.clear();


	uint64_t tiles_merged = 0;
	uint64_t tiles_to_import = imported_map.tilecount;
	for(MapIterator mit = imported_map.begin(); mit != imported_map.end(); ++mit) {
		if(tiles_merged % 8092 == 0) {
			SetLoadDone(int(100.0 * tiles_merged / tiles_to_import));
		}
		++tiles_merged;

		Tile* import_tile = (*mit)->get();
		Position new_pos = import_tile->getPosition() + offset;
		if(!new_pos.isValid()) {
			++discarded_tiles;
			continue;
		}

		if(!resizemap && (new_pos.x > map.getWidth() || new_pos.y > map.getHeight())) {
			if(resize_asked) {
				++discarded_tiles;
				continue;
			} else {
				resize_asked = true;
				int ret = PopupDialog("Collision", "The imported tiles are outside the current map scope. Do you want to resize the map? (Else additional tiles will be removed)", wxYES | wxNO);

				if(ret == wxID_YES) {
					// ...
					resizemap = true;
				} else {
					++discarded_tiles;
					continue;
				}
			}
		}

		if(new_pos.x > newsize_x) {
			newsize_x = new_pos.x;
		}
		if(new_pos.y > newsize_y) {
			newsize_y = new_pos.y;
		}

		imported_map.setTile(import_tile->getPosition(), nullptr);
		TileLocation* location = map.createTileL(new_pos);


		// Check if we should update any houses
		int new_houseid = house_id_map[import_tile->getHouseID()];
		House* house = map.houses.getHouse(new_houseid);
		if(import_tile->isHouseTile() && house_import_type != IMPORT_DONT && house) {
			// We need to notify houses of the tile moving
			house->removeTile(import_tile);
			import_tile->setLocation(location);
			house->addTile(import_tile);
		} else {
			import_tile->setLocation(location);
		}

		if(offset != Position(0,0,0)) {
			for(Item *item = import_tile->items; item != NULL; item = item->next){
				if(item->getFlag(TELEPORTABSOLUTE)){
					Position pos = UnpackAbsoluteCoordinate(item->getAttribute(ABSTELEPORTDESTINATION));
					item->setAttribute(ABSTELEPORTDESTINATION, PackAbsoluteCoordinate(pos + offset));
				}
			}
		}

		Tile* old_tile = map.getTile(new_pos);
		if(old_tile) {
			map.removeSpawn(old_tile);
		}
		import_tile->spawn = nullptr;

		map.setTile(new_pos, import_tile, true);
	}

	for(std::map<Position, Spawn*>::iterator spawn_iter = spawn_map.begin(); spawn_iter != spawn_map.end(); ++spawn_iter) {
		Position pos = spawn_iter->first;
		TileLocation* location = map.createTileL(pos);
		Tile* tile = location->get();
		if(!tile) {
			tile = map.allocator(location);
			map.setTile(pos, tile);
		} else if(tile->spawn) {
			map.removeSpawnInternal(tile);
			delete tile->spawn;
		}
		tile->spawn = spawn_iter->second;

		map.addSpawn(tile);
	}

	DestroyLoadBar();

	map.setWidth(newsize_x);
	map.setHeight(newsize_y);
	PopupDialog("Success", "Map imported successfully, " + i2ws(discarded_tiles) + " tiles were discarded as invalid.", wxOK);

	RefreshPalettes();
	FitViewToMap();

	return true;
}

void Editor::borderizeSelection()
{
	if(selection.empty()) {
		SetStatusText("No items selected. Can't borderize.");
		return;
	}

	Action* action = actionQueue->createAction(ACTION_BORDERIZE);
	for(const Tile* tile : selection) {
		Tile* new_tile = tile->deepCopy(map);
		new_tile->borderize(&map);
		new_tile->select();
		action->addChange(new Change(new_tile));
	}
	addAction(action);
	updateActions();
}

void Editor::borderizeMap(bool showdialog)
{
	if(showdialog) {
		CreateLoadBar("Borderizing map...");
	}

	uint64_t tiles_done = 0;
	for(TileLocation* tileLocation : map) {
		if(showdialog && tiles_done % 4096 == 0) {
			SetLoadDone(static_cast<int32_t>(tiles_done / double(map.tilecount) * 100.0));
		}

		Tile* tile = tileLocation->get();
		ASSERT(tile);

		tile->borderize(&map);
		++tiles_done;
	}

	if(showdialog) {
		DestroyLoadBar();
	}
}

void Editor::randomizeSelection()
{
	if(selection.empty()) {
		SetStatusText("No items selected. Can't randomize.");
		return;
	}

	Action* action = actionQueue->createAction(ACTION_RANDOMIZE);
	for(const Tile* tile : selection) {
		Tile* new_tile = tile->deepCopy(map);
		GroundBrush* brush = new_tile->getGroundBrush();
		if(brush && brush->isReRandomizable()) {
			brush->draw(&map, new_tile, nullptr);
			new_tile->select();
			action->addChange(new Change(new_tile));
		}
	}
	addAction(action);
	updateActions();
}

void Editor::randomizeMap(bool showdialog)
{
	if(showdialog) {
		CreateLoadBar("Randomizing map...");
	}

	uint64_t tiles_done = 0;
	for(TileLocation* tileLocation : map) {
		if(showdialog && tiles_done % 4096 == 0) {
			SetLoadDone(static_cast<int32_t>(tiles_done / double(map.tilecount) * 100.0));
		}

		Tile* tile = tileLocation->get();
		ASSERT(tile);

		if(GroundBrush* groundBrush = tile->getGroundBrush()) {
			groundBrush->draw(&map, tile, nullptr);
		}
		++tiles_done;
	}

	if(showdialog) {
		DestroyLoadBar();
	}
}

void Editor::clearInvalidHouseTiles(bool showdialog)
{
	if(showdialog) {
		CreateLoadBar("Clearing invalid house tiles...");
	}

	Houses& houses = map.houses;

	HouseMap::iterator iter = houses.begin();
	while(iter != houses.end()) {
		House* h = iter->second;
		if(map.towns.getTown(h->townid) == nullptr) {
#ifdef __VISUALC__ // C++0x compliance to some degree :)
			iter = houses.erase(iter);
#else // Bulky, slow way
			HouseMap::iterator tmp_iter = iter;
			++tmp_iter;
			uint32_t next_key = 0;
			if(tmp_iter != houses.end()) {
				next_key = tmp_iter->first;
			}
			houses.erase(iter);
			if(next_key != 0) {
				iter = houses.find(next_key);
			} else {
				iter = houses.end();
			}
#endif
		} else {
			++iter;
		}
	}

	uint64_t tiles_done = 0;
	for(MapIterator map_iter = map.begin(); map_iter != map.end(); ++map_iter) {
		if(showdialog && tiles_done % 4096 == 0) {
			SetLoadDone(int(tiles_done / double(map.tilecount) * 100.0));
		}

		Tile* tile = (*map_iter)->get();
		ASSERT(tile);
		if(tile->isHouseTile()) {
			if(houses.getHouse(tile->getHouseID()) == nullptr) {
				tile->setHouse(nullptr);
			}
		}
		++tiles_done;
	}

	if(showdialog) {
		DestroyLoadBar();
	}
}

void Editor::clearModifiedTileState(bool showdialog)
{
	if(showdialog) {
		CreateLoadBar("Clearing modified state from all tiles...");
	}

	uint64_t tiles_done = 0;
	for(MapIterator map_iter = map.begin(); map_iter != map.end(); ++map_iter) {
		if(showdialog && tiles_done % 4096 == 0) {
			SetLoadDone(int(tiles_done / double(map.tilecount) * 100.0));
		}

		Tile* tile = (*map_iter)->get();
		ASSERT(tile);
		tile->unmodify();
		++tiles_done;
	}

	if(showdialog) {
		DestroyLoadBar();
	}
}

void Editor::moveSelection(const Position& offset)
{
	if(!hasSelection()) {
		return;
	}

	bool borderize = false;
	int drag_threshold = g_settings.getInteger(Config::BORDERIZE_DRAG_THRESHOLD);
	bool create_borders = g_settings.getInteger(Config::USE_AUTOMAGIC)
		&& g_settings.getInteger(Config::BORDERIZE_DRAG);

	TileSet storage;
	BatchAction* batch_action = actionQueue->createBatch(ACTION_MOVE);
	Action* action = actionQueue->createAction(batch_action);

	// Update the tiles with the new positions
	for(Tile* tile : selection) {
		Tile* new_tile = tile->deepCopy(map);
		Tile* storage_tile = map.allocator(tile->getLocation());

		storage_tile->addItems(new_tile->popSelectedItems());

		if(new_tile->spawn && new_tile->spawn->isSelected()) {
			storage_tile->spawn = new_tile->spawn;
			new_tile->spawn = nullptr;
		}

		if(new_tile->creature && new_tile->creature->isSelected()) {
			storage_tile->creature = new_tile->creature;
			new_tile->creature = nullptr;
		}

		if(storage_tile->getFlag(BANK)){
			storage_tile->house_id = new_tile->house_id;
			storage_tile->flags = new_tile->flags;
			new_tile->house_id = 0;
			new_tile->flags = 0;
			borderize = true;
		}

		storage.insert(storage_tile);
		action->addChange(new Change(new_tile));
	}
	batch_action->addAndCommitAction(action);

	// Remove old borders (and create some new?)
	if(create_borders && selection.size() < static_cast<size_t>(drag_threshold)) {
		action = actionQueue->createAction(batch_action);
		std::vector<Tile*> borderize_tiles;
		// Go through all modified (selected) tiles (might be slow)
		for(const Tile* tile : storage) {
			const Position& pos = tile->getPosition();
			// Go through all neighbours
			Tile* t;
			t = map.getTile(pos.x  , pos.y  , pos.z); if(t && !t->isSelected()) { borderize_tiles.push_back(t); }
			t = map.getTile(pos.x-1, pos.y-1, pos.z); if(t && !t->isSelected()) { borderize_tiles.push_back(t); }
			t = map.getTile(pos.x  , pos.y-1, pos.z); if(t && !t->isSelected()) { borderize_tiles.push_back(t); }
			t = map.getTile(pos.x+1, pos.y-1, pos.z); if(t && !t->isSelected()) { borderize_tiles.push_back(t); }
			t = map.getTile(pos.x-1, pos.y  , pos.z); if(t && !t->isSelected()) { borderize_tiles.push_back(t); }
			t = map.getTile(pos.x+1, pos.y  , pos.z); if(t && !t->isSelected()) { borderize_tiles.push_back(t); }
			t = map.getTile(pos.x-1, pos.y+1, pos.z); if(t && !t->isSelected()) { borderize_tiles.push_back(t); }
			t = map.getTile(pos.x  , pos.y+1, pos.z); if(t && !t->isSelected()) { borderize_tiles.push_back(t); }
			t = map.getTile(pos.x+1, pos.y+1, pos.z); if(t && !t->isSelected()) { borderize_tiles.push_back(t); }
		}

		{ // Remove duplicates
			std::sort(borderize_tiles.begin(), borderize_tiles.end());
			auto end = std::unique(borderize_tiles.begin(), borderize_tiles.end());
			borderize_tiles.erase(end, borderize_tiles.end());
		}

		// Create borders
		for(const Tile* tile : borderize_tiles) {
			Tile* new_tile = tile->deepCopy(map);
			if(borderize) {
				new_tile->borderize(&map);
			}
			new_tile->wallize(&map);
			new_tile->tableize(&map);
			new_tile->carpetize(&map);
			if(Item *ground = new_tile->getFirstItem(BANK)){
				if(ground->isSelected()){
					new_tile->selectGround();
				}
			}
			action->addChange(new Change(new_tile));
		}
		batch_action->addAndCommitAction(action);
	}

	// New action for adding the destination tiles
	action = actionQueue->createAction(batch_action);
	for(Tile* tile : storage) {
		const Position& old_pos = tile->getPosition();
		Position new_pos = old_pos - offset;
		if(new_pos.z < rme::MapMinLayer && new_pos.z > rme::MapMaxLayer) {
			delete tile;
			continue;
		}

		TileLocation* location = map.createTileL(new_pos);
		Tile* old_dest_tile = location->get();
		Tile* new_dest_tile = nullptr;

		if(!tile->getFlag(BANK) || g_settings.getInteger(Config::MERGE_MOVE)) {
			// Move items
			if(old_dest_tile) {
				new_dest_tile = old_dest_tile->deepCopy(map);
			} else {
				new_dest_tile = map.allocator(location);
			}
			new_dest_tile->merge(tile);
			delete tile;
		} else {
			// Replace tile instead of just merge
			tile->setLocation(location);
			new_dest_tile = tile;
		}
		action->addChange(new Change(new_dest_tile));
	}
	batch_action->addAndCommitAction(action);

	if(create_borders && selection.size() < static_cast<size_t>(drag_threshold)) {
		action = actionQueue->createAction(batch_action);
		std::vector<Tile*> borderize_tiles;
		// Go through all modified (selected) tiles (might be slow)
		for(Tile* tile : selection) {
			bool add_me = false; // If this tile is touched
			const Position& pos = tile->getPosition();
			// Go through all neighbours
			Tile* t;
			t = map.getTile(pos.x-1, pos.y-1, pos.z); if(t && !t->isSelected()) { borderize_tiles.push_back(t); add_me = true; }
			t = map.getTile(pos.x-1, pos.y-1, pos.z); if(t && !t->isSelected()) { borderize_tiles.push_back(t); add_me = true; }
			t = map.getTile(pos.x  , pos.y-1, pos.z); if(t && !t->isSelected()) { borderize_tiles.push_back(t); add_me = true; }
			t = map.getTile(pos.x+1, pos.y-1, pos.z); if(t && !t->isSelected()) { borderize_tiles.push_back(t); add_me = true; }
			t = map.getTile(pos.x-1, pos.y  , pos.z); if(t && !t->isSelected()) { borderize_tiles.push_back(t); add_me = true; }
			t = map.getTile(pos.x+1, pos.y  , pos.z); if(t && !t->isSelected()) { borderize_tiles.push_back(t); add_me = true; }
			t = map.getTile(pos.x-1, pos.y+1, pos.z); if(t && !t->isSelected()) { borderize_tiles.push_back(t); add_me = true; }
			t = map.getTile(pos.x  , pos.y+1, pos.z); if(t && !t->isSelected()) { borderize_tiles.push_back(t); add_me = true; }
			t = map.getTile(pos.x+1, pos.y+1, pos.z); if(t && !t->isSelected()) { borderize_tiles.push_back(t); add_me = true; }
			if(add_me) {
				borderize_tiles.push_back(tile);
			}
		}

		{ // Remove duplicates
			std::sort(borderize_tiles.begin(), borderize_tiles.end());
			auto end = std::unique(borderize_tiles.begin(), borderize_tiles.end());
			borderize_tiles.erase(end, borderize_tiles.end());
		}

		// Create borders
		for(const Tile* tile : borderize_tiles) {
			if(!tile || !tile->getFlag(BANK)){
				continue;
			}
			if(tile->getGroundBrush()) {
				Tile* new_tile = tile->deepCopy(map);
				if(borderize) {
					new_tile->borderize(&map);
				}
				new_tile->wallize(&map);
				new_tile->tableize(&map);
				new_tile->carpetize(&map);
				if(Item *ground = new_tile->getFirstItem(BANK)){
					if(ground->isSelected()){
						new_tile->selectGround();
					}
				}
				action->addChange(new Change(new_tile));
			}
		}
		batch_action->addAndCommitAction(action);
	}

	// Store the action for undo
	addBatch(batch_action);
	updateActions();
	selection.updateSelectionCount();
}

void Editor::destroySelection()
{
	if(selection.size() == 0) {
		SetStatusText("No selected items to delete.");
	} else {
		int tile_count = 0;
		int item_count = 0;
		PositionList tilestoborder;

		BatchAction* batch = actionQueue->createBatch(ACTION_DELETE_TILES);
		Action* action = actionQueue->createAction(batch);

		for(TileSet::iterator it = selection.begin(); it != selection.end(); ++it) {
			tile_count++;

			Tile* tile = *it;
			Tile* newtile = tile->deepCopy(map);


			newtile->removeItems([](const Item *item){ return item->isSelected(); });

			if(newtile->creature && newtile->creature->isSelected()) {
				delete newtile->creature;
				newtile->creature = nullptr;
			}

			if(newtile->spawn && newtile->spawn->isSelected()) {
				delete newtile->spawn;
				newtile->spawn = nullptr;
			}

			if(g_settings.getInteger(Config::USE_AUTOMAGIC)) {
				for(int y = -1; y <= 1; y++) {
					for(int x = -1; x <= 1; x++) {
						const Position& position = tile->getPosition();
						tilestoborder.push_back(Position(position.x + x, position.y + y, position.z));
					}
				}
			}
			action->addChange(newd Change(newtile));
		}

		batch->addAndCommitAction(action);

		if(g_settings.getInteger(Config::USE_AUTOMAGIC)) {
			// Remove duplicates
			tilestoborder.sort();
			tilestoborder.unique();

			action = actionQueue->createAction(batch);
			for(PositionList::iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
				TileLocation* location = map.createTileL(*it);
				Tile* tile = location->get();

				if(tile) {
					Tile* new_tile = tile->deepCopy(map);
					new_tile->borderize(&map);
					new_tile->wallize(&map);
					new_tile->tableize(&map);
					new_tile->carpetize(&map);
					action->addChange(newd Change(new_tile));
				} else {
					Tile* new_tile = map.allocator(location);
					new_tile->borderize(&map);
					if(new_tile->size()) {
						action->addChange(newd Change(new_tile));
					} else {
						delete new_tile;
					}
				}
			}

			batch->addAndCommitAction(action);
		}

		addBatch(batch);
		updateActions();

		wxString ss;
		ss << "Deleted " << tile_count << " tile" << (tile_count > 1 ? "s" : "") <<  " (" << item_count << " item" << (item_count > 1? "s" : "") << ")";
		SetStatusText(ss);
	}
}

	// Macro to avoid useless code repetition
void doSurroundingBorders(DoodadBrush* doodad_brush, PositionList& tilestoborder, Tile* buffer_tile, Tile* new_tile)
{
	if(doodad_brush->doNewBorders() && g_settings.getInteger(Config::USE_AUTOMAGIC)) {
		const Position& position = new_tile->getPosition();
		tilestoborder.push_back(Position(position));
		if(buffer_tile->getFlag(BANK)) {
			for(int y = -1; y <= 1; y++) {
				for(int x = -1; x <= 1; x++) {
					tilestoborder.push_back(Position(position.x + x, position.y + y, position.z));
				}
			}
		} else if(buffer_tile->getWall()) {
			tilestoborder.push_back(Position(position.x,   position.y-1, position.z));
			tilestoborder.push_back(Position(position.x-1, position.y,   position.z));
			tilestoborder.push_back(Position(position.x+1, position.y,   position.z));
			tilestoborder.push_back(Position(position.x,   position.y+1, position.z));
		}
	}
}

void removeDuplicateWalls(Tile* buffer, Tile* tile)
{
	if (!buffer || !buffer->items || !tile || !tile->items) {
		return;
	}

	for(const Item *item = buffer->items; item != NULL; item = item->next){
		if(WallBrush *brush = item->getWallBrush()){
			tile->removeWalls(brush);
		}
	}
}

void Editor::drawInternal(Position offset, bool alt, bool dodraw)
{
	Brush* brush = GetCurrentBrush();
	if(!brush) {
		return;
	}

	if(brush->isDoodad()) {
		BatchAction* batch = actionQueue->createBatch(ACTION_DRAW);
		Action* action = actionQueue->createAction(batch);
		BaseMap* buffer_map = doodad_buffer_map;

		Position delta_pos = offset - Position(0x8000, 0x8000, 0x8);
		PositionList tilestoborder;

		for(MapIterator it = buffer_map->begin(); it != buffer_map->end(); ++it) {
			Tile* buffer_tile = (*it)->get();
			Position pos = buffer_tile->getPosition() + delta_pos;
			if(!pos.isValid()) {
				continue;
			}

			TileLocation* location = map.createTileL(pos);
			Tile* tile = location->get();
			DoodadBrush* doodad_brush = brush->asDoodad();

			if(doodad_brush->placeOnBlocking() || alt) {
				if(tile) {
					bool place = true;
					if(!doodad_brush->placeOnDuplicate() && !alt) {
						for(Item *item = tile->items; item != NULL; item = item->next){
							if(doodad_brush->ownsItem(item)) {
								place = false;
								break;
							}
						}
					}
					if(place) {
						Tile* new_tile = tile->deepCopy(map);
						removeDuplicateWalls(buffer_tile, new_tile);
						doSurroundingBorders(doodad_brush, tilestoborder, buffer_tile, new_tile);
						new_tile->merge(buffer_tile);
						action->addChange(newd Change(new_tile));
					}
				} else {
					Tile* new_tile = map.allocator(location);
					removeDuplicateWalls(buffer_tile, new_tile);
					doSurroundingBorders(doodad_brush, tilestoborder, buffer_tile, new_tile);
					new_tile->merge(buffer_tile);
					action->addChange(newd Change(new_tile));
				}
			} else {
				if(tile && !tile->getFlag(UNPASS)) {
					bool place = true;
					if(!doodad_brush->placeOnDuplicate() && !alt) {
						for(Item *item = tile->items; item != NULL; item = item->next){
							if(doodad_brush->ownsItem(item)) {
								place = false;
								break;
							}
						}
					}
					if(place) {
						Tile* new_tile = tile->deepCopy(map);
						removeDuplicateWalls(buffer_tile, new_tile);
						doSurroundingBorders(doodad_brush, tilestoborder, buffer_tile, new_tile);
						new_tile->merge(buffer_tile);
						action->addChange(newd Change(new_tile));
					}
				}
			}
		}
		batch->addAndCommitAction(action);

		if(tilestoborder.size() > 0) {
			Action* action = actionQueue->createAction(batch);

			// Remove duplicates
			tilestoborder.sort();
			tilestoborder.unique();

			for(PositionList::const_iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
				Tile* tile = map.getTile(*it);
				if(tile) {
					Tile* new_tile = tile->deepCopy(map);
					new_tile->borderize(&map);
					new_tile->wallize(&map);
					action->addChange(newd Change(new_tile));
				}
			}
			batch->addAndCommitAction(action);
		}
		addBatch(batch, 2);
	} else if(brush->isHouseExit()) {
		HouseExitBrush* house_exit_brush = brush->asHouseExit();
		if(!house_exit_brush->canDraw(&map, offset))
			return;

		House* house = map.houses.getHouse(house_exit_brush->getHouseID());
		if(!house)
			return;

		BatchAction* batch = actionQueue->createBatch(ACTION_DRAW);
		Action* action = actionQueue->createAction(batch);
		action->addChange(Change::Create(house, offset));
		batch->addAndCommitAction(action);
		addBatch(batch, 2);
	} else if(brush->isWaypoint()) {
		WaypointBrush* waypoint_brush = brush->asWaypoint();
		if(!waypoint_brush->canDraw(&map, offset))
			return;

		Waypoint* waypoint = map.waypoints.getWaypoint(waypoint_brush->getWaypoint());
		if(!waypoint || waypoint->pos == offset)
			return;

		BatchAction* batch = actionQueue->createBatch(ACTION_DRAW);
		Action* action = actionQueue->createAction(batch);
		action->addChange(Change::Create(waypoint, offset));
		batch->addAndCommitAction(action);
		addBatch(batch, 2);
	} else if(brush->isWall()) {
		BatchAction* batch = actionQueue->createBatch(dodraw ? ACTION_DRAW : ACTION_ERASE);
		Action* action = actionQueue->createAction(batch);
		// This will only occur with a size 0, when clicking on a tile (not drawing)
		Tile* tile = map.getTile(offset);
		Tile* new_tile = nullptr;
		if(tile) {
			new_tile = tile->deepCopy(map);
		} else {
			new_tile = map.allocator(map.createTileL(offset));
		}

		if(dodraw) {
			bool b = true;
			brush->asWall()->draw(&map, new_tile, &b);
		} else {
			brush->asWall()->undraw(&map, new_tile);
		}
		action->addChange(newd Change(new_tile));
		batch->addAndCommitAction(action);
		addBatch(batch, 2);
	} else if(brush->isSpawn() || brush->isCreature()) {
		BatchAction* batch = actionQueue->createBatch(dodraw ? ACTION_DRAW : ACTION_ERASE);
		Action* action = actionQueue->createAction(batch);

		Tile* tile = map.getTile(offset);
		Tile* new_tile = nullptr;
		if(tile) {
			new_tile = tile->deepCopy(map);
		} else {
			new_tile = map.allocator(map.createTileL(offset));
		}
		int param;
		if(!brush->isCreature()) {
			param = GetBrushSize();
		}
		if(dodraw) {
			brush->draw(&map, new_tile, &param);
		} else {
			brush->undraw(&map, new_tile);
		}
		action->addChange(newd Change(new_tile));
		batch->addAndCommitAction(action);
		addBatch(batch, 2);
	}
}

void Editor::drawInternal(const PositionVector& tilestodraw, bool alt, bool dodraw)
{
	Brush* brush = GetCurrentBrush();
	if(!brush) {
		return;
	}

#ifdef __DEBUG__
	if(brush->isGround() || brush->isWall()) {
		// Wrong function, end call
		return;
	}
#endif

	Action* action = actionQueue->createAction(dodraw ? ACTION_DRAW : ACTION_ERASE);

	if(brush->isOptionalBorder()) {
		// We actually need to do borders, but on the same tiles we draw to
		for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			TileLocation* location = map.createTileL(*it);
			Tile* tile = location->get();
			if(tile) {
				if(dodraw) {
					Tile* new_tile = tile->deepCopy(map);
					brush->draw(&map, new_tile);
					new_tile->borderize(&map);
					action->addChange(newd Change(new_tile));
				} else if(!dodraw && tile->hasOptionalBorder()) {
					Tile* new_tile = tile->deepCopy(map);
					brush->undraw(&map, new_tile);
					new_tile->borderize(&map);
					action->addChange(newd Change(new_tile));
				}
			} else if(dodraw) {
				Tile* new_tile = map.allocator(location);
				brush->draw(&map, new_tile);
				new_tile->borderize(&map);
				if(new_tile->size() == 0) {
					delete new_tile;
					continue;
				}
				action->addChange(newd Change(new_tile));
			}
		}
	} else {

		for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			TileLocation* location = map.createTileL(*it);
			Tile* tile = location->get();
			if(tile) {
				Tile* new_tile = tile->deepCopy(map);
				if(dodraw) {
					brush->draw(&map, new_tile, &alt);
				} else {
					brush->undraw(&map, new_tile);
				}
				action->addChange(newd Change(new_tile));
			} else if(dodraw) {
				Tile* new_tile = map.allocator(location);
				brush->draw(&map, new_tile, &alt);
				action->addChange(newd Change(new_tile));
			}
		}
	}
	addAction(action, 2);
}

void Editor::drawInternal(const PositionVector& tilestodraw, PositionVector& tilestoborder, bool alt, bool dodraw)
{
	Brush* brush = GetCurrentBrush();
	if(!brush) {
		return;
	}

	if(brush->isGround() || brush->isEraser()) {
		ActionIdentifier identifier = (dodraw && !brush->isEraser()) ? ACTION_DRAW : ACTION_ERASE;
		BatchAction* batch = actionQueue->createBatch(identifier);
		Action* action = actionQueue->createAction(batch);

		for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			TileLocation* location = map.createTileL(*it);
			Tile* tile = location->get();
			if(tile) {
				Tile* new_tile = tile->deepCopy(map);
				if(g_settings.getInteger(Config::USE_AUTOMAGIC)) {
					new_tile->removeBorders();
				}
				if (dodraw) {
					GetCurrentBrush()->draw(&map, new_tile, nullptr);
				} else {
					GetCurrentBrush()->undraw(&map, new_tile);
					tilestoborder.push_back(*it);
				}
				action->addChange(newd Change(new_tile));
			} else if(dodraw) {
				Tile* new_tile = map.allocator(location);
				GetCurrentBrush()->draw(&map, new_tile, nullptr);
				action->addChange(newd Change(new_tile));
			}
		}

		// Commit changes to map
		batch->addAndCommitAction(action);

		if(g_settings.getInteger(Config::USE_AUTOMAGIC)) {
			// Do borders!
			action = actionQueue->createAction(batch);
			for(PositionVector::const_iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
				TileLocation* location = map.createTileL(*it);
				Tile* tile = location->get();
				if(tile) {
					Tile* new_tile = tile->deepCopy(map);
					if(brush->isEraser()) {
						new_tile->wallize(&map);
						new_tile->tableize(&map);
						new_tile->carpetize(&map);
					}
					new_tile->borderize(&map);
					action->addChange(newd Change(new_tile));
				} else {
					Tile* new_tile = map.allocator(location);
					if(brush->isEraser()) {
						// There are no carpets/tables/walls on empty tiles...
						//new_tile->wallize(map);
						//new_tile->tableize(map);
						//new_tile->carpetize(map);
					}
					new_tile->borderize(&map);
					if(new_tile->size() > 0) {
						action->addChange(newd Change(new_tile));
					} else {
						delete new_tile;
					}
				}
			}
			batch->addAndCommitAction(action);
		}

		addBatch(batch, 2);
	} else if(brush->isTable() || brush->isCarpet()) {
		BatchAction* batch = actionQueue->createBatch(ACTION_DRAW);
		Action* action = actionQueue->createAction(batch);

		for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			TileLocation* location = map.createTileL(*it);
			Tile* tile = location->get();
			if(tile) {
				Tile* new_tile = tile->deepCopy(map);
				if(dodraw)
					GetCurrentBrush()->draw(&map, new_tile, nullptr);
				else
					GetCurrentBrush()->undraw(&map, new_tile);
				action->addChange(newd Change(new_tile));
			} else if(dodraw) {
				Tile* new_tile = map.allocator(location);
				GetCurrentBrush()->draw(&map, new_tile, nullptr);
				action->addChange(newd Change(new_tile));
			}
		}

		// Commit changes to map
		batch->addAndCommitAction(action);

		// Do borders!
		action = actionQueue->createAction(batch);
		for(PositionVector::const_iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
			Tile* tile = map.getTile(*it);
			if(brush->isTable()) {
				if(tile && tile->getTable()) {
					Tile* new_tile = tile->deepCopy(map);
					new_tile->tableize(&map);
					action->addChange(newd Change(new_tile));
				}
			} else if(brush->isCarpet()) {
				if(tile && tile->getCarpet()) {
					Tile* new_tile = tile->deepCopy(map);
					new_tile->carpetize(&map);
					action->addChange(newd Change(new_tile));
				}
			}
		}
		batch->addAndCommitAction(action);

		addBatch(batch, 2);
	} else if(brush->isWall()) {
		BatchAction* batch = actionQueue->createBatch(ACTION_DRAW);
		Action* action = actionQueue->createAction(batch);

		if(alt && dodraw) {
			// This is exempt from USE_AUTOMAGIC
			doodad_buffer_map->clear();
			BaseMap* draw_map = doodad_buffer_map;

			for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
				TileLocation* location = map.createTileL(*it);
				Tile* tile = location->get();
				if(tile) {
					Tile* new_tile = tile->deepCopy(map);
					new_tile->removeWalls(brush->isWall());
					GetCurrentBrush()->draw(draw_map, new_tile);
					draw_map->setTile(*it, new_tile, true);
				} else if(dodraw) {
					Tile* new_tile = map.allocator(location);
					GetCurrentBrush()->draw(draw_map, new_tile);
					draw_map->setTile(*it, new_tile, true);
				}
			}
			for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
				// Get the correct tiles from the draw map instead of the editor map
				Tile* tile = draw_map->getTile(*it);
				if(tile) {
					tile->wallize(draw_map);
					action->addChange(newd Change(tile));
				}
			}
			draw_map->clear(false);
			// Commit
			batch->addAndCommitAction(action);
		} else {
			for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
				TileLocation* location = map.createTileL(*it);
				Tile* tile = location->get();
				if(tile) {
					Tile* new_tile = tile->deepCopy(map);
					// Wall cleaning is exempt from automagic
					new_tile->removeWalls(brush->isWall());
					if(dodraw)
						GetCurrentBrush()->draw(&map, new_tile);
					else
						GetCurrentBrush()->undraw(&map, new_tile);
					action->addChange(newd Change(new_tile));
				} else if(dodraw) {
					Tile* new_tile = map.allocator(location);
					GetCurrentBrush()->draw(&map, new_tile);
					action->addChange(newd Change(new_tile));
				}
			}

			// Commit changes to map
			batch->addAndCommitAction(action);

			if(g_settings.getInteger(Config::USE_AUTOMAGIC)) {
				// Do borders!
				action = actionQueue->createAction(batch);
				for(PositionVector::const_iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
					Tile* tile = map.getTile(*it);
					if(tile) {
						Tile* new_tile = tile->deepCopy(map);
						new_tile->wallize(&map);
						//if(*tile == *new_tile) delete new_tile;
						action->addChange(newd Change(new_tile));
					}
				}
				batch->addAndCommitAction(action);
			}
		}

		actionQueue->addBatch(batch, 2);
	} else if(brush->isDoor()) {
		BatchAction* batch = actionQueue->createBatch(ACTION_DRAW);
		Action* action = actionQueue->createAction(batch);
		DoorBrush* door_brush = brush->asDoor();

		// Loop is kind of redundant since there will only ever be one index.
		for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			TileLocation* location = map.createTileL(*it);
			Tile* tile = location->get();
			if(tile) {
				Tile* new_tile = tile->deepCopy(map);
				// Wall cleaning is exempt from automagic
				if(brush->isWall())
					new_tile->removeWalls(brush->asWall());
				if(dodraw)
					door_brush->draw(&map, new_tile, &alt);
				else
					door_brush->undraw(&map, new_tile);
				action->addChange(newd Change(new_tile));
			} else if(dodraw) {
				Tile* new_tile = map.allocator(location);
				door_brush->draw(&map, new_tile, &alt);
				action->addChange(newd Change(new_tile));
			}
		}

		// Commit changes to map
		batch->addAndCommitAction(action);

		if(g_settings.getInteger(Config::USE_AUTOMAGIC)) {
			// Do borders!
			action = actionQueue->createAction(batch);
			for(PositionVector::const_iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
				Tile* tile = map.getTile(*it);
				if(tile) {
					Tile* new_tile = tile->deepCopy(map);
					new_tile->wallize(&map);
					//if(*tile == *new_tile) delete new_tile;
					action->addChange(newd Change(new_tile));
				}
			}
			batch->addAndCommitAction(action);
		}

		addBatch(batch, 2);
	} else {
		Action* action = actionQueue->createAction(ACTION_DRAW);
		for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			TileLocation* location = map.createTileL(*it);
			Tile* tile = location->get();
			if(tile) {
				Tile* new_tile = tile->deepCopy(map);
				if(dodraw)
					GetCurrentBrush()->draw(&map, new_tile);
				else
					GetCurrentBrush()->undraw(&map, new_tile);
				action->addChange(newd Change(new_tile));
			} else if(dodraw) {
				Tile* new_tile = map.allocator(location);
				GetCurrentBrush()->draw(&map, new_tile);
				action->addChange(newd Change(new_tile));
			}
		}
		addAction(action, 2);
	}
}

std::ostream& operator<<(std::ostream& os, const Hotkey& hotkey)
{
	switch(hotkey.type) {
		case Hotkey::POSITION: {
			os << "pos:{" << hotkey.pos << "}";
		} break;
		case Hotkey::BRUSH: {
			if(hotkey.brushname.find('{') != std::string::npos ||
					hotkey.brushname.find('}') != std::string::npos) {
				break;
			}
			os << "brush:{" << hotkey.brushname << "}";
		} break;
		default: {
			os << "none:{}";
		} break;
	}
	return os;
}

std::istream& operator>>(std::istream& is, Hotkey& hotkey)
{
	std::string type;
	getline(is, type, ':');
	if(type == "none") {
		is.ignore(2); // ignore "{}"
	} else if(type == "pos") {
		is.ignore(1); // ignore "{"
		Position pos;
		is >> pos;
		hotkey = Hotkey(pos);
		is.ignore(1); // ignore "}"
	} else if(type == "brush") {
		is.ignore(1); // ignore "{"
		std::string brushname;
		getline(is, brushname, '}');
		hotkey = Hotkey(brushname);
	} else {
		// Do nothing...
	}

	return is;
}

