#ifndef __WXIMU_DRIVER_GESTURE_RECOGNIZER_H__
#define __WXIMU_DRIVER_GESTURE_RECOGNIZER_H__

#include <vector>
#include <unordered_map>
#include "driver_serial.h"

namespace imustd{
    class wxDriverSerial;
    class wxZPBTThread : public wxThread{
        wxDriverSerial *host;
        double bandStop = 0.7, bandStopAngle=500, threshold = 3.0, influence = 0.0;
        int lag = 15, iup=0, idown=0, ileft=0, iright=0, iccw=0, icw=0;

        std::unordered_map<std::string, std::vector<double>> zScoreThresholding(std::vector<double> input);
        void ToFile(const char *filename);

        public:
        wxZPBTThread(wxDriverSerial *parent) : wxThread(wxTHREAD_DETACHED){
            host = parent;
        }
        virtual ExitCode Entry();
        uint8_t direction = 0;
        short int Infinity(double *rawStream, std::vector<double> neg, std::vector<double> pos, double band);
    };
}

#endif