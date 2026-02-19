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

#include "main.h"
#include "welcome_dialog.h"
#include "settings.h"
#include "preferences.h"
#include <wx/dirdlg.h>

wxDEFINE_EVENT(WELCOME_DIALOG_ACTION, wxCommandEvent);

WelcomeDialog::WelcomeDialog(const wxString& title_text,
                             const wxString &version_text,
                             const wxSize& size,
                             const wxBitmap &rme_logo,
                             const std::vector<wxString> &recent_files)
        : wxDialog(nullptr, wxID_ANY, "", wxDefaultPosition, size) {
    Centre();
    wxColour base_colour = wxColor(250, 250, 250);
    m_welcome_dialog_panel = newd WelcomeDialogPanel(this,
                                                     GetClientSize(),
                                                     title_text,
                                                     version_text,
                                                     base_colour,
                                                     wxBitmap(rme_logo.ConvertToImage().Scale(FROM_DIP(this, 48), FROM_DIP(this, 48))),
                                                     recent_files);
}

void WelcomeDialog::OnButtonClicked(const wxMouseEvent &event) {
    auto *button = dynamic_cast<WelcomeDialogButton *>(event.GetEventObject());
    wxSize button_size = button->GetSize();
    wxPoint click_point = event.GetPosition();
    if(click_point.x > 0 && click_point.x < button_size.x && click_point.y > 0 && click_point.y < button_size.x) {
        if(button->GetAction() == wxID_PREFERENCES) {
            PreferencesWindow preferences_window(m_welcome_dialog_panel);
            preferences_window.ShowModal();
            m_welcome_dialog_panel->updateInputs();
        } else {
            wxCommandEvent action_event(WELCOME_DIALOG_ACTION);
            if(button->GetAction() == wxID_OPEN) {
                wxDirDialog openDialog(this, "Select project directory...", wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
                if(openDialog.ShowModal() == wxID_CANCEL) {
                    return;
                }

                action_event.SetString(openDialog.GetPath());
            }
            action_event.SetId(button->GetAction());
            ProcessWindowEvent(action_event);
        }
    }
}

void WelcomeDialog::OnCheckboxClicked(const wxCommandEvent &event) {
    g_settings.setInteger(Config::WELCOME_DIALOG, event.GetInt());
}

void WelcomeDialog::OnRecentProjectClicked(const wxMouseEvent &event) {
    RecentProject *obj = dynamic_cast<RecentProject*>(event.GetEventObject());
    if(obj && obj->GetClientRect().Contains(event.GetPosition())){
        wxCommandEvent action_event(WELCOME_DIALOG_ACTION);
        action_event.SetString(obj->GetText());
        action_event.SetId(wxID_OPEN);
        ProcessWindowEvent(action_event);
    }
}

WelcomeDialogPanel::WelcomeDialogPanel(WelcomeDialog *dialog,
                                       const wxSize &size,
                                       const wxString &title_text,
                                       const wxString &version_text,
                                       const wxColour &base_colour,
                                       const wxBitmap &rme_logo,
                                       const std::vector<wxString> &recent_files)
        : wxPanel(dialog),
          m_rme_logo(rme_logo),
          m_title_text(title_text),
          m_version_text(version_text),
          m_text_colour(base_colour.ChangeLightness(40)),
          m_background_colour(base_colour) {

    Bind(wxEVT_PAINT, &WelcomeDialogPanel::OnPaint, this);

    wxSizer *rootSizer = newd wxBoxSizer(wxHORIZONTAL);

	{
		wxSizer *vertical_sizer = newd wxBoxSizer(wxVERTICAL);

		{
			wxSize button_size = FROM_DIP(this, wxSize(150, 35));
			wxColour button_base_colour = base_colour.ChangeLightness(90);

			wxSizer *buttons_sizer = newd wxBoxSizer(wxVERTICAL);
			buttons_sizer->AddSpacer(size.y / 2);

			WelcomeDialogButton *new_project_button = newd WelcomeDialogButton(this,
					wxDefaultPosition, button_size, button_base_colour, "New");
			new_project_button->SetAction(wxID_NEW);
			new_project_button->Bind(wxEVT_LEFT_UP, &WelcomeDialog::OnButtonClicked, dialog);
			buttons_sizer->Add(new_project_button, 0, wxALIGN_CENTER | wxTOP, FROM_DIP(this, 10));

			WelcomeDialogButton *open_project_button = newd WelcomeDialogButton(this,
					wxDefaultPosition, button_size, button_base_colour, "Open");
			open_project_button->SetAction(wxID_OPEN);
			open_project_button->Bind(wxEVT_LEFT_UP, &WelcomeDialog::OnButtonClicked, dialog);
			buttons_sizer->Add(open_project_button, 0, wxALIGN_CENTER | wxTOP, FROM_DIP(this, 10));


			WelcomeDialogButton *preferences_button = newd WelcomeDialogButton(this,
					wxDefaultPosition, button_size, button_base_colour, "Preferences");
			preferences_button->SetAction(wxID_PREFERENCES);
			preferences_button->Bind(wxEVT_LEFT_UP, &WelcomeDialog::OnButtonClicked, dialog);
			buttons_sizer->Add(preferences_button, 0, wxALIGN_CENTER | wxTOP, FROM_DIP(this, 10));

			vertical_sizer->Add(buttons_sizer, 1, wxEXPAND);
		}

		{
			wxSizer *sizer = newd wxBoxSizer(wxHORIZONTAL);
			m_show_welcome_dialog_checkbox = newd wxCheckBox(this, wxID_ANY, "Show this dialog on startup");
			m_show_welcome_dialog_checkbox->SetValue(g_settings.getInteger(Config::WELCOME_DIALOG) == 1);
			m_show_welcome_dialog_checkbox->Bind(wxEVT_CHECKBOX, &WelcomeDialog::OnCheckboxClicked, dialog);
			m_show_welcome_dialog_checkbox->SetForegroundColour(m_text_colour);
			m_show_welcome_dialog_checkbox->SetBackgroundColour(m_background_colour);
			sizer->Add(m_show_welcome_dialog_checkbox, wxSizerFlags(1).Expand().Border(wxALL, FROM_DIP(this, 10)));
			vertical_sizer->Add(sizer);
		}

		rootSizer->Add(vertical_sizer, 1, wxEXPAND);
	}

	{
		RecentProjectsPanel *recent_projects_panel = newd RecentProjectsPanel(this, dialog, base_colour, recent_files);
		recent_projects_panel->SetMaxSize(wxSize(size.x / 2, size.y));
		recent_projects_panel->SetBackgroundColour(base_colour.ChangeLightness(98));
		rootSizer->Add(recent_projects_panel, 1, wxEXPAND);
	}

    SetSizerAndFit(rootSizer);
}

void WelcomeDialogPanel::updateInputs() {
    m_show_welcome_dialog_checkbox->SetValue(g_settings.getInteger(Config::WELCOME_DIALOG) == 1);
}

void WelcomeDialogPanel::OnPaint(const wxPaintEvent &event) {
    wxPaintDC dc(this);

    dc.SetBrush(wxBrush(m_background_colour));
    dc.SetPen(wxPen(m_background_colour));
    dc.DrawRectangle(wxRect(wxPoint(0, 0), GetClientSize()));

    dc.DrawBitmap(m_rme_logo, wxPoint(GetSize().x / 4 - m_rme_logo.GetWidth() / 2, FROM_DIP(this, 40)), true);

    wxFont font = GetFont();
    font.SetPointSize(18);
    dc.SetFont(font);
    wxSize header_size = dc.GetTextExtent(m_title_text);
    wxSize header_point(GetSize().x / 4, GetSize().y / 4);
    dc.SetTextForeground(m_text_colour);
    dc.DrawText(m_title_text, wxPoint(header_point.x - header_size.x / 2, header_point.y));

    dc.SetFont(GetFont());
    wxSize version_size = dc.GetTextExtent(m_version_text);
    dc.SetTextForeground(m_text_colour.ChangeLightness(110));
    dc.DrawText(m_version_text, wxPoint(header_point.x - version_size.x / 2, header_point.y + header_size.y + 10));
}

WelcomeDialogButton::WelcomeDialogButton(wxWindow *parent,
                                         const wxPoint &pos,
                                         const wxSize &size,
                                         const wxColour &base_colour,
                                         const wxString &text)
        : wxPanel(parent, wxID_ANY, pos, size),
          m_action(wxID_CLOSE),
          m_text(text),
          m_text_colour(base_colour.ChangeLightness(40)),
          m_background(base_colour.ChangeLightness(96)),
          m_background_hover(base_colour.ChangeLightness(93)),
          m_is_hover(false) {
    Bind(wxEVT_PAINT, &WelcomeDialogButton::OnPaint, this);
    Bind(wxEVT_ENTER_WINDOW, &WelcomeDialogButton::OnMouseEnter, this);
    Bind(wxEVT_LEAVE_WINDOW, &WelcomeDialogButton::OnMouseLeave, this);
}

void WelcomeDialogButton::OnPaint(const wxPaintEvent &event) {
    wxPaintDC dc(this);

    wxColour colour = m_is_hover ? m_background_hover : m_background;
    dc.SetBrush(wxBrush(colour));
    dc.SetPen(wxPen(colour, 1));
    dc.DrawRectangle(wxRect(wxPoint(0, 0), GetClientSize()));

    dc.SetFont(GetFont());
    dc.SetTextForeground(m_text_colour);
    wxSize text_size = dc.GetTextExtent(m_text);
    dc.DrawText(m_text, wxPoint(GetSize().x / 2 - text_size.x / 2, GetSize().y / 2 - text_size.y / 2));
}

void WelcomeDialogButton::OnMouseEnter(const wxMouseEvent &event) {
    m_is_hover = true;
    Refresh();
}

void WelcomeDialogButton::OnMouseLeave(const wxMouseEvent &event) {
    m_is_hover = false;
    Refresh();
}

RecentProjectsPanel::RecentProjectsPanel(wxWindow *parent,
                                 WelcomeDialog *dialog,
                                 const wxColour &base_colour,
                                 const std::vector<wxString> &recent_files)
        : wxPanel(parent, wxID_ANY) {
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    for(const wxString &dir: recent_files) {
        auto *recent_item = newd RecentProject(this, base_colour, dir);
        sizer->Add(recent_item, 0, wxEXPAND);
        recent_item->Bind(wxEVT_LEFT_UP, &WelcomeDialog::OnRecentProjectClicked, dialog);
    }
    SetSizerAndFit(sizer);
}

RecentProject::RecentProject(wxWindow *parent,
                       const wxColour &base_colour,
                       const wxString &item_name)
        : wxPanel(parent, wxID_ANY),
          m_text_colour(base_colour.ChangeLightness(40)),
          m_text_colour_hover(base_colour.ChangeLightness(20)),
          m_item_text(item_name) {
    SetBackgroundColour(base_colour.ChangeLightness(95));
    m_title = newd wxStaticText(this, wxID_ANY, wxFileNameFromPath(m_item_text));
    m_title->SetFont(GetFont().Bold());
    m_title->SetForegroundColour(m_text_colour);
    m_title->SetToolTip(m_item_text);
    m_file_path = newd wxStaticText(this, wxID_ANY, m_item_text, wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_START);
    m_file_path->SetToolTip(m_item_text);
    m_file_path->SetFont(GetFont().Smaller());
    m_file_path->SetForegroundColour(m_text_colour);
    wxBoxSizer *mainSizer = newd wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *sizer = newd wxBoxSizer(wxVERTICAL);
    sizer->Add(m_title);
    sizer->Add(m_file_path, 1, wxTOP, FROM_DIP(this, 2));
    mainSizer->Add(sizer, 0, wxEXPAND | wxALL, FROM_DIP(this, 8));
    Bind(wxEVT_ENTER_WINDOW, &RecentProject::OnMouseEnter, this);
    Bind(wxEVT_LEAVE_WINDOW, &RecentProject::OnMouseLeave, this);
    m_title->Bind(wxEVT_LEFT_UP, &RecentProject::PropagateItemClicked, this);
    m_file_path->Bind(wxEVT_LEFT_UP, &RecentProject::PropagateItemClicked, this);
    SetSizerAndFit(mainSizer);
}

void RecentProject::PropagateItemClicked(wxMouseEvent& event) {
    event.ResumePropagation(1);
    event.SetEventObject(this);
    event.Skip();
}

void RecentProject::OnMouseEnter(const wxMouseEvent &event) {
    if(GetScreenRect().Contains(ClientToScreen(event.GetPosition()))
        && m_title->GetForegroundColour() != m_text_colour_hover) {
        m_title->SetForegroundColour(m_text_colour_hover);
        m_file_path->SetForegroundColour(m_text_colour_hover);
        m_title->Refresh();
        m_file_path->Refresh();
    }
}

void RecentProject::OnMouseLeave(const wxMouseEvent &event) {
    if(!GetScreenRect().Contains(ClientToScreen(event.GetPosition()))
        && m_title->GetForegroundColour() != m_text_colour) {
        m_title->SetForegroundColour(m_text_colour);
        m_file_path->SetForegroundColour(m_text_colour);
        m_title->Refresh();
        m_file_path->Refresh();
    }
}
