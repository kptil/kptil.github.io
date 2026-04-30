#include <iostream>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

Point center;
Mat img;
int radius = 15;
int dist = 15;

void removeBlemish(int action, int x, int y, int flags, void* userdata) {

    // Stores the clicked point as the center of circle when the left mouse button is pressed
    if (action == EVENT_LBUTTONDOWN) {
        // Check the circle will be within the image before proceeding
        if (x - radius * 3 < 0 || x + radius * 3 > img.size().width || y - radius * 3 < 0 || y - radius * 3 > img.size().height) {
            putText(img, "Choose a point further from the edge.", Point(50, 50), FONT_HERSHEY_COMPLEX, 1, Scalar(255, 255, 255), 1, LINE_AA);
        }
        else {
            center = Point(x, y);
        }
    }

    // When the mouse is released, assess nearby patches for smoothness by comparing the means of the
    // gradients. The smoothest patch will be the most blemish-free one. Then use seamless cloning
    // to combine the best patch with the image to cover up the blemish.
    else if (action == EVENT_LBUTTONUP) {

        // Create a vector of center points for nearby patches
        vector<Point> points;
        points.push_back(Point(x, y - 2 * dist));
        points.push_back(Point(x, y + 2 * dist));
        points.push_back(Point(x - dist, y - 2 * dist));
        points.push_back(Point(x - dist, y + 2 * dist));
        points.push_back(Point(x + dist, y - 2 * dist));
        points.push_back(Point(x + dist, y + 2 * dist));
        points.push_back(Point(x + 2 * dist, y));
        points.push_back(Point(x - 2 * dist, y));

        Mat imgNorm, sobelx, sobely, patch, bestPatch;
        Mat patchChannels[3];
        Mat imgChannels[3];
        Mat mask = Mat::zeros(img.size(), CV_8U);

        split(img, imgChannels);

        double mean = DBL_MAX;
        Scalar xmean, ymean, currMean;

        for (auto point : points) {
            // Reset masks 
            mask = Mat::zeros(img.size(), CV_8U);

            // Make masks for new patch
            circle(mask, point, radius, Scalar(255, 255, 255), -1, LINE_AA);
            threshold(mask, mask, 25, 255, THRESH_BINARY);
            normalize(mask, mask, 0, 1, NORM_MINMAX);

            // multiply the image by the mask to get the patch and nothing else
            for (int i = 0; i < 3; i++) {
                multiply(imgChannels[i], mask, patchChannels[i]);
            }
            merge(patchChannels, 3, patch);

            // convert the patch to grayscale, calculate the gradients, and assess the mean
            cvtColor(patch, patch, COLOR_BGR2GRAY);
            Sobel(patch, sobelx, CV_32F, 1, 0); // type must be CV_32F or CV_64F
            Sobel(patch, sobely, CV_32F, 0, 1);

            normalize(sobelx, sobelx, 0, 1, NORM_MINMAX);
            normalize(sobely, sobely, 0, 1, NORM_MINMAX);
            xmean = cv::mean(sobelx);
            ymean = cv::mean(sobely);
            currMean = (xmean + ymean) / 2;

            if (currMean[0] < mean) {
                mean = currMean[0];
                bestPatch = patch;
            }
        }

        // Combine the best patch and the original image using seamless cloning
        cvtColor(bestPatch, bestPatch, COLOR_GRAY2BGR);
        seamlessClone(img, img, bestPatch, center, img, NORMAL_CLONE);
        imshow("w1", img);
    }
}

int main() {
    img = imread("./images/blemish.png");

    namedWindow("w1");
    setMouseCallback("w1", removeBlemish);

    int k = 0;
    int retVal = 1;

    // Display the image in an interactive window until the escape character
    // is pressed
    while (k != 27 && retVal != 0) {
        imshow("w1", img);
        k = waitKey(0);
        retVal = getWindowProperty("w1", WND_PROP_VISIBLE);
    }

    return 0;
}