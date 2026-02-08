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

#include "application.h"
#include "sprites.h"
#include "editor.h"
#include "map_window.h"
#include "common_windows.h"
#include "palette_window.h"
#include "preferences.h"
#include "result_window.h"
#include "minimap_window.h"
#include "about_window.h"
#include "main_menubar.h"
#include "updater.h"
#include "artprovider.h"

#include "materials.h"
#include "map.h"
#include "creature.h"

#include <wx/snglinst.h>

#if defined(__LINUX__) || defined(__WINDOWS__)
#include <GL/glut.h>
#endif

#include "../brushes/icon/rme_icon.xpm"

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_CLOSE(MainFrame::OnExit)

	// Update check complete
#ifdef _USE_UPDATER_
	EVT_ON_UPDATE_CHECK_FINISHED(wxID_ANY, MainFrame::OnUpdateReceived)
#endif
	EVT_ON_UPDATE_MENUS(wxID_ANY, MainFrame::OnUpdateMenus)
	EVT_ON_UPDATE_ACTIONS(wxID_ANY, MainFrame::OnUpdateActions)

	// Idle event handler
	EVT_IDLE(MainFrame::OnIdle)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(MapWindow, wxPanel)
	EVT_SIZE(MapWindow::OnSize)

	EVT_COMMAND_SCROLL_TOP       (MAP_WINDOW_HSCROLL, MapWindow::OnScroll)
	EVT_COMMAND_SCROLL_BOTTOM    (MAP_WINDOW_HSCROLL, MapWindow::OnScroll)
	EVT_COMMAND_SCROLL_THUMBTRACK(MAP_WINDOW_HSCROLL, MapWindow::OnScroll)
	EVT_COMMAND_SCROLL_LINEUP    (MAP_WINDOW_HSCROLL, MapWindow::OnScrollLineUp)
	EVT_COMMAND_SCROLL_LINEDOWN  (MAP_WINDOW_HSCROLL, MapWindow::OnScrollLineDown)
	EVT_COMMAND_SCROLL_PAGEUP    (MAP_WINDOW_HSCROLL, MapWindow::OnScrollPageUp)
	EVT_COMMAND_SCROLL_PAGEDOWN  (MAP_WINDOW_HSCROLL, MapWindow::OnScrollPageDown)

	EVT_COMMAND_SCROLL_TOP       (MAP_WINDOW_VSCROLL, MapWindow::OnScroll)
	EVT_COMMAND_SCROLL_BOTTOM    (MAP_WINDOW_VSCROLL, MapWindow::OnScroll)
	EVT_COMMAND_SCROLL_THUMBTRACK(MAP_WINDOW_VSCROLL, MapWindow::OnScroll)
	EVT_COMMAND_SCROLL_LINEUP    (MAP_WINDOW_VSCROLL, MapWindow::OnScrollLineUp)
	EVT_COMMAND_SCROLL_LINEDOWN  (MAP_WINDOW_VSCROLL, MapWindow::OnScrollLineDown)
	EVT_COMMAND_SCROLL_PAGEUP    (MAP_WINDOW_VSCROLL, MapWindow::OnScrollPageUp)
	EVT_COMMAND_SCROLL_PAGEDOWN  (MAP_WINDOW_VSCROLL, MapWindow::OnScrollPageDown)

	EVT_BUTTON(MAP_WINDOW_GEM, MapWindow::OnGem)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(MapScrollBar, wxScrollBar)
	EVT_KEY_DOWN(MapScrollBar::OnKey)
	EVT_KEY_UP(MapScrollBar::OnKey)
	EVT_CHAR(MapScrollBar::OnKey)
	EVT_SET_FOCUS(MapScrollBar::OnFocus)
	EVT_MOUSEWHEEL(MapScrollBar::OnWheel)
END_EVENT_TABLE()

wxIMPLEMENT_APP(Application);

Application::~Application()
{
	// Destroy
}

