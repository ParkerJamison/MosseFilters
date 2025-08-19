#include <opencv2/opencv.hpp>
#include <iostream>
#include <unistd.h>
#include "CFT_Track.hpp"
#include "TrackID.hpp"

using namespace cv;
using namespace std;

int main() {
    
    //CFT test;

    //test.startTracking();
    Track track;
    CFT test;

    Mat currentFrame;
    namedWindow("Video Player");//Declaring the video to show the video//
    VideoCapture cap(0);//Declaring an object to capture stream of frames from default camera//

    if (!cap.isOpened()){ //This section prompt an error message if no video stream is found//
        cout << "No video stream detected" << endl;
        system("pause");
        return 0;
    }
    waitKey(0);
    cap >> currentFrame;
    
    track = test.initTracking(currentFrame);

    while (true) { 
        cap >> currentFrame;
        if (currentFrame.empty()){ //Breaking the loop if no video frame is detected//
            break;
        }
        test.updateTracking(currentFrame, track);

        rectangle(currentFrame, track.getDisplayBBox(), Scalar(255, 0, 0), 2);

        imshow("Video Player2", currentFrame);
        //waitKey(0);

        char c = (char) waitKey(25);//Allowing 25 milliseconds frame processing time and initiating break condition//
        if (c == 27){ //If 'Esc' is entered break the loop//
            break;
        }
    }
    
    return 0;
    
}