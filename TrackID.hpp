
#ifndef TRACK_H
#define TRACK_H


#include "CFT_Track.hpp"
#include "opencv2/opencv.hpp"


class Track : public CFT {
    private:
        int ID;
        cv::Rect bbox;
        cv::Rect searchArea;
        cv::Rect displayBbox;
        cv::Rect imageBounds;
        //double w; 
        //double h;
        //double bw;
        //double bh;

    public:

        Track () {
            ID = 0;
            bbox = cv::Rect(0, 0, 0, 0);
            searchArea = cv::Rect(0, 0, 0, 0);
        };

        void initBBox(cv::Mat frame);
        void updateBBox(int dx, int dy, cv::Rect bounds);

        inline cv::Rect getBBox() const {return this->bbox;}

        inline cv::Rect getDisplayBBox() const {return this->displayBbox;}

        inline cv::Rect getSearchArea() const {return this->searchArea;}

        inline cv::Rect getImageBounds() const {return this->imageBounds;}
       
        cv::Mat cropForSearch(cv::Mat frame);
        cv::Mat cropForROI(cv::Mat frame);

        void updateFilter(double lr);

        cv::Mat Ai, Bi, G, Gi, fi, Hi;
        bool psrFlag = false;

};


#endif // TRACK_H