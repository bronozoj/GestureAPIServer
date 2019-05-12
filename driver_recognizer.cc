#include <wx/wxprec.h>

#ifndef WX_PRECOMP
    #include <wx/wx.h>
    #include <wx/utils.h>
    #include <wx/ffile.h>
#endif

//#include <vector>
//#include <algorithm>
//#include <unordered_map>
//#include <cmath>
//#include <iterator>
#include <numeric>
//#include <cstring>
//#include <iomanip>
//#include <algorithm>
//#include <numeric>
#include "driver_recognizer.h"

#define GESTURE_NONE 0
#define GESTURE_CLOCKWISE 1
#define GESTURE_COUNTERCLOCKWISE 2
#define GESTURE_UP 3
#define GESTURE_DOWN 4
#define GESTURE_LEFT 5
#define GESTURE_RIGHT 6
#define GESTURE_FORWARD 7
#define GESTURE_BACKWARD 8

using namespace std;

namespace imustd{
    class VectorStats {
    public:
        /**
         * Constructor for VectorStats class.
         *
         * @param start - This is the iterator position of the start of the window,
         * @param end   - This is the iterator position of the end of the window,
         */
        VectorStats(vector<double>::iterator start, vector<double>::iterator end) {
            this->start = start;
            this->end = end;
            this->compute();
        }

        /**
         * This method calculates the mean and standard deviation using STL function.
         * This is the Two-Pass implementation of the Mean & Variance calculation.
         */
        void compute() {
            double sum = std::accumulate(start, end, 0.0);
            unsigned int slice_size = std::distance(start, end);
            double mean = sum / slice_size;
            vector<double> diff(slice_size);
            std::transform(start, end, diff.begin(), [mean](double x) { return x - mean; });
            double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
            double std_dev = std::sqrt(sq_sum / slice_size);

            this->m1 = mean;
            this->m2 = std_dev;
        }

        double mean() {
            return m1;
        }

        double standard_deviation() {
            return m2;
        }

    private:
        vector<double>::iterator start;
        vector<double>::iterator end;
        double m1;
        double m2;
    };

    wxThread::ExitCode wxZPBTThread::Entry(){
        while(!TestDestroy()){
            int value;
            //wxString fileName;
            host->bufferRefresh();
            value = Infinity(host->dataBuffers[4], {-1,-1,-1,-1}, {1,1,1,1}, bandStopAngle);
            if(value > 0){
                direction = GESTURE_CLOCKWISE;
                host->dataIn->clearData();
                //fileName.Printf("r_cw%04d.nou", icw++);
                //ToFile(fileName.mb_str());
            }
            else if(value < 0){
                direction = GESTURE_COUNTERCLOCKWISE;
                host->dataIn->clearData();
                //fileName.Printf("r_ccw%04d.nou", iccw++);
                //ToFile(fileName.mb_str());
            }
            else{
                value = Infinity(host->dataBuffers[2], {-1,-1,-1,-1,-1,1,1,1,1,1}, {1,1,1,1,1,-1,-1,-1,-1,-1}, bandStop);
                if(value > 0){
                    direction = GESTURE_UP;
                    host->dataIn->clearData();
                    //fileName.Printf("r_up%04d.nou", iup++);
                    //ToFile(fileName.mb_str());
                }
                else if(value < 0){
                    direction = GESTURE_DOWN;
                    host->dataIn->clearData();
                    //fileName.Printf("r_down%04d.nou", idown++);
                    //ToFile(fileName.mb_str());
                }
                else{
                    value = Infinity(host->dataBuffers[0], {-1,-1,-1,-1,-1,1,1,1,1,1}, {1,1,1,1,1,-1,-1,-1,-1,-1}, bandStop);
                    if(value < 0){
                        direction = GESTURE_LEFT;
                        host->dataIn->clearData();
                        //fileName.Printf("r_left%04d.nou", ileft++);
                        //ToFile(fileName.mb_str());
                    }
                    else if(value > 0){
                        direction = GESTURE_RIGHT;
                        host->dataIn->clearData();
                        //fileName.Printf("r_right%04d.nou", iright++);
                        //ToFile(fileName.mb_str());
                    }
                    else{
                        value = Infinity(host->dataBuffers[1], {-1,-1,-1,-1,1,1,1,1}, {1,1,1,1,-1,-1,-1,-1}, bandStop);
                        if(value > 0){
                            direction = GESTURE_FORWARD;
                            host->dataIn->clearData();
                        }
                        else if(value < 0){
                            direction = GESTURE_BACKWARD;
                            host->dataIn->clearData();
                        }
                        else{
                            direction = 0;
                            wxMilliSleep(20);
                            continue;
                        }
                    }
                }
            }
            wxMilliSleep(500);
        }

        return (wxThread::ExitCode) 0;
    }

