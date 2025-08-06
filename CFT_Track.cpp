#include "CFT_Track.hpp"
#include "TrackID.hpp"
#include <iostream>
#include <cmath>
#include <random>

using namespace cv;
using namespace std;



void fft2d(Mat& frame) {
    //int optimalRows = getOptimalDFTSize((frame).rows);
    //int optimalCols = getOptimalDFTSize((frame).cols);
    //cout << optimalRows << "::" << optimalCols << endl;

    //int Rows = (frame).rows;
    //int Cols = (frame).cols;

    //Mat padded;
    //copyMakeBorder((frame), padded, 0, optimalRows - Rows, 0, optimalCols - Cols, BORDER_CONSTANT, Scalar::all(0));

    Mat planes[] = {Mat_<double>(frame), Mat::zeros((frame).size(), CV_64F)};
    Mat complexI;
    merge(planes, 2, complexI);

    dft(complexI, complexI, DFT_COMPLEX_OUTPUT);

    //"return" the fourier domain image
    //resize(complexI, frame, Size(Cols, Rows));
    frame = complexI;
    return;
    // the rest is just to visualize 
   //split(complexI, planes);
   //magnitude(planes[0], planes[1], planes[0]);
   //Mat magI = planes[0];
   //magI += Scalar::all(1);

   //log(magI, magI);
   //magI = magI(Rect(0, 0, magI.cols & -2, magI.rows & -2));

   //int cx = magI.cols/2;
   //int cy = magI.rows/2;

   //Mat q0(magI, Rect(0, 0, cx, cy));
   //Mat q1(magI, Rect(cx, 0, cx, cy));
   //Mat q2(magI, Rect(0, cy, cx, cy));
   //Mat q3(magI, Rect(cx, cy, cx, cy));

   //Mat tmp;

   //q0.copyTo(tmp);
   //q3.copyTo(q0);
   //tmp.copyTo(q3);

   //q1.copyTo(tmp);
   //q2.copyTo(q1);
   //tmp.copyTo(q2);

   //normalize(magI, magI, 0, 1, NORM_MINMAX);

   //imshow("spectrum", magI);

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

void updateFilter(Mat &G, Mat &Ai, Mat &Bi, Mat &fi, double lr) {

    Mat tmp1, tmp2;

    if (fi.size() != G.size()) {
        cout << "RESIZED" << endl;
        resize(fi, fi, G.size());
    }
    mulSpectrums(G, fi, tmp1, 0, true);
    mulSpectrums(fi, fi, tmp2, 0, true);
    Ai = ((1-lr) * Ai) + ((lr) * tmp1);
    Bi = ((1-lr) * Bi) + ((lr) * tmp2);

}

void CFT::startTracking() {
    Mat currentFrame;//Declaring a matrix to load the frames//
    Mat grayFrame;

    namedWindow("Video Player");//Declaring the video to show the video//
    VideoCapture cap(0);//Declaring an object to capture stream of frames from default camera//

    if (!cap.isOpened()){ //This section prompt an error message if no video stream is found//
        cout << "No video stream detected" << endl;
        system("pause");
        return;
    }
    waitKey(0);
    cap >> currentFrame;
    
    cvtColor(currentFrame, grayFrame, COLOR_BGR2GRAY);
    grayFrame.convertTo(grayFrame, CV_64F);
    Rect imageBounds(0, 0, grayFrame.cols, grayFrame.rows);

    Track test;

    test.initBBox(currentFrame);

    Mat response = guass(grayFrame, test.getSearchArea(), this->sigma);
    Mat window = hanningWindow((response.rows), (response.cols)); 
    preProcess(response, window);
    
    Mat G = test.cropForSearch(response);
    Mat fi = test.cropForROI(grayFrame);

    fft2d(G);
    Size dftSize = G.size();
    if (fi.size() != dftSize) {
        cout << "RESIZED" << dftSize << "::::" << fi.size() << endl;
        resize(fi, fi, dftSize);
    }

    Mat Ai(dftSize, CV_64F);
    Mat Bi(dftSize, CV_64F);
    trainFilter(fi, G, Ai, Bi, this->numTrain, this->lr);
    cout << "here:"<<endl;
    //cout << "Tracking: search fi size = " << searchArea.size() << endl;

    bool psrFlag = false;
    Mat Hi(dftSize, CV_64F);
    Mat Gi(dftSize, CV_64F);
    Mat gi(dftSize, CV_64F);
    Mat mask = Mat::ones(gi.size(), CV_8U);

    window = hanningWindow((dftSize.height), (dftSize.width));   
    int frame = 0;
    time_t startTime = time(NULL);
    //cout << "here:::" << endl;
    while (true) { 
        frame++;
        cap >> currentFrame;
        if (currentFrame.empty()){ //Breaking the loop if no video frame is detected//
            break;
        }

        cvtColor(currentFrame, grayFrame, COLOR_BGR2GRAY);

        divSpectrums(Ai, Bi, Hi, 0, false);

        fi = test.cropForSearch(grayFrame);

        //imshow("test", fi);
        //waitKey(0);
        //cout << fi.rows << "::" << fi.cols << response.rows << "::" << response.cols <<  endl;

        if (fi.size() != dftSize) {
        cout << "RESIZED" << dftSize << "::::" << fi.size() << endl;
            resize(fi, fi, dftSize); 
        }
        preProcess(fi, window);
        //cout << "after" << endl;
        fft2d(fi);

        //cout << "here1" << endl;
        mulSpectrums(Hi, fi, Gi, 0, false);
        //cout << "G: " << G.size() << ", fi: " << fi.size() << ", Hi: " << Hi.size() << ", Gi: " << Gi.size() << ", gi: " << gi.size() << endl;

        idft(Gi, gi, DFT_REAL_OUTPUT | DFT_SCALE);

        double maxVal;
        Point maxLoc;
        minMaxLoc(gi, NULL, &maxVal, NULL, &maxLoc);

        Rect peakWindow = Rect(maxLoc.x - 20, maxLoc.y - 20, 41, 41);
        peakWindow &= imageBounds;
        mask(peakWindow) = 0;
        Scalar m, sd;
        meanStdDev(gi, m, sd, mask);
        double psr = (maxVal - m[0]) / sd[0];
        mask(peakWindow) = 1;
        cout << "PSR::::::" << psr << endl;

        int dx = int(maxLoc.x - gi.cols / 2);
        int dy = int(maxLoc.y - gi.rows / 2);

        test.updateBBox(dx, dy, imageBounds);

        if (!psrFlag) {
            updateFilter(G, Ai, Bi, fi, this->lr);
            if (psr > 20) psrFlag = true;
        }
        else {
            if (psr > 12) 
                updateFilter(G, Ai, Bi, fi, this->lr);

            else {
                cout << "PSR too low to update" << endl;
            }
        }

        rectangle(currentFrame, test.getDisplayBBox(), Scalar(255, 0, 0), 2);
        imshow("Video Player2", currentFrame);
        //waitKey(0);

        char c = (char) waitKey(25);//Allowing 25 milliseconds frame processing time and initiating break condition//
        if (c == 27){ //If 'Esc' is entered break the loop//
            time_t end = time(NULL);
            cout << frame / difftime(end, startTime) << endl;
            break;
        }
    }

    G.release();
    fi.release();
    cap.release();//Releasing the buffer memory//
    grayFrame.release();
    currentFrame.release();
    gi.release();
    Gi.release();
    mask.release();
    Hi.release();
    destroyAllWindows();
    }