bool Application::OnInit()
{
#if defined __DEBUG_MODE__ && defined __WINDOWS__
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	std::cout << "This is free software: you are free to change and redistribute it." << std::endl;
	std::cout << "There is NO WARRANTY, to the extent permitted by law." << std::endl;
	std::cout << "Review COPYING in RME distribution for details." << std::endl;
	mt_seed(time(nullptr));
	srand(time(nullptr));

	// Tell that we are the real thing
	wxAppConsole::SetInstance(this);
	wxArtProvider::Push(new ArtProvider());

#if defined(__LINUX__) || defined(__WINDOWS__)
	{
		int glutArgc = 1;
		glutInit(&glutArgc, argv);
	}
#endif

	// Load some internal stuff
	g_settings.load();
	FixVersionDiscrapencies();
	g_editor.LoadHotkeys();

#ifdef _USE_PROCESS_COM
	m_single_instance_checker = newd wxSingleInstanceChecker; //Instance checker has to stay alive throughout the applications lifetime
	if(g_settings.getInteger(Config::ONLY_ONE_INSTANCE) && m_single_instance_checker->IsAnotherRunning()) {
		RMEProcessClient client;
		wxConnectionBase* connection = client.MakeConnection("localhost", "rme_host", "rme_talk");
		if(connection) {
			if(argc == 2){
				wxLogNull nolog; //We might get a timeout message if the file fails to open on the running instance. Let's not show that message.
				connection->Execute(argv[1]);
			}
			connection->Disconnect();
			wxDELETE(connection);
		}
		wxDELETE(m_single_instance_checker);
		return false; //Since we return false - OnExit is never called
	}
	// We act as server then
	m_proc_server = newd RMEProcessServer();
	if(!m_proc_server->Create("rme_host")) {
		wxLogWarning("Could not register IPC service!");
	}
#endif

	// Image handlers
	//wxImage::AddHandler(newd wxBMPHandler);
	wxImage::AddHandler(newd wxPNGHandler);
	wxImage::AddHandler(newd wxJPEGHandler);
	wxImage::AddHandler(newd wxTGAHandler);

	g_editor.gfx.loadEditorSprites();

#ifndef __DEBUG_MODE__
	//wxHandleFatalExceptions(true);
#endif

    m_file_to_open = wxEmptyString;
	if(argc == 2){
		m_file_to_open = argv[1];
	}

    g_editor.root = newd MainFrame(__W_RME_APPLICATION_NAME__, wxDefaultPosition, wxSize(700, 500));
	SetTopWindow(g_editor.root);
	g_editor.SetTitle("");
	g_editor.LoadRecentFiles();
	g_editor.LoadPerspective();

    wxIcon icon(rme_icon);
    g_editor.root->SetIcon(icon);

    if(g_settings.getInteger(Config::WELCOME_DIALOG) == 1 && m_file_to_open == wxEmptyString) {
        g_editor.ShowWelcomeDialog(icon);
    } else {
        g_editor.root->Show();
    }

	// Set idle event handling mode
	wxIdleEvent::SetMode(wxIDLE_PROCESS_SPECIFIED);

	// Goto RME website?
	if(g_settings.getInteger(Config::GOTO_WEBSITE_ON_BOOT) == 1) {
		::wxLaunchDefaultBrowser("http://www.remeresmapeditor.com/", wxBROWSER_NEW_WINDOW);
		g_settings.setInteger(Config::GOTO_WEBSITE_ON_BOOT, 0);
	}

	// Check for updates
#ifdef _USE_UPDATER_
	if(g_settings.getInteger(Config::USE_UPDATER) == -1) {
		int ret = g_editor.PopupDialog(
			"Notice",
			"Do you want the editor to automatically check for updates?\n"
			"It will connect to the internet if you choose yes.\n"
			"You can change this setting in the preferences later.", wxYES | wxNO);
		if(ret == wxID_YES) {
			g_settings.setInteger(Config::USE_UPDATER, 1);
		} else {
			g_settings.setInteger(Config::USE_UPDATER, 0);
		}
	}
	if(g_settings.getInteger(Config::USE_UPDATER) == 1) {
		UpdateChecker updater;
		updater.connect(g_editor.root);
	}
#endif

    // Keep track of first event loop entry
    m_startup = true;
	return true;
}

void Application::OnEventLoopEnter(wxEventLoopBase* loop) {
	// TODO(fusion): Is this flag even required?
    if(!m_startup)
        return;
    m_startup = false;

    if(m_file_to_open != wxEmptyString) {
        g_editor.OpenProject(m_file_to_open);
    } else if(!g_editor.IsWelcomeDialogShown()) {
		g_editor.NewProject();
    }
}

