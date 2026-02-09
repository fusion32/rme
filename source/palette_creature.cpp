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
#include "wxids.h"

#include "settings.h"
#include "brush.h"
#include "editor.h"
#include "palette_creature.h"
#include "creature_brush.h"
#include "materials.h"

// ============================================================================
// Creature palette

BEGIN_EVENT_TABLE(CreaturePalettePanel, PalettePanel)
	EVT_CHOICE(PALETTE_CREATURE_TILESET_CHOICE, CreaturePalettePanel::OnTilesetChange)
	EVT_LISTBOX(PALETTE_CREATURE_LISTBOX, CreaturePalettePanel::OnListBoxChange)
	EVT_SPINCTRL(PALETTE_CREATURE_SPAWN_RADIUS, CreaturePalettePanel::OnChangeSpawnRadius)
	EVT_SPINCTRL(PALETTE_CREATURE_SPAWN_AMOUNT, CreaturePalettePanel::OnChangeSpawnAmount)
	EVT_SPINCTRL(PALETTE_CREATURE_SPAWN_INTERVAL, CreaturePalettePanel::OnChangeSpawnInterval)
END_EVENT_TABLE()

CreaturePalettePanel::CreaturePalettePanel(wxWindow* parent, wxWindowID id) :
	PalettePanel(parent, id),
	handling_event(false)
{
	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);

	{
		wxSizer *sidesizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Creatures");
		tileset_choice = newd wxChoice(this, PALETTE_CREATURE_TILESET_CHOICE, wxDefaultPosition, wxDefaultSize, (int)0, (const wxString*)nullptr);
		sidesizer->Add(tileset_choice, 0, wxEXPAND);

		creature_list = newd SortableListBox(this, PALETTE_CREATURE_LISTBOX);
		sidesizer->Add(creature_list, 1, wxEXPAND);
		topsizer->Add(sidesizer, 1, wxEXPAND);
	}

	{
		int spawnRadius = g_settings.getInteger(Config::SPAWN_RADIUS);
		int spawnAmount = g_settings.getInteger(Config::SPAWN_AMOUNT);
		int spawnInterval = g_settings.getInteger(Config::SPAWN_INTERVAL);

		wxSizer *sidesizer = newd wxStaticBoxSizer(newd wxStaticBox(this, wxID_ANY, "Spawn", wxDefaultPosition, wxSize(150, 200)), wxVERTICAL);

		wxFlexGridSizer* grid = newd wxFlexGridSizer(2, 10, 10);
		grid->AddGrowableCol(1);

		grid->Add(newd wxStaticText(this, wxID_ANY, "Radius"), 0, wxALIGN_CENTER);
		spawn_radius_spin = newd wxSpinCtrl(this, PALETTE_CREATURE_SPAWN_RADIUS, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 50, spawnRadius);
		grid->Add(spawn_radius_spin, 0, wxEXPAND);

		grid->Add(newd wxStaticText(this, wxID_ANY, "Amount"), 0, wxALIGN_CENTER);
		spawn_amount_spin = newd wxSpinCtrl(this, PALETTE_CREATURE_SPAWN_AMOUNT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 20, spawnAmount);
		grid->Add(spawn_amount_spin, 0, wxEXPAND);

		grid->Add(newd wxStaticText(this, wxID_ANY, "Interval"), 0, wxALIGN_CENTER);
		spawn_interval_spin = newd wxSpinCtrl(this, PALETTE_CREATURE_SPAWN_INTERVAL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 3600, spawnInterval);
		grid->Add(spawn_interval_spin, 0, wxEXPAND);

		sidesizer->Add(grid, 0, wxEXPAND);
		topsizer->Add(sidesizer, 0, wxEXPAND);
	}

	SetSizerAndFit(topsizer);
	OnUpdate();
}

CreaturePalettePanel::~CreaturePalettePanel()
{
	////
}

PaletteType CreaturePalettePanel::GetType() const
{
	return TILESET_CREATURE;
}

void CreaturePalettePanel::SelectFirstBrush()
{
	SelectTileset(0);
}

Brush* CreaturePalettePanel::GetSelectedBrush() const
{
	Brush *brush = NULL;
	if(creature_list->GetCount() > 0){
		brush = (Brush*)creature_list->GetClientData(creature_list->GetSelection());
		if(brush && brush->isCreature()) {
			return brush;
		}
	}
	return brush;
}

