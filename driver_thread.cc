#include <wx/wxprec.h>

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <windows.h>
#include <algorithm>
#include <iterator>
#include "driver_thread.h"
#include "dllfunc.h"

namespace imustd{
    DWORD unblockPipe = PIPE_READMODE_MESSAGE | PIPE_NOWAIT;
    DWORD blockPipe = PIPE_READMODE_MESSAGE | PIPE_WAIT;
    wxThread::ExitCode wxDataBufferThread::Entry(){
        dataGrabber getFrame = NULL;
        if(driver){
            getFrame = (dataGrabber) GetProcAddress(this->driver, "driver_getFrame");
        }
        while(!TestDestroy()){
            if(getFrame){
                double frame[9];
                if((getFrame)(frame)){
                    wxCriticalSectionLocker lock(bufferLock);
                    for(int counter = 0; counter < 9; counter++){
                        dataBuffers[counter][bufcount] = frame[counter];
                        dataBuffers[counter][bufcount+100] = frame[counter];
                    }
                    bufcount = (bufcount+1) % 100;
                }
            }
        }
        return (wxThread::ExitCode) 0;
    }

    void wxDataBufferThread::getData(double data[9][100]){
        wxCriticalSectionLocker lock(bufferLock);
        for(int count = 0; count < 9; count++){
            memcpy(&data[count], &dataBuffers[count][bufcount+1], 100*sizeof(double));
        }
    }

    void wxDataBufferThread::clearData(){
        wxCriticalSectionLocker lock(bufferLock);
        memset(dataBuffers, 0, sizeof(dataBuffers));
        //std::fill(std::begin(dataBuffers[2]), std::end(dataBuffers[2]), 0.7);
    }

    wxThread::ExitCode wxAPIThread::Entry(){
        bool isConnected = false;
        bool isAuthenticated = false;
        char buffer[420];
        recog = new wxZPBTThread(caller);
        if(recog->Run() != wxTHREAD_NO_ERROR){
           wxMessageBox("Cannot start recognizer");
           return (wxThread::ExitCode) 0; 
        }
        fifo = CreateNamedPipe(TEXT("\\\\.\\pipe\\wxImuApi"), PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT,
            PIPE_UNLIMITED_INSTANCES, 512, 512, 0, NULL);
        while(!TestDestroy()){
            if(isConnected){
                DWORD readSize;
                bool writeRet = ReadFile(fifo, buffer, sizeof(buffer), &readSize, NULL);
                if(!writeRet || readSize == 0){
                    if(GetLastError() == ERROR_BROKEN_PIPE){
                        isConnected = false;
                        FlushFileBuffers(fifo);
                        DisconnectNamedPipe(fifo);
                    }
                }
                else{
                    if(!isAuthenticated){
                        if(!strcmp("imuapi1.0", buffer))
                            isAuthenticated = true;
                            WriteFile(fifo, "imuapi1.0", 10, &readSize, NULL);
                    }
                    else{
                        if(!strcmp("apiclose", buffer)){
                            isConnected = false;
                            WriteFile(fifo, "exit", 5, &readSize, NULL);
                            FlushFileBuffers(fifo);
                            DisconnectNamedPipe(fifo);
                        }
                        else if(!strcmp("apidetect", buffer)){
                            uint8_t dir = recog->direction;
                            //recog->direction = 0;
                            WriteFile(fifo, (void*) &dir, 1, &readSize, NULL);
                            recog->direction = 0;
                        }
                    }
                }
            }
            else if(fifo != INVALID_HANDLE_VALUE){
                isConnected = ConnectNamedPipe(fifo, NULL) ? true : (GetLastError() == ERROR_PIPE_CONNECTED);
            }
        }
        recog->Delete();

        CloseHandle(fifo);

        return (wxThread::ExitCode) 0;
    }
}