void Application::MacOpenFiles(const wxArrayString& fileNames)
{
	if(!fileNames.IsEmpty()) {
		g_editor.OpenProject(fileNames.Item(0));
	}
}

void Application::FixVersionDiscrapencies()
{
	// Here the registry should be fixed, if the version has been changed
	if(g_settings.getInteger(Config::VERSION_ID) < MAKE_VERSION_ID(1, 0, 5)) {
		g_settings.setInteger(Config::USE_MEMCACHED_SPRITES_TO_SAVE, 0);
	}

	wxString ss = wxstr(g_settings.getString(Config::SCREENSHOT_DIRECTORY));
	if(ss.empty()) {
		ss = wxStandardPaths::Get().GetDocumentsDir();
#ifdef __WINDOWS__
		ss += "/My Pictures/RME/";
#endif
	}
	g_settings.setString(Config::SCREENSHOT_DIRECTORY, nstr(ss));

	// Set registry to newest version
	g_settings.setInteger(Config::VERSION_ID, __RME_VERSION_ID__);
}

void Application::Unload()
{
	g_editor.CloseProject();
	g_editor.SaveHotkeys();
	g_editor.SavePerspective();
	g_editor.SaveRecentFiles();
	g_settings.save(true);
}

int Application::OnExit()
{
#ifdef _USE_PROCESS_COM
	wxDELETE(m_proc_server);
	wxDELETE(m_single_instance_checker);
#endif
	return 1;
}

void Application::OnFatalException()
{
	////
}

MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size) :
	wxFrame((wxFrame *)nullptr, -1, title, pos, size, wxDEFAULT_FRAME_STYLE)
{
	// Receive idle events
	SetExtraStyle(wxWS_EX_PROCESS_IDLE);

	#if wxCHECK_VERSION(3, 1, 0) //3.1.0 or higher
		// Make sure ShowFullScreen() uses the full screen API on macOS
		EnableFullScreenView(true);
    #endif

	// Creates the file-dropdown menu
	wxString error;         // unused?
	wxArrayString warnings; // unused?
	g_editor.menubar = newd MainMenuBar(this);
	if(!g_editor.menubar->Load(GetExecDirectory() + "menubar.xml", warnings, error)) {
		wxLogError(wxString() + "Could not load menubar.xml, editor will NOT be able to show the menu.\n");
	}

	g_editor.aui_manager = newd wxAuiManager(this);
	g_editor.toolbar = newd MainToolBar(this, g_editor.aui_manager);
	g_editor.mapWindow = newd MapWindow(this);
	g_editor.aui_manager->AddPane(g_editor.mapWindow, wxAuiPaneInfo().CenterPane().Floatable(false).CloseButton(false).PaneBorder(false));

	g_editor.aui_manager->Update();
	g_editor.UpdateMenubar();

	wxStatusBar* statusbar = CreateStatusBar();
	statusbar->SetFieldsCount(4);
	SetStatusText(wxString("Welcome to ") << __W_RME_APPLICATION_NAME__ << " " << __W_RME_VERSION__);
}

MainFrame::~MainFrame(){
	// no-op
}

void MainFrame::OnIdle(wxIdleEvent& event)
{
	////
}

#ifdef _USE_UPDATER_
void MainFrame::OnUpdateReceived(wxCommandEvent& event)
{
	std::string data = *(std::string*)event.GetClientData();
	delete (std::string*)event.GetClientData();
	size_t first_colon = data.find(':');
	size_t second_colon = data.find(':', first_colon+1);

	if(first_colon == std::string::npos || second_colon == std::string::npos)
		return;

	std::string update = data.substr(0, first_colon);
	std::string verstr = data.substr(first_colon+1, second_colon-first_colon-1);
	std::string url = (second_colon == data.size()? "" : data.substr(second_colon+1));

	if(update == "yes") {
		int ret = g_editor.PopupDialog(
			"Update Notice",
			wxString("There is a newd update available (") << wxstr(verstr) <<
			"). Do you want to go to the website and download it?",
			wxYES | wxNO,
			"I don't want any update notices",
			Config::AUTOCHECK_FOR_UPDATES
			);
		if(ret == wxID_YES)
			::wxLaunchDefaultBrowser(wxstr(url),  wxBROWSER_NEW_WINDOW);
	}
}
#endif

