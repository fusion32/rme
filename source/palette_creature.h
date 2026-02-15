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


#ifndef RME_TILESET_CREATURE_H_
#define RME_TILESET_CREATURE_H_

#include "palette_common.h"

class CreaturePalettePanel : public PalettePanel {
public:
	CreaturePalettePanel(wxWindow* parent, wxWindowID id = wxID_ANY);
	~CreaturePalettePanel() override;

	PaletteType GetType() const override;

	// Select the first brush
	void SelectFirstBrush() override;
	// Returns the currently selected brush (first brush if panel is not loaded)
	Brush* GetSelectedBrush() const override;
	// Returns the currently selected brush size
	int GetSelectedBrushSize() const override;
	// Select the brush in the parameter, this only changes the look of the panel
	bool SelectBrush(const Brush* whatbrush) override;

	// Updates the palette window to use the current brush size
	void OnUpdateBrushSize(BrushShape shape, int size) override;
	// Called when this page is displayed
	void OnSwitchIn() override;
	// Called sometimes?
	void OnUpdate() override;

protected:
	void SelectTileset(size_t index);
	void SelectCreature(size_t index);
	void SelectCreature(std::string name);

public:
	// Event handling
	void OnTilesetChange(wxCommandEvent& event);
	void OnListBoxChange(wxCommandEvent& event);
	void OnChangeSpawnRadius(wxSpinEvent& event);
	void OnChangeSpawnAmount(wxSpinEvent& event);
	void OnChangeSpawnInterval(wxSpinEvent& event);

protected:
	wxChoice* tileset_choice;
	SortableListBox* creature_list;
	wxSpinCtrl* spawn_radius_spin;
	wxSpinCtrl* spawn_amount_spin;
	wxSpinCtrl* spawn_interval_spin;

	bool handling_event;

	DECLARE_EVENT_TABLE()
};

#endif
