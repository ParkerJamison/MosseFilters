
#ifndef CFT_H
#define CFT_H

#include <iostream>
#include <vector>
#include <complex>
#include <thread>
#include <future>
#include <cmath>
#include "time.h"

#include "opencv2/opencv.hpp"

class Track;
class CFT {
    private:
        double lr;
        int sigma;
        int numTrain;
        cv::Mat window;
        cv::Mat mask;

    public:

        CFT() {
            lr = 0.125;
            sigma = 100;
            numTrain = 16;
        };

        CFT(double x, int y, int z) {
            lr = x;
            sigma = y;
            numTrain = z;
        }


        

        Track initTracking(cv::Mat frame);
        int updateTracking(cv::Mat frame, Track &track);
        //Track tttf(cv::Mat frame);
};


#endif // CFT_H