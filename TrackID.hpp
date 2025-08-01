
#ifndef TRACK_H
#define TRACK_H

#include <iostream>
#include <vector>
#include <complex>
#include <thread>
#include <future>
#include <cmath>

#include "opencv2/opencv.hpp"


class Track {
    private:
        int ID;
        cv::Rect2d bbox;
        cv::Rect2d searchArea;
        cv::Rect2d imageBounds;
        double w; 
        double h;
        double bw;
        double bh;

    public:
        Track () {
            ID = 0;
            bbox = cv::Rect2d(0, 0, 0, 0);
            searchArea = cv::Rect2d(0, 0, 0, 0);
        };
        void initBBox(cv::Mat frame);

        cv::Rect2d getBBox();
        cv::Rect2d getSearchArea(); 
        void updateBBox(int dx, int dy, cv::Rect2d bounds);
       
        cv::Mat cropForSearch(cv::Mat frame);

        cv::Mat cropForROI(cv::Mat frame);

};


#endif // TRACK_H