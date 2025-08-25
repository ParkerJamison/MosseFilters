#include <opencv2/opencv.hpp>
#include "CFT_Track.hpp"
#include "TrackID.hpp"

using namespace cv;
using namespace std;

int main() {
    // Create Track Objects
    //Track track1, track2;
    vector<Track> tracks;

    for (int i = 0; i < 10; i++) {
        Track track;
        tracks.push_back(track);
    }

    // Create the tracekr object
    CFT tracker;

    Mat currentFrame;
    namedWindow("Video Player");
    VideoCapture cap(0);

    if (!cap.isOpened()) {
        cout << "No video stream detected" << endl;
        return 0;
    }

    waitKey(50); // Wait for the camera to be ready

    cap >> currentFrame;

    // init all the tracks. 
    for (Track &track : tracks) {
        track = tracker.initTracking(currentFrame);
    }

    while (true) { 
        cap >> currentFrame;
        if (currentFrame.empty()){ 
            break;
        }

        // Update each track. 
        for (Track &track : tracks) {
            tracker.updateTracking(currentFrame, track);
            rectangle(currentFrame, track.getDisplayBBox(), Scalar(255, 0, 0), 2);
        }

        imshow("Video Player2", currentFrame);

        char c = (char) waitKey(25);
        if (c == 27){ //If 'Esc' is entered break the loop
            break;
        }
    }
    return 0;
}