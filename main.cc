#include <wx/wxprec.h>

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include "driver_serial.h"

class ImuApp: public wxApp{
    public:
        virtual bool OnInit();
};

class ImuFrame: public wxFrame{
    public:
        ImuFrame(const wxString &title, const wxPoint &pos, const wxSize &size);
    
    private:
        void OnExit(wxCommandEvent &event);
    
    wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(ImuFrame, wxFrame)
    EVT_MENU(wxID_EXIT, ImuFrame::OnExit)
wxEND_EVENT_TABLE()

wxIMPLEMENT_APP(ImuApp);

bool ImuApp::OnInit(){
    ImuFrame *frame = new ImuFrame("Gesture Recognition API", wxPoint(50, 50), wxSize(450, 340) );
    frame->Show(true);
    return true;
}

ImuFrame::ImuFrame(const wxString &title, const wxPoint &pos, const wxSize& size)
        : wxFrame(NULL, wxID_ANY, title, pos, size){
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(wxID_EXIT);

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    
    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("API test menu");

    imustd::wxDriverSerial *hwDriver = new imustd::wxDriverSerial(this);
    wxSizer *frameSizer = new wxGridSizer(1);
    frameSizer->Add(hwDriver, wxSizerFlags(0).Expand());
    SetSizerAndFit(frameSizer);
}

void ImuFrame::OnExit(wxCommandEvent &event){
    Close(true);
}