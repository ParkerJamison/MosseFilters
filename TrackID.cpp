#include "TrackID.hpp"

using namespace cv;
using namespace std;

void Track::initBBox(cv::Mat frame) {

    bool fromCenter = false;
    int rows = frame.rows;
    int cols = frame.cols;
    this->imageBounds = Rect2d(0, 0, cols, rows);

    this->bbox = selectROI(frame);

    int optimalRows = getOptimalDFTSize(this->bbox.height);
    int optimalCols = getOptimalDFTSize(this->bbox.width);

    this->searchArea = this->bbox;

    this->bw = this->bbox.width;
    this->bh = this->bbox.height;

    this->searchArea.x -= (optimalCols - this->bbox.width) / 2;
    this->searchArea.y -= (optimalRows - this->bbox.height) / 2;
    this->searchArea.width = optimalCols;
    this->searchArea.height = optimalRows;
    this->w = optimalCols;
    this->h = optimalRows;

    this->searchArea &= this->imageBounds;
}

Rect2d Track::getBBox() {return this->bbox;}

Rect2d Track::getSearchArea() {return this->searchArea;}

void Track::updateBBox(int dx, int dy, cv::Rect2d bounds) {
    
    this->bbox.x += dx;
    this->bbox.y += dy;
    this->bbox.height = this->bh;
    this->bbox.width = this->bw;
    this->bbox &= bounds;


    this->searchArea.x += dx;
    this->searchArea.y += dy;
    this->searchArea.width = this->w;
    this->searchArea.height = this->h;
    this->searchArea &= bounds;
}

Mat Track::cropForSearch(Mat frame) {return frame(this->searchArea);}

Mat Track::cropForROI(Mat frame) {return frame(this->bbox);}


