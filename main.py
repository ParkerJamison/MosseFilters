import cv2 as cv
import cft_tracker
import time

def main():
    cap = cv.VideoCapture(0)
    if not cap.isOpened():
        print("Error: Could not open video capture.")
        return
    
    ret, frame = cap.read()
    if not ret:
        print("Error: Could not read frame.")
        cap.release()
        return
    
    tracker = cft_tracker.CFT()

    tracks = []
    
    for i in range(1):
        tracks.append(tracker.initTracking(frame))
    
    # Create a window with specific size
    window_name = "Frame"
    cv.namedWindow(window_name, cv.WINDOW_NORMAL)
    cv.resizeWindow(window_name, 640, 480)  # Window size: 640x480
    
    prev_time = time.time()
    fps = 0
    while True:
        ret, frame = cap.read()
        if not ret:
            print("Error: Could not read frame.")
            cap.release()
            return
        
        for track in tracks:
            tracker.updateTracking(frame, track)
            cv.rectangle(frame, track.getDisplayBBox(), color=(0, 255, 0), thickness=2)

        # Calculate FPS
        current_time = time.time()
        delta_time = current_time - prev_time
        if delta_time > 0: 
            fps = 1.0 / delta_time
        prev_time = current_time
        
        # Display FPS on frame
        cv.putText(
            frame,
            f"FPS: {fps:.2f}",
            (10, 30),
            cv.FONT_HERSHEY_SIMPLEX,
            1.0, 
            (0, 255, 0), 
            2  
        )
        cv.imshow(window_name, frame)

        if cv.waitKey(1) == ord('q'):
            break


    cap.release()

if __name__ == "__main__":
    main()