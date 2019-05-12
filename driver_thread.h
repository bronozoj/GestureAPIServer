#ifndef __WXIMU_DRIVER_THREAD_H__
#define __WXIMU_DRIVER_THREAD_H__

#include "driver_serial.h"
#include "driver_recognizer.h"

namespace imustd{
    class wxDriverSerial;
    class wxZPBTThread;

    class wxDataBufferThread : public wxThread{
        wxDriverSerial *caller;
        HINSTANCE driver;
        double dataBuffers[9][200];

        public:
        wxDataBufferThread(wxDriverSerial *parent, HINSTANCE libAddr) : wxThread(wxTHREAD_DETACHED){
            caller = parent;
            driver = libAddr;
        }
        void getData(double data[9][100]);
        void clearData();
        int bufcount = 0;
        wxCriticalSection bufferLock;

        protected:
        virtual ExitCode Entry();
    };

    class wxAPIThread : public wxThread{
        wxDriverSerial *caller;
        HANDLE fifo;

        public:
        wxAPIThread(wxDriverSerial *parent) : wxThread(wxTHREAD_DETACHED){
            caller = parent;
        }
        wxZPBTThread *recog;

        protected:
        virtual ExitCode Entry();
    };
}

#endif