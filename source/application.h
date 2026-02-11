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

#ifndef RME_APPLICATION_H_
#define RME_APPLICATION_H_

#include "editor.h"
#include "main_toolbar.h"
#include "action.h"
#include "settings.h"

#include "map_display.h"
#include "welcome_dialog.h"

class MainFrame;
class MapWindow;
class wxEventLoopBase;
class wxSingleInstanceChecker;

class Application : public wxApp
{
public:
	~Application();
	virtual bool OnInit();
    virtual void OnEventLoopEnter(wxEventLoopBase* loop);
	virtual void MacOpenFiles(const wxArrayString& fileNames);
	virtual int OnExit();
	void Unload();

private:
    bool m_startup;
    wxString m_file_to_open;
	void FixVersionDiscrapencies();
	virtual void OnFatalException();
};

class MainMenuBar;

class MainFrame : public wxFrame
{
public:
	MainFrame(const wxString& title,
		const wxPoint& pos, const wxSize& size);
	~MainFrame();

	void OnUpdateMenus(wxCommandEvent& event);
	void OnUpdateActions(wxCommandEvent& event);
	void UpdateFloorMenu();
	void UpdateIndicatorsMenu();
	void OnIdle(wxIdleEvent& event);
	void OnExit(wxCloseEvent& event);

#ifdef __WINDOWS__
	virtual bool MSWTranslateMessage(WXMSG *msg);
#endif

	void PrepareDC(wxDC& dc);
protected:

	friend class Application;
	friend class Editor;

	DECLARE_EVENT_TABLE()
};

#endif
