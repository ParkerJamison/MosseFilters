
# Correlation Filter Tracking with MOSSE Filters. 

This implementation allows for support in Python and C++ and currently designed to work from a webcam, as well as many Raspberry Pi cameras.

This tracker utilizes two separate filters per track. One designated for long-term tracking, and one designated for short-term tracking. Both are updated depending on the response each filter has on a video frame. 

## Tracking Initialization

Filters for tracking are initialized by a call to initTracking(). Within this function, the user selects the ROI for the track/s. This ROI is used to determine the search area for tracking. 

## Tracking Updates

The sharpness of the correlation response for each filter is measure by Peak-to-Sidelobe ratio (PSR). A larger PSR indicates a more indicates a strong detection, and a lower PSR indicates a weaker detection. Both the long and short term filter's PSR are evaluated each frame. The maximum of the two is what is utilized to update the track. The logic for updating the filters is as follows. 

- PSR > 20
    * This indicates a strong detection. So both filters are updated
- 6 < PSR < 20
    * This indicates a fair detection. With there being a strong likelihood the track is partially occluded or changing its appearance. The short term filter is the only filter updated under this condition. This is done to gather as much information about a track's new appearance or occlusion. 
- PSR < 6
    * This indicates a failure to detect or really heavy occlusion. With this PSR, no filters are updated. 
  
The conditions listed above were used based on the paper listed in the references below, and my own testing. 

## Usage

A usage example can be seen in the main.cpp and main.py file. This project is designed for the tracker to take in concurrent video frames from the main file. The first step is initializing the tracker. Currently, the best way to do this is with cv::VideoCapture, use this to capture an initial video frame and pass that into the initTracking() function. After this, inside a while loop, continuously capture video frames again with VideoCapture(). Also passing these video frames into updateTracking(). 

## Future

### Adding a new track in real time
I have plans to add the ability to add a new track in real time. This will feature will have a set time limit once started (10 sec). The user will then have the ability to select a new track on the paused video frame. This is all done while real time video frames are being stored in a buffer. Once the track is selected, or the time limit runs out, the tracker will be updated on the stored buffer video frames until it reaches back to the real time video feed. 

### Track going out of frame

If a track goes out of frame, instead of halting tracking, the track will be placed in a stand-by mode. Where the search region will increase (possibly to the size of the frame), and the attempt to detect the track will go from every frame to every ten frames (or more). This stand-by mode will go on either forever, to a set time limit, or until the track is found again. 

### Add specific tracking modes
Being able to select specific tracking modes, like human, car, drones, would allow for more specific optimizations and lead to better tracking performance. 




  
## References
This project is heavily based on the paper below. 

- D. S. Bolme, J. R. Beveridge, B. A. Draper, and Y. M. Lui,  
  "Visual Object Tracking using Adaptive Correlation Filters,"  
  *IEEE Conference on Computer Vision and Pattern Recognition (CVPR)*, 2010.  
  [PDF](https://www.cs.colostate.edu/~draper/papers/bolme_cvpr10.pdf)
