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

#ifndef RME_APPLICATION_H_
#define RME_APPLICATION_H_

#include "editor.h"

class wxEventLoopBase;

wxDECLARE_EVENT(EVT_UPDATE_MENUS, wxCommandEvent);
wxDECLARE_EVENT(EVT_UPDATE_ACTIONS, wxCommandEvent);

class Application : public wxApp
{
public:
	~Application() override;
	bool OnInit() override;
	int OnExit() override;
	void OnFatalException() override;
	void OnEventLoopEnter(wxEventLoopBase *loop) override;
#ifdef __WXMAC__
	void MacOpenFiles(const wxArrayString &fileNames) override;
#endif

	void FixVersionDiscrapencies();
	void Unload();

private:
    wxString file_to_open;
};

class MainFrame : public wxFrame
{
public:
	MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

	~MainFrame() override;
	void PrepareDC(wxDC& dc) override;
#ifdef __WXMSW__
	bool MSWTranslateMessage(WXMSG *msg) override;
#endif

	void OnIdle(wxIdleEvent& event);
	void OnExit(wxCloseEvent& event);
	void OnUpdateMenus(wxCommandEvent& event);
	void OnUpdateActions(wxCommandEvent& event);
	void UpdateFloorMenu();
	void UpdateIndicatorsMenu();

	DECLARE_EVENT_TABLE()
};

#endif
