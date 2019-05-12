#ifndef __WXIMU_DRIVER_SERIAL_H__
#define __WXIMU_DRIVER_SERIAL_H__

#include "driver_subsettings.h"
#include "dllfunc.h"
#include "driver_thread.h"

namespace imustd{
    class wxDataBufferThread;
    class wxAPIThread;

    class wxDriverSerial : public wxWindow{
        HINSTANCE driver;
        char *driverVersion, *apiVersion;
        const char *errorString;
        wxDriverSubsetting **elements;
        int elementCount, bufferStart = 0;
        commandDriver driverStarter = NULL, driverStopper = NULL;
        wxButton *driverSwitch = NULL, *dataRecord = NULL;
        wxAPIThread *dataOut;
        wxCriticalSection dataInLocker;

        public:
        wxDriverSerial(wxWindow *parent);
        ~wxDriverSerial();
        void onReset(wxCommandEvent& event);
        void onSet(wxCommandEvent& event);
        void onStart(wxCommandEvent& event);
        void onStop(wxCommandEvent& event);
        void onRecord(wxCommandEvent& event);
        void bufferRefresh();
        double dataBuffers[9][100] = {};
        wxDataBufferThread *dataIn;

        wxDECLARE_EVENT_TABLE();
    };
}

#endif