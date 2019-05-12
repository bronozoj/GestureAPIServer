#include <wx/wxprec.h>

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include "dllfunc.h"
#include "driver_subsettings.h"

namespace imustd{
    wxDriverSubsetting::wxDriverSubsetting(wxWindow *parent, settingValues getter, settingValues setter)
    : wxWindow(parent, wxID_ANY){
        this->get = getter;
        this->set = setter;
    }

    wxStringSubsetting::wxStringSubsetting(wxWindow *parent, const char *name, settingValues getter, settingValues setter)
    : wxDriverSubsetting(parent, getter, setter){
        char buffer[150];
        wxSizer *grid = new wxBoxSizer(wxHORIZONTAL);
        this->name = wxString(name);
        if((getter) ((char *)name, buffer)){
            this->data = new wxTextCtrl(this, wxID_ANY, buffer);
            grid->Add(new wxStaticText(this, wxID_ANY, wxString(name) << ": "));
            grid->Add(this->data);
        }
        else{
            grid->Add(new wxStaticText(this, wxID_ANY, wxString("Invalid setting ") + wxString(name)));
        }
        SetSizer(grid);
    }

    void wxStringSubsetting::Reset(){
        char buffer[150];
        if((this->get) ((char *)name.mb_str().data(), (void*) buffer)){
            data->Clear();
            (*data) << buffer;
        }
    }

    void wxStringSubsetting::Set(){
        (this->set)((char *)name.mb_str().data(), (void*) data->GetLineText(0).mb_str().data());
    }

    wxNumberSubsetting::wxNumberSubsetting(wxWindow *parent, const char *name, settingValues getter, settingValues setter)
    : wxDriverSubsetting(parent, getter, setter){
        uint32_t buffer;
        wxSizer *grid = new wxBoxSizer(wxHORIZONTAL);
        this->name = wxString(name);
        if((getter)((char*)name, &buffer)){
            this->data = new wxSpinCtrl(this, wxID_ANY, wxString() << buffer);
            this->data->SetRange(0, 115200);
            grid->Add(new wxStaticText(this, wxID_ANY, wxString(name) << ": "));
            grid->Add(this->data);
        }
        else{
            grid->Add(new wxStaticText(this, wxID_ANY, wxString("Invalid setting ") + wxString(name)));
        }
        SetSizer(grid);
    }

    void wxNumberSubsetting::Reset(){
        uint32_t buffer;
        if((get) ((char*)name.mb_str().data(), (void *) &buffer))
            data->SetValue(buffer);
    }

    void wxNumberSubsetting::Set(){
        uint32_t buffer = data->GetValue();
        (set) ((char *) name.mb_str().data(), (void*) &buffer);
    }
}