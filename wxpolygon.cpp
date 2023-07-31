/*
    wxPolygon - Interactive tool to build polygons for OpenSCAD
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

#include "wx/wx.h"
#include "wxpolygon.xpm"
#include "myConfig.h"
#include "myPropertyDialog.h"
#include "strutil.h"
#include <wx/clipbrd.h>
#include <wx/tokenzr.h>
#include <wx/filename.h>
#include <wx/dnd.h>
#include <wx/cmdline.h>
#include <wx/grid.h>
#include <wx/aui/aui.h>
#include <vector>
#include <string>

#define MARGIN 80

//struct pt {int x, y; float r;};
struct pt {float x, y, r;};

#define POLYROUND
#define TEXTCTRLHEIGHT 20

const wxString WXPOLYGON_VERSION = "1.2";

wxArrayString split(wxString str, wxString delim)
{
	wxArrayString a;
	wxStringTokenizer tokenizer(str, delim);
	while ( tokenizer.HasMoreTokens() ) {
		wxString token = tokenizer.GetNextToken();
		a.Add(token);
	}
	return a;
}

std::string getExeDir(std::string filename)
{
	std::string dir;

#ifdef _WIN32
	TCHAR exePath[MAX_PATH];
	GetModuleFileName(NULL, exePath, MAX_PATH) ;
	std::wstring exepath(exePath);
	dir = std::string(exepath.begin(), exepath.end());
	dir.erase(dir.find_last_of('\\'));
	if (filename != "") dir.append("\\"+filename);
#else
	char exePath[PATH_MAX];
	size_t len = readlink("/proc/self/exe", exePath, sizeof(exePath));
	if (len == -1 || len == sizeof(exePath))
	        len = 0;
	exePath[len] = '\0';
	dir = std::string(exePath);
	dir.erase(dir.find_last_of('/'));
	if (filename != "") dir.append("/"+filename);
#endif

	return dir;
}


class myPointDialog: public wxDialog
{
public:
	myPointDialog(wxWindow *parent, float x, float y, float r): wxDialog(parent, wxID_ANY, "Edit Point")
	{
		wxBoxSizer *sz = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer *ct = new wxBoxSizer(wxHORIZONTAL);
			
		sz->Add(new wxStaticText(this, wxID_ANY, "x: "), 0, wxLEFT, 5);
		tx = new wxTextCtrl(this, wxID_ANY, wxString::Format("%0.3f", x), wxDefaultPosition, wxSize(250,TEXTCTRLHEIGHT),wxTE_PROCESS_ENTER);
		sz->Add(tx, 0, wxLEFT | wxRIGHT, 5);
		sz->Add(new wxStaticText(this, wxID_ANY, "y: "), 0, wxLEFT, 5);
		ty = new wxTextCtrl(this, wxID_ANY, wxString::Format("%0.3f", y), wxDefaultPosition, wxSize(250,TEXTCTRLHEIGHT),wxTE_PROCESS_ENTER);
		sz->Add(ty, 0, wxLEFT | wxRIGHT, 5);
		sz->Add(new wxStaticText(this, wxID_ANY, "r: "), 0, wxLEFT, 5);
		tr = new wxTextCtrl(this, wxID_ANY, wxString::Format("%0.3f", r), wxDefaultPosition, wxSize(250,TEXTCTRLHEIGHT),wxTE_PROCESS_ENTER);
		sz->Add(tr, 0, wxLEFT | wxRIGHT, 5);
			
		ct->Add(new wxButton(this, wxID_OK, "Ok"), 0, wxALL, 10);
		ct->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 10);
		sz->Add(ct, 0, wxALL, 10);
		SetSizerAndFit(sz);

		Bind(wxEVT_TEXT_ENTER, &myPointDialog::OnTextEnter, this);
	}
	
	float GetX()
	{
		return atof(tx->GetValue().ToStdString().c_str());
	}
	
	float GetY()
	{
		return atof(ty->GetValue().ToStdString().c_str());
	}
	
	float GetR()
	{
		return atof(tr->GetValue().ToStdString().c_str());
	}
	
	void OnTextEnter(wxCommandEvent& event)
	{
		EndModal(wxID_OK);
	}

private:
	wxTextCtrl *tx, *ty, *tr;

};

class myFrame;

class pointList: public wxGrid
{
public:
	pointList(wxWindow *parent): wxGrid(parent, wxID_ANY)
	{
		CreateGrid( 0, 3 );
		HideRowLabels();
		SetColLabelValue (0, "x");
		SetColLabelValue (1, "y");
		SetColLabelValue (2, "r");
		SetColFormatFloat(0, -1, 3);
		SetColFormatFloat(1, -1, 3);
		SetColFormatFloat(2, -1, 3);
	}
	
	int size()
	{
		return GetNumberRows();
	}
	
	void push_back(pt p)
	{
		AppendRows(1);
		int row = GetNumberRows()-1;
		SetCellValue(row, 0, wxString::Format("%f",p.x));
		SetCellValue(row, 1, wxString::Format("%f",p.y));
		SetCellValue(row, 2, wxString::Format("%f",p.r));	
	}
	
	std::vector<pt> getPoints()
	{
		std::vector<pt> points;
		for (int i=0; i< size(); i++) {
			pt p;
			p.x = atof(GetCellValue(i, 0).ToStdString().c_str());
			p.y = atof(GetCellValue(i, 1).ToStdString().c_str());
			p.r = atof(GetCellValue(i, 2).ToStdString().c_str());
			points.push_back(p);
		}
		return points;
	}
	
	void setPoints(std::vector<pt> pts)
	{
		//ClearGrid();
		DeleteRows(0,GetNumberRows());
		for (int i=0; i< pts.size(); i++) {
			AppendRows(1);
			int row = GetNumberRows()-1;
			SetCellValue(row, 0, wxString::Format("%f",pts[i].x));
			SetCellValue(row, 1, wxString::Format("%f",pts[i].y));
			SetCellValue(row, 2, wxString::Format("%f",pts[i].r));	
		}
	}
	
	void insert(int row, pt p)
	{
		InsertRows(row);
		SetCellValue(row, 0, wxString::Format("%f",p.x));
		SetCellValue(row, 1, wxString::Format("%f",p.y));
		SetCellValue(row, 2, wxString::Format("%f",p.r));	
	}
	
	void change(int row, float x, float y)
	{
		SetCellValue(row, 0, wxString::Format("%f", x));
		SetCellValue(row, 1, wxString::Format("%f", y));
	}
	
	bool erase(int pt)
	{
		return DeleteRows(pt);
	}
	
	void select(int pt)
	{
		SelectRow(pt);
		//SetGridCursor(pt,0);
		GoToCell(pt,0);
	}
	
	pt operator[](int index) const
	{
		pt p;
		p.x = atof(GetCellValue(index, 0).ToStdString().c_str());
		p.y = atof(GetCellValue(index, 1).ToStdString().c_str());
		p.r = atof(GetCellValue(index, 2).ToStdString().c_str());
		return p;
	} 
	
	pt at(int index)
	{
		pt p;
		p.x = atof(GetCellValue(index, 0).ToStdString().c_str());
		p.y = atof(GetCellValue(index, 1).ToStdString().c_str());
		p.r = atof(GetCellValue(index, 2).ToStdString().c_str());
		return p;
	}
	
	
private:
	
	
};

class myPolyPane: public wxPanel
{
public:
	myPolyPane(wxWindow *parent, pointList *p): wxPanel(parent, wxID_ANY)
	{
		ptlist = p;
		dragging=inserting=imgdragging=false;
		SetDoubleBuffered(true);
		imgscale = 1.0;
		//scale = 0.001;
		//precision = 3;
		constraint = 0; //0: no constraint; 1: constrain mousemove to x; 2: constrain mousemove to y
		margin = MARGIN;
		circleradius = 5;
		displayscale = 1.0;
		pos.x = -1; pos.y = -1;
		imgx = imgy = 0;
		//pts.push_back(pt{0,0});  //todo: remove
		ptlist->push_back(pt{0,0});
		ptlist->select(0);
		
		Bind(wxEVT_SIZE, &myPolyPane::OnSize, this);
		Bind(wxEVT_PAINT, &myPolyPane::onPaint, this);
		
		Bind(wxEVT_LEFT_DOWN, &myPolyPane::mouseLeftDown, this);
		Bind(wxEVT_RIGHT_DOWN, &myPolyPane::mouseRightDown, this);
		Bind(wxEVT_RIGHT_DCLICK, &myPolyPane::mouseRightDClick, this);
		Bind(wxEVT_MOTION, &myPolyPane::mouseMoved, this);
		Bind(wxEVT_LEFT_UP, &myPolyPane::mouseReleased, this);
		Bind(wxEVT_RIGHT_UP, &myPolyPane::mouseReleased, this);
		Bind(wxEVT_LEAVE_WINDOW, &myPolyPane::OnMouseLeave,  this);
		Bind(wxEVT_MOUSEWHEEL, &myPolyPane::OnMouseWheel,  this);
		Bind(wxEVT_KEY_DOWN, &myPolyPane::OnKey,  this);
		ptlist->Bind(wxEVT_GRID_CELL_CHANGED, &myPolyPane::OnGrid, this);
	}
	
	int getNumberOfPoints()
	{
		return ptlist->size();
	}
	
	void setScale(float s)
	{
		scale = s;
	}

	float getScale()
	{
		return scale;
	}
	
	void OnSize(wxSizeEvent & evt)
	{
		wxClientDC dc(this);
		render(dc);
	}
	
	void onPaint(wxPaintEvent & evt)
	{
		wxPaintDC dc(this);
		render(dc);
	}
	
	int pointAt(float x, float y)
	{
		for (int i=0; i< ptlist->size(); i++)
			if ((ptlist->at(i).x/scale-circleradius < x/scale) & (ptlist->at(i).x/scale+circleradius > x/scale))
				if ((ptlist->at(i).y/scale-circleradius < y/scale) & (ptlist->at(i).y/scale+circleradius > y/scale))
					return i;
		return -1;
	}
	
	float sqr(float x)
	{
		return x * x;
	}
	
	float distance(pt a, pt b)
	{
		return sqrt(sqr(a.x - b.x) + sqr(a.y - b.y));
	}
	
	bool isBetween(pt a, pt b, pt c)
	{
		float ABC = distance(a,c) + distance(b,c);
		float AB = distance(a,b);
		if ((ABC < AB+scale) & (ABC > AB-scale))
			return true;
		return false;
	}

	int insertBefore(pt p)
	{
		for(int i=1; i<ptlist->size(); i++) {
			if (isBetween(ptlist->at(i-1), ptlist->at(i), p))
				return i;
		}
		return -1;
	}
	
	wxPoint deviceToLogical(wxPoint p)
	{
		int h = GetSize().GetHeight();
		int w = GetSize().GetWidth();
		return wxPoint(p.x, h-p.y);
	}
	
	void render(wxDC& dc)
	{
		//prop - **precision**: Sets the display/export precision of floating point numbers.  Default: 3
		int precision = atoi(myConfig::getConfig().getValueOrDefault("precision","3").c_str());
		int h = dc.GetSize().GetHeight();
		int w = dc.GetSize().GetWidth();
		
		dc.Clear();
		dc.DrawText("Y",margin-15,15);
		dc.DrawText("X",w-25,h-(margin-10));
		dc.SetUserScale(displayscale, displayscale);
		if (img.IsOk()) dc.DrawBitmap(wxBitmap(img.Scale(imgw*imgscale, imgh*imgscale)), imgx, imgy);
		
		dc.SetPen(wxPen(wxColour(128,128,128), 1, wxPENSTYLE_DOT_DASH ));
		dc.DrawLine(margin,h-margin,w,h-margin); //X axis
		dc.DrawLine(margin,h-margin,margin,0); //Y axis
		
		dc.SetPen(*wxBLACK_PEN);
		
		std::vector<pt> pts = ptlist->getPoints();
		bool first = true;
		pt prev = pts[0];
		int cnt = 0;
		for(int i=1; i<pts.size(); i++) {
			dc.DrawLine(margin+(pts[i].x/scale), h-(margin+(pts[i].y/scale)), margin+(prev.x/scale), h-(margin+(prev.y/scale)));			
			prev = pts[i];
			cnt++;
		}
		dc.DrawLine(margin+(prev.x/scale), h-(margin+(prev.y/scale)), margin+(pts[0].x/scale), h-(margin+(pts[0].y/scale)));
		
		for(int i=0; i<pts.size(); i++) {
			if (i == selectedpoint)
				dc.SetPen(*wxRED_PEN);
			else
				dc.SetPen(*wxBLACK_PEN);
			dc.DrawCircle(margin+(pts[i].x/scale), h-(margin+(pts[i].y/scale)), circleradius);
		}
		
		dc.SetPen(*wxBLACK_PEN);
		wxCoord mw, mh;
		if (pos.x >= 0 & pos.y >= 0) {
			wxString prec = wxString::Format("%d",precision);
			wxString fmt = "%0." + prec + "f,%0." + prec + "f";
			wxString mouse = wxString::Format(fmt,(float) pos.x*scale, (float) pos.y*scale);
			dc.GetTextExtent(mouse, &mw, &mh);
			dc.DrawText(mouse,margin+(pos.x-mw), h-(margin+(pos.y+mh)));
		}
	}
	
	void OnMouseWheel(wxMouseEvent& event)
	{
		double increment = scale/10;
		//if (event.ShiftDown()) increment = scale*2;
		//if (event.ControlDown()) increment = scale*5;
		
		if (event.AltDown()) {
			if (event.GetWheelRotation() > 0)
				imgscale *= 1.1;
			else
				imgscale *= 0.9;
			Refresh();
			return;
		}

		if (event.GetWheelRotation() > 0)
			scale -= increment;
		else
			scale += increment;
		Refresh();
	}
	
	void mouseLeftDown(wxMouseEvent& event)
	{
		pos = event.GetPosition();
		int m = margin;
		int h = GetSize().GetHeight();
		int w = GetSize().GetWidth();
		
		pos.x = pos.x-m;
		pos.y = h-m-pos.y;
		
		if (pos.x < 0) pos.x = 0;
		if (pos.y < 0) pos.y = 0;
		
		if (event.ShiftDown()) {
			pos.x = ((pos.x + 5) / 10) *10;
			pos.y = ((pos.y + 5) / 10) *10;
		}
		if (event.ControlDown()) {
			pos.x = ((pos.x + 50) / 100) *100;
			pos.y = ((pos.y + 50) / 100) *100;
		}
		
		pt p; p.x = (float) pos.x*scale; p.y = (float) pos.y*scale; p.r = 0.0;
		begin.x = p.x;
		begin.y = p.y;
		int pnt = pointAt(p.x, p.y);
		if (pnt > -1) { //mouse is on a point
			dragging = true;
			selectedpoint = pnt;
		}
		else { //mouse is on a line between two points
			
			int i = insertBefore(p);
			if ( i > -1) {
				//pts.insert(pts.begin()+i, p); //todo: remove
				ptlist->insert(i, p);
				selectedpoint = i;
				inserting =  true;
				dragging = true;
				setModified(true);
			}
			else {
				ptlist->push_back(p);
				selectedpoint = ptlist->size()-1;
				dragging = true;
				setModified(true);
			}
		}
		ptlist->select(selectedpoint);
		constraint = 0;
			
		Refresh();
	}


	void mouseRightDown(wxMouseEvent& event)
	{
		wxPoint p = event.GetPosition();
		int m = margin;
		int h = GetSize().GetHeight();
		int w = GetSize().GetWidth();
		
		p.x = p.x-m;
		p.y = h-m-p.y;
		
		//if (event.AltDown()) {
		prevx=pos.x; prevy=pos.y;
		imgdragging = true;
		//}
		
		/*
		pt pt; pt.x = (float) pos.x*scale; pt.y = (float) pos.y*scale;
		
		int pnt = pointAt(pt.x, pt.y);
		myPointDialog ptdialog(this, pts[pnt].x, pts[pnt].y, pts[pnt].r);
		if (ptdialog.ShowModal() ==  wxID_OK) {
			pts[pnt].x = ptdialog.GetX();
			pts[pnt].y = ptdialog.GetY();
			pts[pnt].r = ptdialog.GetR();
			setModified(true);
			Refresh();
		}
		*/
	}
	
	void mouseRightDClick(wxMouseEvent& event)
	{
		imgx = imgy = 0;
		imgscale = 1.0;
		Refresh();
	}

	
	void OnGrid(wxGridEvent &event)
	{
		setModified(true);
		Refresh();
	}
	
	void mouseMoved(wxMouseEvent& event)
	{
		((wxFrame *) GetParent())->SetStatusText("");
		pos = event.GetPosition();

		int m = margin;
		int h = GetSize().GetHeight();
		int w = GetSize().GetWidth();
		
		pos.x = pos.x-m;
		pos.y = h-m-pos.y;
		
		if (imgdragging) {
			imgx -= prevx - pos.x;
			imgy += prevy - pos.y;
			prevx = pos.x;
			prevy = pos.y;
			Refresh();
			return;
		}
		
		if (event.AltDown()) {
			if (constraint == 0) {
				if (abs(begin.x - pos.x) > abs(begin.y - pos.y)) 
						constraint = 2;
				else
						constraint = 1;
			}
		}
		
		if (event.ShiftDown()) {
			pos.x = ((pos.x + 5) / 10) *10;
			pos.y = ((pos.y + 5) / 10) *10;
		}
		if (event.ControlDown()) {
			pos.x = ((pos.x + 50) / 100) *100;
			pos.y = ((pos.y + 50) / 100) *100;
		}
		
		if (pos.x < 0) pos.x = 0;
		if (pos.y < 0) pos.y = 0;
		
		pt p; p.x = (float) pos.x*scale; p.y = (float) pos.y*scale;
		
		if (dragging) {
			if (event.AltDown()) {
				if (constraint == 2) 
					p.y = begin.y;
				else if (constraint == 1)
					p.x = begin.x;
			}
			ptlist->change(selectedpoint, p.x, p.y);
			setModified(true);
		}
		
		Refresh();
	}
	
	
	void mouseReleased(wxMouseEvent& event)
	{
		dragging = false;
		inserting = false;
		imgdragging = false;
		constraint = 0;
		Refresh();
	}
	
	void OnMouseLeave(wxMouseEvent& event)
	{
		pos.x = -1;
		pos.y = -1;
		dragging = false;
		inserting = false;
		imgdragging = false;
		constraint = 0;
		Refresh();
	}
	
	void setPointString(wxString s)
	{
		std::vector<pt> polypoints;
		std::string str = s.ToStdString();
		float maxX = 0, maxY = 0;
		
		//basically strips out all between the first '=' and the subsequent ';' 
		str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
		str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
		str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
		std::vector<std::string> vec = bifurcate(str, '=');
		if (vec.size() >= 2) 
			str = bifurcate(vec[1], ';')[0];
		else
			str = vec[0];

		replace_all(str,"],[",";");
		replace_all(str,"]","");
		replace_all(str,"[","");
		std::vector<std::string> points = split(str,";");
		for (std::vector<std::string>::iterator it = points.begin(); it != points.end(); ++it) {
			std::vector<std::string> pnt = split(*it, ",");
			pt p;
			if (pnt.size() >= 2) {
				p.x = atof(pnt[0].c_str());
				p.y = atof(pnt[1].c_str());
			}
			else continue;
			if (pnt.size() >=3) 
				p.r = atof(pnt[2].c_str());
			else
				p.r = 0.0;
			if (p.x > maxX) maxX = p.x;
			if (p.y > maxY) maxY = p.y;
			polypoints.push_back(p);
		}
		if (polypoints.size() > 0) {
			ptlist->setPoints(polypoints);
			int w = GetSize().GetWidth();
			int h = GetSize().GetHeight();
			if (h>w) {
				scale = (maxY / (float) (h-margin)) * 1.05;
			}
			else {
				scale = (maxX / (float) (w-margin)) * 1.05;
			}
			Refresh();
		}
	}
	
	wxString getPointString()
	{
		int precision = atoi(myConfig::getConfig().getValueOrDefault("precision","3").c_str());
		wxString pointstring;
		bool first = true;
		wxString prec = wxString::Format("%d",precision);
		
		std::vector<pt> pts = ptlist->getPoints();
		
		//prop - **polyround**: 0|1, sets the use of the third component in the polygon points, for use by the RoundAnything library.  Default: 0 (only x and y)
		if (myConfig::getConfig().getValueOrDefault("polyround","0") == "1") {
			wxString fmtfirst = "[%0." + prec + "f,%0." + prec + "f,%0." + prec + "f]";
			wxString fmt = ",\n[%0." + prec + "f,%0." + prec + "f,%0." + prec + "f]";
			for(std::vector<pt>::iterator it = pts.begin(); it!= pts.end(); ++it) {
				if (first) 
					pointstring.Append(wxString::Format(fmtfirst,(float)(*it).x, (float) (*it).y, (*it).r));
				else
					pointstring.Append(wxString::Format(fmt,(float)(*it).x, (float) (*it).y, (*it).r));
				first = false;
			}
		}
		else {
			wxString fmtfirst = "[%0." + prec + "f,%0." + prec + "f]";
			wxString fmt = ",\n[%0." + prec + "f,%0." + prec + "f]";
			for(std::vector<pt>::iterator it = pts.begin(); it!= pts.end(); ++it) {
				if (first) 
					pointstring.Append(wxString::Format(fmtfirst,(float)(*it).x, (float) (*it).y));
				else
					pointstring.Append(wxString::Format(fmt,(float)(*it).x, (float) (*it).y));
				first = false;
			}
		}
		return pointstring;
	}
	
	void copyToClipboard()
	{
		clipboardpoints = getPointString();
							
		if (wxTheClipboard->Open())
		{
			wxTheClipboard->SetData( new wxTextDataObject(clipboardpoints) );
			wxTheClipboard->Close();
		}
		((wxFrame *) GetParent())->SetStatusText(wxString::Format("copied %d points from clipboard\n",(int) ptlist->size()));
	}
	
	void pasteFromClipboard() 
	{
		
		if (wxTheClipboard->Open())
		{
			if (wxTheClipboard->IsSupported( wxDF_TEXT ))
			{
				wxTextDataObject data;
				wxTheClipboard->GetData( data );
				clipboardpoints = data.GetText();
				setPointString(clipboardpoints);
				((wxFrame *) GetParent())->SetStatusText(wxString::Format("pasted %d points over existing data.\n",(int) ptlist->size()));
				Refresh();
			}
			wxTheClipboard->Close();
		}
	}
		
	void OnKey(wxKeyEvent& event)
	{
		std::vector<pt> pts = ptlist->getPoints();
		event.Skip();
		int precision = atoi(myConfig::getConfig().getValueOrDefault("precision","3").c_str());
		wxChar uc = event.GetUnicodeKey();
		wxArrayString l, t;  //paste (ctrl-v) data holders
		int i; //loop counter
		std::vector<pt> clippts;  // paste (ctrl-v) data holder
		int x, y; float r;
		pt p;
		wxString s;
		if ( uc != WXK_NONE )
		{
			// It's a "normal" character. Notice that this includes
			// control characters in 1..31 range, e.g. WXK_RETURN or
			// WXK_BACK, so check for them explicitly.
			if ( uc >= 32 )
			{
				switch (uc) {
					//case 45: //- zoom out
					//	event.Skip();
					//	break;
					case 127: //delete: selected point
						if (pts.size() > 1) {
							ptlist->erase(selectedpoint);
							selectedpoint--;
							ptlist->select(selectedpoint);
						}
						event.Skip();
						Refresh();
						break;
						
					case 8: //backspace: remove last point
						//if (pts.size() > 0) pts.pop_back();
						event.Skip();
						Refresh();
						break;
						
					//case 67: //c - with Ctrl-, copy points to clipboard
					//	if (event.ControlDown()) 
					//		copyToClipboard();
					//	break;
						
					//case 86: //v - with Ctrl-, copy points from clipboard
					//	if (event.ControlDown())
					//		pasteFromClipboard();
					//	break;
						
					case 78:  //n
						s = "";
						for (i=0; i<pts.size(); i++) {
							s.Append(wxString::Format("%d: %f,%f  %f\n", i, pts[i].x, pts[i].y, pts[i].r));
						}
						wxMessageBox(s);
						break;
				}
			}
			else
			{
				// It's a control character, < WXK_START
				switch (uc)
				{
					case WXK_TAB:
						event.Skip();
						break;
				}
			}
		}
		else // No Unicode equivalent.
		{
			// It's a special key, > WXK_START, deal with all the known ones:
			switch ( event.GetKeyCode() )
			{



			}
		}
	}
	
	void setModified(bool m)
	{
		modified = m;
		if (m)
			((wxFrame *) GetParent())->SetStatusText("Modified",1);
		else
			((wxFrame *) GetParent())->SetStatusText("",1);
	}
	
	bool isModified()
	{
		return modified;
	}
	
	void setImage(wxImage image)
	{
		img = image;
		imgw = image.GetWidth();
		imgh = image.GetHeight();
	}

