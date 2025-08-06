#include "TrackID.hpp"

using namespace cv;
using namespace std;

void Track::initBBox(cv::Mat frame) {

    bool fromCenter = false;
    int rows = frame.rows;
    int cols = frame.cols;
    this->imageBounds = Rect(0, 0, cols, rows);

    this->bbox = selectROI(frame);
    this->displayBbox = this->bbox;

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

    //this->searchArea &= this->imageBounds;
}

Rect Track::getBBox() {return this->bbox;}

Rect Track::getDisplayBBox() {return this->displayBbox;}

Rect Track::getSearchArea() {return this->searchArea;}

void Track::updateBBox(int dx, int dy, cv::Rect bounds) {
    
    this->bbox.x += dx;
    this->bbox.y += dy;
    
    this->searchArea.x += dx;
    this->searchArea.y += dy;

    this->displayBbox = this->bbox & this->imageBounds;
}

Mat Track::cropForSearch(Mat frame) {

    Rect tmp = this->searchArea;
    tmp &= this->imageBounds;
    Mat cropped = frame(tmp);


    int left = (0 < -this->searchArea.x) ? -this->searchArea.x : 0;
    int top = (0 < -this->searchArea.y) ? -this->searchArea.y : 0;
    int right = (0 < this->searchArea.x + this->searchArea.width   - this->imageBounds.width) 
                    ? this->searchArea.x + this->searchArea.width   - this->imageBounds.width : 0;
    int bottom = (0 < this->searchArea.y + this->searchArea.height  - this->imageBounds.height) 
                    ? this->searchArea.y + this->searchArea.height   - this->imageBounds.height : 0;
    if (top > 0 || bottom > 0 || left > 0 || right > 0) {
        copyMakeBorder(cropped, cropped, top, bottom, left, right, BORDER_REPLICATE);
    }

    return cropped;
}

Mat Track::cropForROI(Mat frame) {
    return frame(this->bbox);
}