bool CreaturePalettePanel::SelectBrush(const Brush *brush)
{
	if(brush && brush->isCreature()){
		int current_index = tileset_choice->GetSelection();
		if(current_index != wxNOT_FOUND) {
			const TilesetCategory *category = (const TilesetCategory*)tileset_choice->GetClientData(current_index);
			for(const Brush *other: category->brushlist){
				if(brush == other) {
					SelectCreature(brush->getName());
					return true;
				}
			}
		}

		// Not in the current display, search the hidden one's
		for(int index = 0; index < (int)tileset_choice->GetCount(); index += 1) {
			if(current_index == index){
				continue;
			}

			const TilesetCategory *category = (const TilesetCategory*)tileset_choice->GetClientData(index);
			for(const Brush *other: category->brushlist){
				if(brush == other){
					SelectTileset(index);
					SelectCreature(brush->getName());
					return true;
				}
			}
		}
	}
	return false;
}

int CreaturePalettePanel::GetSelectedBrushSize() const
{
	return spawn_radius_spin->GetValue();
}

void CreaturePalettePanel::OnUpdate()
{
	tileset_choice->Clear();
	g_materials.createOtherTileset();

	for(TilesetContainer::const_iterator iter = g_materials.tilesets.begin(); iter != g_materials.tilesets.end(); ++iter) {
		const TilesetCategory* tsc = iter->second->getCategory(TILESET_CREATURE);
		if(tsc && tsc->size() > 0) {
			tileset_choice->Append(wxstr(iter->second->name), const_cast<TilesetCategory*>(tsc));
		} else if(iter->second->name == "NPCs" || iter->second->name == "Others") {
			Tileset* ts = const_cast<Tileset*>(iter->second);
			TilesetCategory* rtsc = ts->getCategory(TILESET_CREATURE);
			tileset_choice->Append(wxstr(ts->name), rtsc);
		}
	}
	SelectTileset(0);
}

void CreaturePalettePanel::OnUpdateBrushSize(BrushShape shape, int size)
{
	// no-op
}

void CreaturePalettePanel::OnSwitchIn()
{
	g_editor.ActivatePalette(GetParentPalette());
	g_editor.SetBrushSize(spawn_radius_spin->GetValue());
}

void CreaturePalettePanel::SelectTileset(size_t index)
{
	ASSERT(tileset_choice->GetCount() >= index);

	creature_list->Clear();
	if(tileset_choice->GetCount() > 0) {
		const TilesetCategory *category = (const TilesetCategory*)tileset_choice->GetClientData(index);
		for(Brush *brush: category->brushlist){
			creature_list->Append(wxstr(brush->getName()), brush);
		}
		creature_list->Sort();
		SelectCreature(0);
		tileset_choice->SetSelection(index);
	}
}

void CreaturePalettePanel::SelectCreature(size_t index)
{
	ASSERT(creature_list->GetCount() >= index);
	if(creature_list->GetCount() > 0) {
		creature_list->SetSelection(index);
	}
}

void CreaturePalettePanel::SelectCreature(std::string name)
{
	if(creature_list->GetCount() > 0) {
		if(!creature_list->SetStringSelection(wxstr(name))) {
			creature_list->SetSelection(0);
		}
	}
}

void CreaturePalettePanel::OnTilesetChange(wxCommandEvent& event)
{
	SelectTileset(event.GetSelection());
	g_editor.ActivatePalette(GetParentPalette());
	g_editor.SelectBrush();
}

void CreaturePalettePanel::OnListBoxChange(wxCommandEvent& event)
{
	SelectCreature(event.GetSelection());
	g_editor.ActivatePalette(GetParentPalette());
	g_editor.SelectBrush();
}

void CreaturePalettePanel::OnChangeSpawnRadius(wxSpinEvent& event)
{
	g_editor.ActivatePalette(GetParentPalette());
	g_editor.SetSpawnRadius(event.GetPosition());
	g_settings.setInteger(Config::SPAWN_RADIUS, event.GetPosition());
}

void CreaturePalettePanel::OnChangeSpawnAmount(wxSpinEvent& event)
{
	g_editor.ActivatePalette(GetParentPalette());
	g_editor.SetSpawnAmount(event.GetPosition());
	g_settings.setInteger(Config::SPAWN_AMOUNT, event.GetPosition());
}

void CreaturePalettePanel::OnChangeSpawnInterval(wxSpinEvent& event)
{
	g_editor.ActivatePalette(GetParentPalette());
	g_editor.SetSpawnInterval(event.GetPosition());
	g_settings.setInteger(Config::SPAWN_INTERVAL, event.GetPosition());
}

