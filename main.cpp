#include <opencv2/opencv.hpp>
#include <iostream>
#include <unistd.h>
#include "CFT_Track.hpp"

using namespace cv;
using namespace std;

int main() {
    
    CFT test;
    test.startTracking();
    
    return 0;
    
}