private:
	double displayscale;
	float scale;
	int margin;
	wxPoint pos, begin;
	//std::vector<pt> pts;
	bool dragging, inserting;
	int selectedpoint;
	int constraint;
	int circleradius;
	wxString clipboardpoints;
	bool modified;
	pointList *ptlist;
	wxImage img;
	int imgx, imgy, imgw, imgh, prevx, prevy;
	bool imgdragging;
	float imgscale;
};


enum {
	Polygon_Quit = wxID_EXIT,
	Polygon_About = wxID_ABOUT,
	Polygon_Copy,
	Polygon_Paste,
	Polygon_Open,
	Polygon_Save,
	Polygon_SaveAs,
	Polygon_LoadImage,
	Polygon_Properties
};

class myFileDropTarget;

class MyFrame : public wxFrame
{
public:
	MyFrame(const wxString& title): wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(800,600))
	{

		wxMenu *fileMenu = new wxMenu;\
		fileMenu->Append(Polygon_Open, "O&pen\tCtrl-O", "Open a point file");
		fileMenu->Append(Polygon_Save, "S&ave\tCtrl-S", "Save a point file");
		fileMenu->Append(Polygon_SaveAs, "Save A&s...\tCtrl-A", "Save a point file");
		fileMenu->Append(Polygon_LoadImage, "Load I&mage...\tCtrl-L", "Load an image");
		fileMenu->Append(Polygon_Quit, "Q&uit\tCtrl-Q", "Quit wxpolygon");
		
		wxMenu *editMenu = new wxMenu;
		editMenu->Append(Polygon_Copy, "Copy\tCtrl-C", "Copy point set to clipboard");
		editMenu->Append(Polygon_Paste, "Paste\tCtrl-V", "Paste point set in clipboard to wxpolygon.  Overwrites previous point set.");
		editMenu->AppendSeparator();
		editMenu->Append(Polygon_Properties, "Properties...", "Edit program properties");
		
		wxMenu *helpMenu = new wxMenu;
		helpMenu->Append(Polygon_About, "&About\tF1", "Show about dialog");


		wxMenuBar *menuBar = new wxMenuBar();
		menuBar->Append(fileMenu, "&File");
		menuBar->Append(editMenu, "&Edit");
		menuBar->Append(helpMenu, "&Help");

		SetMenuBar(menuBar);
		
		
		grid = new pointList( this);
		poly = new myPolyPane(this, grid);
		
		 //prop - **fontsize**: Sets the font size in points. Requires program restart to take effect. Default: 10.0
		double pointSize = atof(myConfig::getConfig().getValueOrDefault("fontsize","10").c_str());
		wxFont font = poly->GetFont();
		font.SetFractionalPointSize(pointSize);	
		poly->SetFont(font);
		font = grid->GetDefaultCellFont();
		font.SetFractionalPointSize(pointSize);	
		grid->SetDefaultCellFont(font);
		grid->SetDefaultRowSize(int (pointSize*1.33*1.5));


		wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(poly, wxSizerFlags(1).Expand());
		sizer->Add(grid, wxSizerFlags().Expand());
		SetSizer(sizer);
		
		//m_mgr.SetManagedWindow(this);
		//m_mgr.AddPane(grid,  	wxAuiPaneInfo().Left().CloseButton(false).Name("Points"));
		//m_mgr.AddPane(poly, wxCENTER);
		//m_mgr.Update();

		CreateStatusBar(3);
		configfile = "(none)";
		
		propdiag = NULL;
		
		//prop - **filepath**: Sets the default path when the program is started.  Default: current working directory
		file.SetPath(wxString(myConfig::getConfig().getValueOrDefault("filepath","")));
		
		if (myConfig::getConfig().getValueOrDefault("polyround","0") == "1") 
			SetStatusText(_("polyround"),2);
		
		SetDropTarget(new myFileDropTarget(this));

		//wxInitAllImageHandlers();
		Bind(wxEVT_PAINT, &MyFrame::OnPaint, this);
		Bind(wxEVT_CLOSE_WINDOW, &MyFrame::OnClose, this);
		Bind(wxEVT_KEY_DOWN, &myPolyPane::OnKey, poly);
		
		//prop - **maxvalue**: Sets the default maximum value when the program is open.  This is used to scale  Default: 0.001
		float maxvalue = atof(myConfig::getConfig().getValueOrDefault("maxvalue","10.0").c_str());
		int w = poly->GetSize().GetWidth();
		int h = poly->GetSize().GetHeight();
		if (h<w)
			poly->setScale(( maxvalue / (float) (h-MARGIN)) * 1.05);
		else
			poly->setScale(( maxvalue / (float) (w-MARGIN)) * 1.05);
		SetStatusText(wxString::Format("maxvalue: %f",maxvalue));

	}
	
	~MyFrame()
	{
		m_mgr.UnInit();
		//Quit();
	}

	void OnPaint(wxPaintEvent& WXUNUSED(event))
	{
		wxPaintDC dc(this);
		//put dc.DrawWhatever() here...
	}
	
	void OnKey(wxKeyEvent& event)
	{
		printf("MyFrame::OnKey...\n"); fflush(stdout);
		event.Skip();
	}
	
	void OpenFile(wxFileName f)
	{
		wxString pointstring;
		if (f.FileExists()) {
			wxFile openfile(f.GetFullPath(), wxFile::read);
			if (openfile.IsOpened()) {
				openfile.ReadAll(&pointstring);
				openfile.Close();
				poly->setPointString(pointstring);
				file = f;
				poly->setModified(false);
				SetTitle(wxString::Format("wxpolygon: %s", file.GetFullPath()));
				SetStatusText(wxString::Format("%d points read from %s.",(int) poly->getNumberOfPoints(), file.GetFullPath()));
			}
			else wxMessageBox(wxString::Format("File open of %s failed.", file.GetFullPath()));
		}
		else wxMessageBox(wxString::Format("File %s not found.", file.GetFullPath()));
	}
	
	void OnOpen(wxCommandEvent& WXUNUSED(event))
	{
		wxFileName f = wxFileSelector("Specify a point file:", file.GetPath(), "", "scad", wxFileSelectorDefaultWildcardStr, wxFD_OPEN);
		OpenFile(f);
	}
	
	void OnSave(wxCommandEvent& WXUNUSED(event))
	{
		if (file.GetName()== wxEmptyString) {
			file = wxFileSelector("Specify a point file:", file.GetPath(), file.GetFullName(), "scad", wxFileSelectorDefaultWildcardStr, wxFD_SAVE);
			//file.MakeAbsolute();
			
		}
		
		if (file.IsOk()) {
			//prop - **fileformat**: Sets the format of the saved file contents.  Default: "filename = [ %s ];".  Must contain a %s for the place to put the points. "filename" tells wxpolygon to use the filename minus the extension for the name of the polygon point array; any other value will be used literally as the array name.
			wxString fileformat = wxString(myConfig::getConfig().getValueOrDefault("fileformat","filename = [ %s ];"));
			if (fileformat.Contains("filename")) fileformat.Replace("filename",file.GetName());
			wxString filestring = wxString::Format(fileformat, poly->getPointString());
			wxFile savefile(file.GetFullPath(), wxFile::write);
			if (savefile.IsOpened()) {
				savefile.Write(filestring);
				savefile.Close();
				poly->setModified(false);
				SetTitle(wxString::Format("wxpolygon: %s", file.GetFullPath()));
				SetStatusText(wxString::Format("%d points written to %s.",(int) poly->getNumberOfPoints(), file.GetFullPath()));
			}
			else wxMessageBox(wxString::Format("File save to %s failed.",file.GetFullPath()));
		}
		else wxMessageBox(wxString::Format("File %s not found.",file.GetFullPath()));
	}
	
	void OnSaveAs(wxCommandEvent& WXUNUSED(event))
	{
		file = wxFileSelector("Specify a point file:", file.GetPath(), file.GetFullName(), "scad", wxFileSelectorDefaultWildcardStr, wxFD_SAVE);
		//file.MakeAbsolute();

		if (file.IsOk()) {
			wxString fileformat = wxString(myConfig::getConfig().getValueOrDefault("fileformat","pts = [ %s ];"));
			wxString filestring = wxString::Format(fileformat, poly->getPointString());
			wxFile savefile(file.GetFullPath(), wxFile::write);
			if (savefile.IsOpened()) {
				savefile.Write(filestring);
				savefile.Close();
				poly->setModified(false);
				SetTitle(wxString::Format("wxpolygon: %s", file.GetFullPath()));
				SetStatusText(wxString::Format("%d points written to %s.",(int) poly->getNumberOfPoints(), file.GetFullPath()));
			}
			else wxMessageBox(wxString::Format("File save to %s failed.",file.GetFullPath()));
		}
	}
	
	void OnLoadImage(wxCommandEvent& WXUNUSED(event))
	{
		wxFileName f = wxFileSelector("Specify an image file:", file.GetPath(), "", "", wxFileSelectorDefaultWildcardStr, wxFD_OPEN);
		if (f.FileExists()) {
			wxImage img(f.GetFullPath());
			poly->setImage(img);
		}
	}
	
	void OnProperties(wxCommandEvent& WXUNUSED(event))
	{
		if (configfile == "(none)") {
			wxMessageBox(_("No configuration file found."));
			return;
		}
		if (propdiag == nullptr) {
			propdiag = new PropertyDialog(this, wxID_ANY, _("Properties"), wxDefaultPosition, wxSize(640,480));
			propdiag->LoadConfig();
			Bind(wxEVT_PG_CHANGED,&MyFrame::UpdateConfig,this);
		}
		if (propdiag != nullptr) {
			propdiag->ClearModifiedStatus();
			propdiag->Show();
		}
		else {
			wxMessageBox(_("Failed to create Properties dialog"));
		}
	}
	
	void UpdateConfig(wxPropertyGridEvent& event)
	{
		wxPGProperty * prop = event.GetProperty();
		wxString propname = event.GetPropertyName();
		wxString propval;

		propval = event.GetPropertyValue().GetString();

		SetStatusText(wxString::Format(_("Changed %s to %s."), propname, propval));
		myConfig::getConfig().setValue((const char  *) propname.mb_str(), (const char  *) propval.mb_str());
		if (!myConfig::getConfig().flush()) SetStatusText(_("Write to configuration file failed."));
		if (myConfig::getConfig().getValueOrDefault("polyround","0") == "1") 
			SetStatusText(_("polyround"),2);
		else
			SetStatusText(_(""),2);

		poly->Refresh();
	}
	
	void setConfigFile(wxString conf)
	{
		configfile = conf;
	}
	
	void OnClose(wxCloseEvent& event)
	{
		if ( event.CanVeto() && poly->isModified())
		{
			if ( wxMessageBox("Point list is modified.  Quit program?",
							"Confirm",
							wxICON_QUESTION | wxYES_NO) != wxYES )
			{
				event.Veto();
				return;
			}
		}
		Destroy();  
	}
	
	void OnQuit(wxCommandEvent& WXUNUSED(event))
	{
		if (poly->isModified()) {
			int answer = wxMessageBox("Point list is modified.  Quit program?",
							"Confirm",
							wxICON_QUESTION | wxYES_NO);
			if (answer == wxYES)
				Close(true);
			else
				return;
		}
		else Close(true);
	}
	
	void OnCopy(wxCommandEvent& WXUNUSED(event))
	{
		poly->copyToClipboard();
	}
	
	void OnPaste(wxCommandEvent& WXUNUSED(event))
	{
		poly->pasteFromClipboard();
	}
	
	void OnAbout(wxCommandEvent& WXUNUSED(event))
	{
		wxMessageBox(wxString::Format
		(
			"wxPolygon %s\n"
			"\n"
			"This is a program that draws polygons for\n"
			"copy/pasting from/to OpenSCAD arrays.\n\n"
			"%s\n%s.",
			WXPOLYGON_VERSION.c_str(),
			wxVERSION_STRING,
			wxGetOsDescription()
                 ),
                 "About wxPolygon",
                 wxOK | wxICON_INFORMATION,
                 this);
	}

