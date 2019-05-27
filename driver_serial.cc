#include <wx/wxprec.h>

#ifndef WX_PRECOMP
    #include <wx/wx.h>
    #include <wx/ffile.h>
#endif

#include "dllfunc.h"
#include "driver_serial.h"

namespace imustd{
    wxBEGIN_EVENT_TABLE(wxDriverSerial, wxWindow)
    wxEND_EVENT_TABLE()

    wxDriverSerial::wxDriverSerial(wxWindow *parent) : wxWindow(parent, wxID_ANY){
        const char *outString;
        versionString stringLoader;

        this->driver = LoadLibrary(TEXT("imudriver.dll"));
        if(driver == NULL){
            wxMessageBox("Cannot load driver");
            return;
        }

        stringLoader = (versionString) GetProcAddress(this->driver, "driver_target");
        if(stringLoader == NULL){
            FreeLibrary(this->driver);
            this->driver = NULL;
            this->errorString = "Unknown target version";
            return;
        }
        outString = (stringLoader) ();
        this->apiVersion = new char[strlen(outString) + 2];
        strcpy(this->apiVersion, outString);

        stringLoader = (versionString) GetProcAddress(this->driver, "driver_version");
        if(stringLoader != NULL){
            outString = (stringLoader) ();
            this->driverVersion = new char[strlen(outString) + 2];
            strcpy(this->driverVersion, outString);
        }
        else{
            this->driverVersion = new char[3];
            strcpy(this->driverVersion, "");
        }

        wxSizer *grid = new wxStaticBoxSizer(wxVERTICAL, this, wxString("Inertial Measurement Unit Driver v") + wxString(this->driverVersion));
        grid->Add(new wxStaticText(this, wxID_ANY, wxString("Target API version: ") + wxString(this->apiVersion)));
        grid->Add(new wxStaticText(this, wxID_ANY, wxString("Driver version: ") + wxString(this->driverVersion)));

        getSettings settingsEnumerator = (getSettings) GetProcAddress(this->driver, "driver_init_preferences");
        settingValues getter = (settingValues) GetProcAddress(this->driver, "driver_setting_read");
        settingValues setter = (settingValues) GetProcAddress(this->driver, "driver_setting_write");
        wxWindow *pref = new wxWindow(this, wxID_ANY);
        wxSizer *prefGrid = new wxStaticBoxSizer(wxVERTICAL, pref, "Initialization Settings");
        if(settingsEnumerator != NULL && getter != NULL && setter != NULL){
            driver_attribute **items;
            int total = (settingsEnumerator)(&items);
            elementCount = total;
            elements = new wxDriverSubsetting*[total];

            //prefGrid->Add(new wxStaticText(pref, wxID_ANY, wxString("Time to call ") << total));
            if(!total){
                prefGrid->Add(new wxStaticText(pref, wxID_ANY, wxT("no preferences to configure")));
            }
            for(int count = 0; count < total; count++){
                if(!strcmp((items[count])->type, "string")){
                    //prefGrid->Add(new wxStaticText(pref, wxID_ANY, wxString("string setting for ") << (items[count])->name));
                    elements[count] = new wxStringSubsetting(pref, (items[count])->name, getter, setter);
                    prefGrid->Add(elements[count]);
                }
                else if(!strncmp((items[count])->type, "uint", 4)){
                    //prefGrid->Add(new wxStaticText(pref, wxID_ANY, wxString("uint setting for ") << (items[count])->name));
                    elements[count] = new wxNumberSubsetting(pref, (items[count])->name, getter, setter);
                    prefGrid->Add(elements[count]);
                }
                else{
                    prefGrid->Add(new wxStaticText(pref, wxID_ANY, wxString("No setting for ") << (items[count])->name));
                    elements[count] = NULL;
                }
            }
            wxWindow *buttonHolder = new wxWindow(pref, wxID_ANY);
            wxSizer *holderGrid = new wxBoxSizer(wxHORIZONTAL);
            wxButton *resetButton = new wxButton(buttonHolder, wxID_ANY, "Reset");
            wxButton *setButton = new wxButton(buttonHolder, wxID_ANY, "Set");
            resetButton->Bind(wxEVT_BUTTON, &imustd::wxDriverSerial::onReset, this);
            setButton->Bind(wxEVT_BUTTON, &imustd::wxDriverSerial::onSet, this);
            holderGrid->Add(resetButton);
            holderGrid->Add(setButton);
            buttonHolder->SetSizer(holderGrid);
            grid->Add(pref);
            prefGrid->Add(buttonHolder);
        }
        else{
            grid->Add(new wxStaticText(this, wxID_ANY, "Error with driver dll: missing functions"));
            grid->Add(pref);
        }

        dataRecord = new wxButton(this, wxID_ANY, "Record");
        dataRecord->Bind(wxEVT_BUTTON, &imustd::wxDriverSerial::onRecord, this);
        dataRecord->Enable(false);
        grid->Add(dataRecord);

        driverStarter = (commandDriver) GetProcAddress(this->driver, "driver_initialize");
        driverStopper = (commandDriver) GetProcAddress(this->driver, "driver_free");
        if(driverStarter != NULL && driverStopper != NULL){
            driverSwitch = new wxButton(this, wxID_ANY, "Connect");
            driverSwitch->Bind(wxEVT_BUTTON, &imustd::wxDriverSerial::onStart, this);
            grid->Add(driverSwitch);
        }
        else{
            driverSwitch = new wxButton(this, wxID_ANY, "Disabled");
            driverSwitch->Enable(false);
            grid->Add(driverSwitch);
        }
        pref->SetSizer(prefGrid);
        SetSizer(grid);
    }

