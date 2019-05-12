#ifndef __WXIMU_DRIVER_SUBSETTINGS_H__
#define __WXIMU_DRIVER_SUBSETTINGS_H__

#ifndef WX_PRECOMP
    #include <wx/spinctrl.h>
#endif

#include "dllfunc.h"

namespace imustd{
    class wxDriverSubsetting : public wxWindow{
        protected:
        settingValues get, set;
        wxString name;
        public:
        wxDriverSubsetting(wxWindow *parent, settingValues getter, settingValues setter);
        wxDriverSubsetting(wxWindow *parent, driver_setting_functions function) : wxDriverSubsetting(parent, function.getter, function.setter){};
        virtual void Reset(){};
        virtual void Set(){};
    };

    class wxStringSubsetting : public wxDriverSubsetting{
        wxTextCtrl *data;
        public:
        wxStringSubsetting(wxWindow *parent, const char *name, settingValues getter, settingValues setter);
        wxStringSubsetting(wxWindow *parent, const char *name, driver_setting_functions function) : wxStringSubsetting(parent, name, function.getter, function.setter){};
        void Reset();
        void Set();
    };

    class wxNumberSubsetting : public wxDriverSubsetting{
        wxSpinCtrl *data;
        wxString name;
        public:
        wxNumberSubsetting(wxWindow *parent, const char *name, settingValues getter, settingValues setter);
        wxNumberSubsetting(wxWindow *parent, const char *name, driver_setting_functions function) : wxNumberSubsetting(parent, name, function.getter, function.setter){};
        void Reset();
        void Set();
    };
}

#endif