#ifndef __WXIMU_DRIVER_GESTURE_RECOGNIZER_H__
#define __WXIMU_DRIVER_GESTURE_RECOGNIZER_H__

#include <vector>
#include <unordered_map>
#include "driver_serial.h"
#include "driver_thread.h"

namespace imustd{
    class wxDriverSerial;
    
    class wxRecogThread : public wxThread{
        void ToFile(const char *filename);

        public:
        wxRecogThread(wxDriverSerial *parent) : wxThread(wxTHREAD_DETACHED){
            host = parent;
        }
        uint8_t direction = 0;

        protected:
        wxDriverSerial *host;
        virtual ExitCode Entry()=0;
        std::unordered_map<std::string, std::vector<double>> zScoreThresholding(std::vector<double> input);
        double bandStop, bandStopAngle, threshold, influence;
        int lag;
    };

    class wxZPBTThread : public wxRecogThread{
        //void ToFile(const char *filename);

        public:
        wxZPBTThread(wxDriverSerial *parent) : wxRecogThread(parent){//wxThread(wxTHREAD_DETACHED){
            lag = 15;
            threshold = 3.0;
            influence = 0.0;
            bandStop = 0.7;
            bandStopAngle = 500;
        }
        virtual ExitCode Entry();
        //uint8_t direction = 0;
        short int Infinity(double *rawStream, std::vector<double> neg, std::vector<double> pos, double band);
    };

    class wxHMMThread : public wxRecogThread{
        public:
        wxHMMThread(wxDriverSerial *parent) : wxRecogThread(parent){
            lag = 10;
            threshold = 1.0;
            influence = 0;
            bandStop = 0.75;
            bandStopAngle = 500;
        }
        virtual ExitCode Entry();
        short int Infinity(double *rawStream, double neg[2][2], double pos[2][2], double band);
    };
}

#endif