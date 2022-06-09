
/*
    PropertyDialog
    
    Part of wxPolygon - Interactive tool to build polygons for OpenSCAD
    Copyright (C) 2022  Glenn Butcher, glenn.butcher@gmail.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "myPropertyDialog.h"
#include "myConfig.h"
#include <wx/sizer.h>
#include <wx/propgrid/advprops.h>

#include <wx/wx.h>

#ifdef WIN32
#define TEXTCTRLHEIGHT 25
#define TEXTHEIGHT 15
#else
#define TEXTCTRLHEIGHT 30
#define TEXTHEIGHT 25
#endif

#define FILTERID 3100
#define ADDID 3101
#define DELETEID 3102
#define HIDEID 3103
#define RESETFILTERID 3104

class AddDialog: public wxDialog
{
	public:
		AddDialog(wxWindow *parent, wxWindowID id): 
		wxDialog(parent, id, "Add Property", wxDefaultPosition, wxDefaultSize) //wxSize(220,300))
		{
			wxBoxSizer *sz = new wxBoxSizer(wxVERTICAL);
			wxBoxSizer *ct = new wxBoxSizer(wxHORIZONTAL);
			
			sz->Add(new wxStaticText(this, wxID_ANY, "Name: "), 0, wxLEFT, 5);
			name = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(250,TEXTCTRLHEIGHT),wxTE_PROCESS_ENTER);
			sz->Add(name, 0, wxLEFT | wxRIGHT, 5);
			sz->Add(new wxStaticText(this, wxID_ANY, "Value: "), 0, wxLEFT, 5);
			value = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(250,TEXTCTRLHEIGHT),wxTE_PROCESS_ENTER);
			sz->Add(value, 0, wxLEFT | wxRIGHT, 5);
			
			ct->Add(new wxButton(this, wxID_OK, "Ok"), 0, wxALL, 10);
			ct->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 10);
			sz->Add(ct, 0, wxALL, 10);
			SetSizerAndFit(sz);

			Bind(wxEVT_TEXT_ENTER, &AddDialog::OnTextEnter, this);
		}
		
		~AddDialog()
		{
			if (name) delete name;
			if (value) delete value;
		}
		
		wxString GetName()
		{
			return name->GetValue();
		}
		
		wxString GetValue()
		{
			return value->GetValue();
		}
		
		void OnTextEnter(wxCommandEvent& event)
		{
			EndModal(wxID_OK);
		}
		
		
	private:
		wxTextCtrl *name, *value;
	
};

PropertyDialog::PropertyDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size):
wxDialog(parent, id, title, pos, size, wxCAPTION|wxRESIZE_BORDER)
{
	sz = new wxBoxSizer(wxVERTICAL);
	ct = new wxBoxSizer(wxHORIZONTAL);

	pg = new wxPropertyGrid(this, wxID_ANY, wxDefaultPosition, size, wxPG_BOLD_MODIFIED | wxPG_HIDE_MARGIN);
	SetExtraStyle(GetExtraStyle() & ~wxWS_EX_BLOCK_EVENTS);
	
	sz->Add(pg, 1, wxEXPAND | wxALL, 3);
	
	ct->Add(new wxButton(this, wxID_OK, "Dismiss"), 0, wxALL, 10);
	ct->Add(new wxStaticText(this, wxID_ANY, "Filter: "), 0, wxALL, 10);
	fil = new wxTextCtrl(this, FILTERID, "", wxDefaultPosition, wxSize(100,25),wxTE_PROCESS_ENTER);
	ct->Add(fil, 0, wxALL, 10);
	ct->Add(new wxButton(this, RESETFILTERID, "X", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), 0, wxTOP|wxBOTTOM|wxRIGHT, 10);
	ct->Add(new wxButton(this, ADDID, "Add"), 0, wxALL, 10);
	ct->Add(new wxButton(this, DELETEID, "Delete"), 0, wxALL, 10);
	sz->Add(ct, 0, wxALL, 10);
	SetSizerAndFit(sz);
	fil->SetFocus();
	Bind(wxEVT_PG_CHANGED,&PropertyDialog::UpdateProperty,this);
	Bind(wxEVT_TEXT_ENTER, &PropertyDialog::FilterGrid, this);
	Bind(wxEVT_TEXT, &PropertyDialog::FilterGrid, this, FILTERID);
	Bind(wxEVT_BUTTON, &PropertyDialog::AddProp, this, ADDID);
	Bind(wxEVT_BUTTON, &PropertyDialog::DelProp, this, DELETEID);
	Bind(wxEVT_BUTTON, &PropertyDialog::HideDialog, this, HIDEID);
	Bind(wxEVT_BUTTON, &PropertyDialog::resetFilter, this, RESETFILTERID);
}


void PropertyDialog::LoadConfig()
{
	std::map<std::string, std::string> props = myConfig::getConfig().getDefault();
	std::map<std::string, std::string> temps = myConfig::getConfig().getSection("Templates");
	
	myConfig& config = myConfig::getConfig();
	
	for (std::map<std::string, std::string>::iterator it=props.begin(); it!=props.end(); ++it) {
		wxString name = it->first.c_str();
		wxString value = it->second.c_str();

		//find the applicable template, if it exists:
		std::string tplate = config.match_name("Templates", it->first.c_str());
		
		if (tplate != std::string()) {
			//std::string tplate = config.getValue("Templates", it->first);
			if (tplate.find_first_of("|") != std::string::npos) {
				wxArrayString choices = split(wxString(tplate.c_str()), "|");
				wxPGChoices ch(choices);
				pg->Append(new wxEnumProperty(name, wxPG_LABEL, ch, ch.Index(value)));
			}
			else if (tplate.find("iccfile") != std::string::npos) {
				//wxArrayString parms = split(wxString(template.c_str()), ":");
				wxString iccdirectory = config.getValue("cms.profilepath").c_str();
				wxPGProperty* prop = pg->Append(new wxFileProperty(name, wxPG_LABEL, value));
				pg->SetPropertyAttribute(prop,"InitialPath",iccdirectory );
				pg->SetPropertyAttribute(prop,"ShowFullPath",0);
			}
			else if (tplate.find("longstring") != std::string::npos) {
				wxPGProperty* prop = pg->Append(new wxLongStringProperty(name, wxPG_LABEL, value));
			}
		}
		//if no template:
		else pg->Append(new wxStringProperty(name, name, value));
	}
	
	pg->Sort();
}


PropertyDialog::~PropertyDialog()
{
	//if (pg) delete pg;
}

wxArrayString PropertyDialog::split(wxString str, wxString delim)
{
	wxArrayString a;
	wxStringTokenizer tokenizer(str, delim);
	while ( tokenizer.HasMoreTokens() ) {
		wxString token = tokenizer.GetNextToken();
		a.Add(token);
	}
	return a;
}

void PropertyDialog::HideDialog(wxCommandEvent& event)
{
	Hide();
}

void PropertyDialog::ClearModifiedStatus()
{
	if (pg) pg->ClearModifiedStatus();
}

void PropertyDialog::UpdateProperty(wxPropertyGridEvent& event)
{
	//Note: rawprocFrm::UpdateConfig() is changing the .conf file, so don't remove event.Skip()
	Refresh();
	event.Skip();
}

void PropertyDialog::resetFilter(wxCommandEvent& event)
{
	fil->SetValue("");
	int i;
	wxPropertyGridIterator it;
	for ( it = pg->GetIterator();
		!it.AtEnd();
		it++ )
	{
    		wxPGProperty* p = *it;
		p->Hide(false);
	}
	fil->SetFocus();
}

void PropertyDialog::FilterGrid(wxCommandEvent& event)
{
	wxString str = fil->GetValue();
	wxPropertyGridIterator it;
	for ( it = pg->GetIterator();
		!it.AtEnd();
		it++ )
	{
    		wxPGProperty* p = *it;
		if (str == "") {
			p->Hide(false);
		}
		else {
			if (p->GetName().Find(str) == wxNOT_FOUND) {
				p->Hide(true);
			}
			else {
				p->Hide(false);
			}
		}
	}
}

std::map<std::string,std::string> PropertyDialog::FilterList(wxString filter)
{
	std::map<std::string,std::string> params;
	wxPropertyGridIterator it;
	for ( it = pg->GetIterator();
		!it.AtEnd();
		it++ )
	{
  		wxPGProperty* p = *it;

		if (p->GetName().Find(filter) != wxNOT_FOUND) {
			params[std::string(p->GetName().c_str())] = std::string(p->GetValue().GetString().c_str());
		}

	}
	return params;
}

std::string PropertyDialog::FilterString(wxString filter)
{
	std::string params;
	wxPropertyGridIterator it;
	for ( it = pg->GetIterator();
		!it.AtEnd();
		it++ )
	{
  		wxPGProperty* p = *it;

		if (p->GetName().Find(filter) != wxNOT_FOUND) {
			params.append(wxString::Format("%s=%s;",p->GetName(),p->GetValue().GetString()).c_str());
			//params[std::string(p->GetName().c_str())] = std::string(p->GetValue().GetString().c_str());
		}

	}
	return params;
}

bool PropertyDialog::PropExists(wxString name)
{
	wxPropertyGridIterator it;
	for ( it = pg->GetIterator();
		!it.AtEnd();
		it++ )
	{
		wxPGProperty* p = *it;
		if (p->GetName() == name) return true;
	}
	return false;
}

void PropertyDialog::AddProp(wxCommandEvent& event)
{
	AddDialog *add = new AddDialog(this, wxID_ANY);
	if (add->ShowModal() == wxID_OK) {
		if (add->GetName() != "") {
			if (!PropExists(add->GetName())) {
				pg->Append(new wxStringProperty(add->GetName(), add->GetName(), add->GetValue()));
				pg->Sort();
				wxMessageBox(wxString::Format(_("Changed %s to %s."), add->GetName(), add->GetValue()));
				myConfig::getConfig().setValue((const char  *) add->GetName().mb_str(),  (const char  *) add->GetValue().mb_str());
				if (!myConfig::getConfig().flush()) wxMessageBox(_("Write to configuration file failed."));
			}
			else
				wxMessageBox(_("Property already exists."));
		}
		else
			wxMessageBox(_("No name specified."));
	}
	add->~AddDialog();
	
}


void PropertyDialog::DelProp(wxCommandEvent& event)
{
	wxPGProperty* p = pg->GetSelectedProperty();
	if (p) {
		wxString name = p->GetName();
		int answer = wxMessageBox(wxString::Format(_("Delete %s?"),name), _("Confirm"),wxYES_NO | wxCANCEL, this);
		if (answer == wxYES) {
			pg->DeleteProperty(p);
			myConfig::getConfig().deleteValue((const char  *) name.mb_str());
			if (!myConfig::getConfig().flush()) wxMessageBox(_("Write to configuration file failed."));
		}
	}
}
