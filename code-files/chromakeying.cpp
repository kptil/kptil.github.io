#include <iostream>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

// Set up global variables holding the user-specified color and tolerance
Mat frame, mask, frameNoGreen;
Vec3b chosenColor = Vec3b(0, 0, 0);
int tol = 64;
int maxTol = 255;
bool init = true;

void removeGreenScreen() {
    // Sets mask bit to 255 if frame bit is in range of chosenColor +- tolerance, else 0 
    inRange(frame, chosenColor - Vec3b(tol, tol, tol), chosenColor + Vec3b(tol, tol, tol), mask); // Addition operation auto limits results to between zero and 255

    // Sets mask to 1 where objects are, 0 otherwise
    threshold(mask, mask, 0, 1, THRESH_BINARY_INV);

    // Removes the green screen by multiplying by the mask
    Mat frameChannels[3], frameChannelsMasked[3];
    split(frame, frameChannels);

    for (int i = 0; i < 3; i++) {
        multiply(frameChannels[i], mask, frameChannelsMasked[i]);
    }
    merge(frameChannelsMasked, 3, frameNoGreen);
}

// Calls removeGreenScreen with the chosen color and existing tolerance
void selectColor(int action, int x, int y, int flags, void* userdata) {
    if (action == EVENT_LBUTTONDOWN) {
        chosenColor = frame.at<Vec3b>(y, x);
    }

    if (action == EVENT_LBUTTONUP) {
        removeGreenScreen();
        imshow("Frame", frameNoGreen);
    }
}

// Calls removeGreenScreen with the chose tolerance and existing color
void chooseTolerance(int, void*) {
    if (init) {
        init = false;
    }
    else {
        removeGreenScreen();
        imshow("Frame", frameNoGreen);
    }
}

Mat applyBackground(Mat frame, Mat bg) {
    Mat maskFlipped = 1 - mask;
    Mat frameBG;
    Mat backgroundChannels[3], frameBackgroundChannels[3], frameNoGreenChannels[3], frameBGChannels[3];

    split(bg, backgroundChannels);
    split(frameNoGreen, frameNoGreenChannels);
    for (int i = 0; i < 3; i++) {
        multiply(backgroundChannels[i], maskFlipped, frameBackgroundChannels[i]);
        add(frameBackgroundChannels[i], frameNoGreenChannels[i], frameBGChannels[i]);
    }

    merge(frameBGChannels, 3, frameBG);
    return frameBG;
}

int main() {
    // Read in video and display the first frame to allow the user to select the green color and set the tolerance
    VideoCapture cap("resources/greenscreen-asteroid.mp4");

    if (!cap.isOpened()) {
        cout << "Error opening video stream or file" << endl;
        return -1;
    }

    int frameWidth = cap.get(CAP_PROP_FRAME_WIDTH);
    int frameHeight = cap.get(CAP_PROP_FRAME_HEIGHT);

    VideoWriter out("output.mp4", VideoWriter::fourcc('X', 'V', 'I', 'D'), cap.get(CAP_PROP_FPS), Size(frameWidth, frameHeight));

    cap >> frame;

    namedWindow("Frame", WINDOW_NORMAL);
    resizeWindow("Frame", frameWidth / 1.5, frameHeight / 1.5);
    imshow("Frame", frame);

    setMouseCallback("Frame", selectColor);
    createTrackbar("Tolerance", "Frame", &tol, maxTol, chooseTolerance);

    // When the user presses escape, read in the background, remove the green screen, and 
    // apply the background to each frame of the video
    char k = (char)waitKey(0);
    if (k == 27) {
        Mat background = imread("resources/background.jpg");
        Mat backgroundCropped = background(Rect(0, 0, frameWidth, frameHeight));

        Mat frameCombined = applyBackground(frame, backgroundCropped);
        imshow("Frame", frameCombined);
        out.write(frameCombined);

        while (1) {
            cap >> frame;

            if (frame.empty())
                break;

            removeGreenScreen();
            frameCombined = applyBackground(frame, backgroundCropped);
            out.write(frameCombined);
        }
    }

    cap.release();
    out.release();
    destroyAllWindows();

    return 0;
}