private:

	class myFileDropTarget: public wxFileDropTarget
	{
		public:
			myFileDropTarget(MyFrame *frm) {
				frame = frm;
			}

			bool OnDropFiles (wxCoord x, wxCoord y, const wxArrayString &filenames)
			{
				if (frame) {
					wxFileName f(filenames[0]);
					f.MakeAbsolute();
					wxSetWorkingDirectory (f.GetPath());
					frame->OpenFile(f);
					return true;
				}
				return false;
			}

		private:
			MyFrame *frame;
	};

	wxFileName file;
	myPolyPane *poly;
	PropertyDialog *propdiag;
	wxString configfile;
	bool modified;
	
	pointList* grid; 
	wxAuiManager m_mgr;
	wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(Polygon_Quit,  MyFrame::OnQuit)
    EVT_MENU(Polygon_About, MyFrame::OnAbout)
    EVT_MENU(Polygon_Copy, MyFrame::OnCopy)
    EVT_MENU(Polygon_Paste, MyFrame::OnPaste)
    EVT_MENU(Polygon_Open, MyFrame::OnOpen)
    EVT_MENU(Polygon_Save, MyFrame::OnSave)
    EVT_MENU(Polygon_SaveAs, MyFrame::OnSaveAs)
	EVT_MENU(Polygon_LoadImage, MyFrame::OnLoadImage)
	EVT_MENU(Polygon_Properties, MyFrame::OnProperties)