    void wxZPBTThread::ToFile(const char *filename){
        wxFFile outFile(filename, "w");
        wxString outData;
        for(int count = 0; count < 100; ++count){
            outData.Printf("%lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf\n",
                host->dataBuffers[0][count], host->dataBuffers[1][count], host->dataBuffers[2][count],
                host->dataBuffers[3][count], host->dataBuffers[4][count], host->dataBuffers[5][count],
                host->dataBuffers[6][count], host->dataBuffers[7][count], host->dataBuffers[8][count]);
            outFile.Write(outData);
        }
    }

    short int wxZPBTThread::Infinity(double *rawStream, vector<double> neg, vector<double>pos, double band){
        double upperbound = rawStream[0] + band;
        double lowerbound = rawStream[0] - band;

        for(int i = 0; i < 100; ++i){
            if((lowerbound < rawStream[i]) && (rawStream[i] < upperbound))
                rawStream[i] = 0;
        }

        vector<double> dataStream(rawStream, rawStream + 100);
        unordered_map<string, vector<double>> output = zScoreThresholding(dataStream);

        output["signals"].erase(remove(output["signals"].begin(), output["signals"].end(), 0), output["signals"].end());
        output["signals"].shrink_to_fit();

        int sumOfElems = 0;
        for(auto& n: output["signals"]){
            sumOfElems += n;
        }

        auto res = search(begin(output["signals"]), end(output["signals"]), begin(pos), end(pos));
        auto found = res != end(output["signals"]);

        if(found == 1 && (-60 < sumOfElems && sumOfElems < 60) && (output["signals"][1] == 1))
            return 1;
        
        res = search(begin(output["signals"]), end(output["signals"]), begin(neg), end(neg));
        found = res != end(output["signals"]);

        if(found == 1 && (-60 < sumOfElems && sumOfElems < 60) && (output["signals"][1] == -1))
            return -1;

        return 0;
    }

    unordered_map<string, vector<double>> wxZPBTThread::zScoreThresholding(vector<double> input){
        unordered_map<string, vector<double>> output;

        unsigned int n = (unsigned int) input.size();
        vector<double> signals(input.size());
        vector<double> filtered_input(input.begin(), input.end());
        vector<double> filtered_mean(input.size());
        vector<double> filtered_stddev(input.size());

        VectorStats lag_subvector_stats(input.begin(), input.begin() + lag);
        filtered_mean[lag-1] = lag_subvector_stats.mean();
        filtered_stddev[lag-1] = lag_subvector_stats.standard_deviation();

        for(int i = lag; i < n; i++){
            if(abs(input[i] - filtered_mean[i-1]) > threshold * filtered_stddev[i-1]){
                signals[i] = (input[i] > filtered_mean[i-1]) ? 1.0 : -1.0;
                filtered_input[i] = influence * input[i] + (1-influence) * filtered_input[i-1];
            }
            else{
                signals[i] = 0.0;
                filtered_input[i] = input[i];
            }
            VectorStats lag_subvector_stats(filtered_input.begin() + (i-lag), filtered_input.begin() + i);
            filtered_mean[i] = lag_subvector_stats.mean();
            filtered_stddev[i] = lag_subvector_stats.standard_deviation();
        }

        output["signals"] = signals;
        output["filtered_mean"] = filtered_mean;
        output["filtered_stddev"] = filtered_stddev;

        return output;
    }
}