    wxDriverSerial::~wxDriverSerial(){
        if(this->driverSwitch != NULL && this->driverSwitch->GetLabel() == "Disconnect"){
            (driverStopper)();
        }
        if(this->dataIn != NULL){
            delete dataIn;
        }
        if(this->apiVersion != NULL)
            delete[] this->apiVersion;
        if(this->driverVersion != NULL)
            delete[] this->driverVersion;
        if(this->driver != NULL)
            FreeLibrary(this->driver);
        if(this->elements != NULL)
            delete[] this->elements;
    }

    void wxDriverSerial::onReset(wxCommandEvent& WXUNUSED(event)){
        for(int count = 0; count < elementCount; count++){
            if(elements[count] != NULL){
                elements[count]->Reset();
            }
        }
    }

    void wxDriverSerial::onSet(wxCommandEvent& WXUNUSED(event)){
        for(int count = 0; count < elementCount; count++){
            if(elements[count] != NULL){
                elements[count]->Set();
            }
        }
    }

    void wxDriverSerial::onStart(wxCommandEvent& WXUNUSED(event)){
        if( (driverStarter)() == true){
            dataIn = new wxDataBufferThread(this, driver);
            if(dataIn->Run() == wxTHREAD_NO_ERROR){
                dataOut = new wxAPIThread(this);
                if(dataOut->Run() == wxTHREAD_NO_ERROR){
                    driverSwitch->Unbind(wxEVT_BUTTON, &imustd::wxDriverSerial::onStart, this);
                    driverSwitch->Bind(wxEVT_BUTTON, &imustd::wxDriverSerial::onStop, this);
                    driverSwitch->SetLabel("Disconnect");
                    dataRecord->Enable(true);
                    wxMessageBox("started");
                }
                else{
                    wxMessageBox("Cannot start api service");
                    dataIn->Delete(NULL, wxTHREAD_WAIT_BLOCK);
                    (driverStopper)();
                }
            }
            else{
                wxMessageBox("Cannot create worker thread");
                (driverStopper)();
            }
        }
        else{
            int errorValue = -1;
            getErrno driverError = (getErrno) GetProcAddress(this->driver, "driver_errno");
            if(driverError)
                errorValue = (driverError)();
            wxString box;
            box.Printf("Cannot connect to device: (%d)", errorValue);
            wxMessageBox(box);
        }
    }

    void wxDriverSerial::onStop(wxCommandEvent& WXUNUSED(event)){
        if(!(driverStopper)()){
            wxMessageBox("error stopping imu driver");
            return;
        }
        dataOut->Delete();
        dataIn->Delete();
        driverSwitch->Unbind(wxEVT_BUTTON, &imustd::wxDriverSerial::onStop, this);
        driverSwitch->Bind(wxEVT_BUTTON, &imustd::wxDriverSerial::onStart, this);
        driverSwitch->SetLabel("Connect");
        dataRecord->Enable(false);
        wxMessageBox("stopped");
    }

    void wxDriverSerial::onRecord(wxCommandEvent& WXUNUSED(event)){
        double tempbuffers[9][100];
        wxString box;
        dataIn->getData(tempbuffers);
        wxFFile outFile("./imuout.txt", "w");
        for(int count = 0; count < 100; count++){
            box.Printf("%lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf\n",
                tempbuffers[0][count], tempbuffers[1][count], tempbuffers[2][count],
                tempbuffers[3][count], tempbuffers[4][count], tempbuffers[5][count],
                tempbuffers[6][count], tempbuffers[7][count], tempbuffers[8][count]);
            outFile.Write(box);
        }
        wxMessageBox("output saved to ./imuout.txt");
    }

    void wxDriverSerial::bufferRefresh(){
        dataIn->getData(dataBuffers);
    }
}