wxEND_EVENT_TABLE()

static const wxCmdLineEntryDesc cmdLineDesc[] =
{
	{ wxCMD_LINE_PARAM,  "",  "", "file to open", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL},
	{ wxCMD_LINE_NONE }
};

class MyApp : public wxApp
{
public:
	bool OnInit()
	{
		wxFileName configfile("wxpolygon.conf");
		configfile.SetPath(wxString(getExeDir("")));
		if (configfile.FileExists()) { 
			myConfig::loadConfig(configfile.GetFullPath().ToStdString());
		}
		else wxMessageBox(wxString::Format("No configuration file: %s",configfile.GetFullPath()));
		
		MyFrame *frame = new MyFrame("wxPolygon");
		if (configfile.FileExists()) frame->setConfigFile(configfile.GetFullPath());
		
		wxCmdLineParser cmdline(cmdLineDesc, argc, argv);
		cmdline.Parse();
		
		if (cmdline.GetParamCount() > 0) {
			wxFileName f(cmdline.GetParam());
			frame->OpenFile(f);
		}
		
		wxInitAllImageHandlers();

		frame->SetIcon(wxpolygon);
		frame->Show(true);
		return true;
	}
	
	int OnRun()
	{
		wxApp::OnRun();
		return 0;
	}
};

wxIMPLEMENT_APP(MyApp);


