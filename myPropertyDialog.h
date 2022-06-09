
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

#ifndef MYPROPERTYDIALOG_H_
#define MYPROPERTYDIALOG_H_

#include <wx/dialog.h>
#include <wx/propgrid/propgrid.h>
#include <wx/fileconf.h>
#include <wx/stattext.h>
#include <string>
#include <map>


class PropertyDialog: public wxDialog
{
	public:
		PropertyDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);

		~PropertyDialog();
		void LoadConfig();
		void UpdateProperty(wxPropertyGridEvent& event);
		void FilterGrid(wxCommandEvent& event);
		void resetFilter(wxCommandEvent& event);
		std::map<std::string,std::string> FilterList(wxString filter);
		std::string FilterString(wxString filter);
		
		bool PropExists(wxString name);
		void AddProp(wxCommandEvent& event);
		void DelProp(wxCommandEvent& event);
		void HideDialog(wxCommandEvent& event);
		void ClearModifiedStatus();


	private:
		wxArrayString split(wxString str, wxString delim);
	
		wxPropertyGrid *pg;
		wxTextCtrl *fil;
		wxBoxSizer *sz, *ct;

};

#endif