void MainFrame::OnUpdateMenus(wxCommandEvent&)
{
	g_editor.UpdateMenubar();
	g_editor.UpdateMinimap(true);
}

void MainFrame::OnUpdateActions(wxCommandEvent&)
{
	g_editor.toolbar->UpdateButtons();
	g_editor.RefreshActions();
}

#ifdef __WINDOWS__
bool MainFrame::MSWTranslateMessage(WXMSG *msg)
{
	if(g_editor.AreHotkeysEnabled()) {
		if(wxFrame::MSWTranslateMessage(msg))
			return true;
	} else {
		if(wxWindow::MSWTranslateMessage(msg))
			return true;
	}
	return false;
}
#endif

bool MainFrame::DoQueryClose() {
	return true;
}

bool MainFrame::DoQuerySave(bool doclose)
{
	if(!g_editor.IsProjectOpen()) {
		return true;
	}

	if(g_editor.IsProjectDirty()) {
		long ret = g_editor.PopupDialog(
			"Save changes",
			"Do you want to save your changes to \"" + g_editor.projectDir + "\"?",
			wxYES | wxNO | wxCANCEL
		);

		if(ret == wxID_CANCEL){
			return false;
		}

		g_editor.SaveProject();
	}

	if(doclose) {
		g_editor.CloseProject();
	}

	return true;
}

bool MainFrame::DoQueryImportCreatures()
{
#if TODO
	if(g_creatures.hasMissing()) {
		long ret = g_editor.PopupDialog("Missing creatures", "There are missing creatures and/or NPC in the editor, do you want to load them from an OT monster/npc file?", wxYES | wxNO);
		if(ret == wxID_YES) {
			do {
				wxFileDialog dlg(g_editor.root, "Import monster/npc file", "","","*.xml", wxFD_OPEN | wxFD_MULTIPLE | wxFD_FILE_MUST_EXIST);
				if(dlg.ShowModal() == wxID_OK) {
					wxArrayString paths;
					dlg.GetPaths(paths);
					for(uint32_t i = 0; i < paths.GetCount(); ++i) {
						wxString error;
						wxArrayString warnings;
						bool ok = g_creatures.importXMLFromOT(paths[i], error, warnings);
						if(ok)
							g_editor.ListDialog("Monster loader errors", warnings);
						else
							wxMessageBox("Error OT data file \"" + paths[i] + "\".\n" + error, "Error", wxOK | wxICON_INFORMATION, g_editor.root);
					}
				} else {
					break;
				}
			} while(g_creatures.hasMissing());
		}
	}
	g_editor.RefreshPalettes();
#endif
	return true;
}

void MainFrame::UpdateFloorMenu()
{
	g_editor.menubar->UpdateFloorMenu();
}

void MainFrame::UpdateIndicatorsMenu()
{
	g_editor.menubar->UpdateIndicatorsMenu();
}

void MainFrame::OnExit(wxCloseEvent& event)
{
	if(g_editor.IsProjectOpen()) {
		if(!DoQuerySave() && event.CanVeto()) {
			event.Veto();
			return;
		}
	}

	Application &app = static_cast<Application&>(wxGetApp());
	app.Unload();
	app.ExitMainLoop();
}

void MainFrame::PrepareDC(wxDC& dc)
{
	dc.SetLogicalOrigin( 0, 0 );
	dc.SetAxisOrientation( 1, 0);
	dc.SetUserScale( 1.0, 1.0 );
	dc.SetMapMode( wxMM_TEXT );
}

#ifdef _WIN32
// This is necessary for cmake to understand that it needs to set the executable
int main(int argc, char** argv)
{
	wxEntryStart(argc, argv); // Start the wxWidgets library
	Application* app = new Application(); // Create the application object
	wxApp::SetInstance(app); // Informs wxWidgets that app is the application object
	wxEntry(); // Call the wxEntry() function to start the application execution
	wxEntryCleanup(); // Clear the wxWidgets library
	return 0;
}
#endif
