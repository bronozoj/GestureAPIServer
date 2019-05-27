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
#define GESTURE_UPLEFT 9
#define GESTURE_UPRIGHT 10
#define GESTURE_DOWNLEFT 11
#define GESTURE_DOWNRIGHT 12

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
                int value2 = Infinity(host->dataBuffers[0], {-1,-1,-1,-1,-1,1,1,1,1,1}, {1,1,1,1,1,-1,-1,-1,-1,-1}, bandStop);
                if(value > 0){
                    if(value2 > 0)
                        direction = GESTURE_UPRIGHT;
                    else if(value2 < 0)
                        direction = GESTURE_UPLEFT;
                    else
                    direction = GESTURE_UP;
                    host->dataIn->clearData();
                    //fileName.Printf("r_up%04d.nou", iup++);
                    //ToFile(fileName.mb_str());
                }
                else if(value < 0){
                    if(value2 > 0)
                        direction = GESTURE_DOWNRIGHT;
                    else if(value2 < 0)
                        direction = GESTURE_DOWNLEFT;
                    else
                    direction = GESTURE_DOWN;
                    host->dataIn->clearData();
                    //fileName.Printf("r_down%04d.nou", idown++);
                    //ToFile(fileName.mb_str());
                }
                else{
                    //value = Infinity(host->dataBuffers[0], {-1,-1,-1,-1,-1,1,1,1,1,1}, {1,1,1,1,1,-1,-1,-1,-1,-1}, bandStop);
                    if(value2 < 0){
                        direction = GESTURE_LEFT;
                        host->dataIn->clearData();
                        //fileName.Printf("r_left%04d.nou", ileft++);
                        //ToFile(fileName.mb_str());
                    }
                    else if(value2 > 0){
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

    short int wxZPBTThread::Infinity(double *rawStream, vector<double> neg, vector<double>pos, double band){
        double average = 0, upperbound, lowerbound;
        for(int counter = 0; counter < 100; counter++){
            average += rawStream[counter];
        }
        average /= 100;

        upperbound = average + band;
        lowerbound = average - band;

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

    void wxRecogThread::ToFile(const char *filename){
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

    unordered_map<string, vector<double>> wxRecogThread::zScoreThresholding(vector<double> input){
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

    short int wxHMMThread::Infinity(double *rawStream, double neg[2][2], double pos[2][2], double band){
        double average = 0;
        for(int counter = 0; counter < 100; counter++){
            average += rawStream[counter];
        }
        average /= 100;
        double upperbound = average + band;
        double lowerbound = average - band;

        for (int i = 0; i < 100; ++i){
            if ((lowerbound < rawStream[i]) && (rawStream[i] < upperbound)) rawStream[i] = 0;

        }
        
        vector<double> inputX;
        copy(rawStream, rawStream + 100, std::back_inserter(inputX));

        std::unordered_map<std::string, std::vector<double>> outputX = zScoreThresholding(inputX);
        
        outputX["signals"].erase(remove(outputX["signals"].begin(), outputX["signals"].end(), 0), outputX["signals"].end());
        outputX["signals"].shrink_to_fit();

        std::vector <int> intbufX(outputX["signals"].begin(), outputX["signals"].end());
        int *o_x = intbufX.data();

        if (outputX["signals"].size() != 0){
            // HMM BASED RECOG, Compute probabilities
            double prob_pos_x = 1;
            double prob_neg_x = 1;
            int prestate, currstate;
            for(int i = 1; i < 20; ++i){
                if (o_x[i - 1] == 1) prestate = 0;
                else prestate = 1;

                if (o_x[i] == 1) currstate = 0;
                else currstate = 1;

                prob_neg_x *= neg[prestate][currstate];
                prob_pos_x *= pos[prestate][currstate];
            }
            // Compare probabilities
            double no_movement_threshold = pow(0.999999,30);
            if (prob_pos_x > no_movement_threshold || prob_neg_x > no_movement_threshold) return 0;
            else if (prob_pos_x > prob_neg_x) return 1;
            else return -1;

        }

        return 0;
    }

    wxThread::ExitCode wxHMMThread::Entry(){
        double neg[2][2] = {
            {0.999989, 1.10E-05},
            {0.0840672, 0.915933}
        };
        double pos[2][2] = {
            {0.90725, 0.0927499},
            {1.00E-06, 0.999999}
        };
        double neg_rot[2][2] = {
            {0.5, 0.5},
            {1.00E-06, 0.999999}
        };
        double pos_rot[2][2] = {
            {0.999999, 1.00E-06},
            {0.5, 0.5}
        };
        while(!TestDestroy()){
            int value;
            //wxString fileName;
            host->bufferRefresh();
            value = Infinity(host->dataBuffers[4], neg_rot, pos_rot, bandStopAngle);
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
                value = Infinity(host->dataBuffers[2], neg, pos, bandStop);
                int value2 = Infinity(host->dataBuffers[0], neg, pos, bandStop);
                if(value > 0){
                    if(value2 > 0)
                        direction = GESTURE_UPRIGHT;
                    else if(value2 < 0)
                        direction = GESTURE_UPLEFT;
                    else
                    direction = GESTURE_UP;
                    host->dataIn->clearData();
                    //fileName.Printf("r_up%04d.nou", iup++);
                    //ToFile(fileName.mb_str());
                }
                else if(value < 0){
                    if(value2 > 0)
                        direction = GESTURE_DOWNRIGHT;
                    else if(value2 < 0)
                        direction = GESTURE_DOWNLEFT;
                    else
                    direction = GESTURE_DOWN;
                    host->dataIn->clearData();
                    //fileName.Printf("r_down%04d.nou", idown++);
                    //ToFile(fileName.mb_str());
                }
                else{
                    //value = Infinity(host->dataBuffers[0], {-1,-1,-1,-1,-1,1,1,1,1,1}, {1,1,1,1,1,-1,-1,-1,-1,-1}, bandStop);
                    if(value2 < 0){
                        direction = GESTURE_LEFT;
                        host->dataIn->clearData();
                        //fileName.Printf("r_left%04d.nou", ileft++);
                        //ToFile(fileName.mb_str());
                    }
                    else if(value2 > 0){
                        direction = GESTURE_RIGHT;
                        host->dataIn->clearData();
                        //fileName.Printf("r_right%04d.nou", iright++);
                        //ToFile(fileName.mb_str());
                    }
                    else{
                        value = Infinity(host->dataBuffers[1], neg, pos, bandStop);
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
            Sleep(500);
        }

        return (wxThread::ExitCode) 0;
    }
}