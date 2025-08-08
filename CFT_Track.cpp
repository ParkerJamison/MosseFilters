#include "CFT_Track.hpp"
#include "TrackID.hpp"
#include <iostream>
#include <cmath>
#include <random>

using namespace cv;
using namespace std;


void fft2d(Mat& frame) {
    Mat planes[] = {Mat_<double>(frame), Mat::zeros((frame).size(), CV_64F)};
    Mat complexI;
    merge(planes, 2, complexI);

    dft(complexI, complexI, DFT_COMPLEX_OUTPUT);
    //"return" the fourier domain image
    frame = complexI;
}

void preProcess(cv::Mat &img, cv::Mat &window) {

    Mat hanningWindow(int height, int width);

    int width = (img).cols;
    int height = (img).rows;

    img.convertTo(img, CV_64F, 1.0/255);
    (img) += 1e-5;
    
    log((img), (img));
    Scalar m, sd;
    cv::meanStdDev((img), m, sd);

    (img) = ((img) - m[0]) / (sd[0] + 0.01);

    mulSpectrums(img, window, img, 0, false);
    return;
}

Mat hanningWindow(int height, int width) {
    Mat win;
    createHanningWindow(win, Size(width, height), CV_64F);
    return win;
}

Mat guass(cv::Mat initFrame, cv::Rect2d rect, int sigma) {
    int width = initFrame.cols;
    int height = initFrame.rows;

    int centerX = rect.x + (0.5 * rect.width);
    int centerY = rect.y + (0.5 * rect.height);

    Mat response = Mat::zeros(height, width, CV_64F);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            float distance = (pow((j - centerX), 2) + pow(i - centerY, 2)) / (2 * sigma);
            double ex = exp(-distance);
            response.at<double>(i, j) = ex;
        }
    }

    double imMax;
    double imMin;
    minMaxLoc(response, &imMin, &imMax, NULL, NULL);
    normalize(response, response, 0, 1, NORM_MINMAX);

    return response;
}

Mat randomWarp(Mat img) {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> angle_dis(-11.25, 11.25);
    double angle = angle_dis(gen);
    Point2f center(img.cols/2.0F, img.rows/2.0F);
    Mat rotMat = getRotationMatrix2D(center, angle, 1.0);
    Mat warped;
    warpAffine(img, warped, rotMat, img.size(), INTER_LINEAR, BORDER_REFLECT);

    uniform_real_distribution<> trans_dis(-8.0, 8.0);
    float tx = trans_dis(gen), ty = trans_dis(gen);
    Mat warpMat = (Mat_<double>(2,3) << 1, 0, tx, 0, 1, ty);
    warpAffine(warped, warped, warpMat, img.size(), INTER_LINEAR, BORDER_REFLECT);

    return warped;
}


void trainFilter(cv::Mat initFrame, cv::Mat& G, cv::Mat& Ai, cv::Mat& Bi,
                    int numTrain, double lr) {

    int height = (G).rows;
    int width = (G).cols;

    Mat window = hanningWindow(height, width);

    Mat fi = initFrame;
    //resize(initFrame, fi, Size(width, height));

    preProcess(fi, window);
    fft2d(fi);
    
    mulSpectrums(G, fi, Ai, 0, true);
    mulSpectrums(fi, fi, Bi, 0, true);

    Mat warped(Size(width, height), fi.type()), tmp1(Ai.size(), Ai.type()), tmp2(Bi.size(), Bi.type());
    for (int i = 0; i < numTrain; i++) {
        warped = randomWarp(initFrame);

        //resize(warped, warped, Size(width, height));
        preProcess(warped, window);
        fi = warped;
        fft2d(fi);

        mulSpectrums(G, fi, tmp1, 0, true);
        mulSpectrums(fi, fi, tmp2, 0, true);

        Ai = (1-lr) * Ai + (lr) * tmp1;
        Bi = (1-lr) * Bi + (lr) * tmp2;
    }
}
    
Track CFT::initTracking(Mat frame) {

    Track track;
    track.initBBox(frame);
    
    Mat grayFrame;
    cvtColor(frame, grayFrame, COLOR_BGR2GRAY);
    grayFrame.convertTo(grayFrame, CV_64F);

    Mat response = guass(grayFrame, track.getSearchArea(), this->sigma);
    Mat window = hanningWindow((response.rows), (response.cols)); 
    preProcess(response, window);
    
    track.G = track.cropForSearch(response);
    track.fi = track.cropForROI(grayFrame);

    fft2d(track.G);
    resize(track.fi, track.fi, track.G.size());

    trainFilter(track.fi, track.G, track.Ai, track.Bi, this->numTrain, this->lr);

    return track;
}

int CFT::updateTracking(cv::Mat frame, Track &track) {

    // change later
    Mat window = hanningWindow((track.G.rows), (track.G.cols));   
    Mat mask = Mat::ones(track.Gi.size(), CV_8U);

    
    cvtColor(frame, frame, COLOR_BGR2GRAY);

    divSpectrums(track.Ai, track.Bi, track.Hi, 0, false);

    track.fi = track.cropForSearch(frame);

    preProcess(track.fi, window);
        //cout << "after" << endl;
    fft2d(track.fi);

        //cout << "here1" << endl;
    mulSpectrums(track.Hi, track.fi, track.Gi, 0, false);

    idft(track.Gi, track.Gi, DFT_REAL_OUTPUT | DFT_SCALE);

    double maxVal;
    Point maxLoc;
    minMaxLoc(track.Gi, NULL, &maxVal, NULL, &maxLoc);
    Rect peakWindow = Rect(maxLoc.x - 20, maxLoc.y - 20, 41, 41);
    peakWindow &= track.getImageBounds();
    mask(peakWindow) = 0;
    Scalar m, sd;
    meanStdDev(track.Gi, m, sd, mask);
    double psr = (maxVal - m[0]) / sd[0];
    mask(peakWindow) = 1;
    cout << "PSR::::::" << psr << endl;
    int dx = int(maxLoc.x - track.Gi.cols / 2);
    int dy = int(maxLoc.y - track.Gi.rows / 2);
    track.updateBBox(dx, dy, track.getImageBounds());

    if (!track.psrFlag) {
            track.updateFilter(this->lr);
            if (psr > 20) track.psrFlag = true;
        }
        else {
            if (psr > 12) 
                track.updateFilter(this->lr);

            else {
                cout << "PSR too low to update" << endl;
            }
        }
}


