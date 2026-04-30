#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

/*
Variance of absolute values of Laplacian
Uses the Laplacian sharpening kernel [0 -1 0, -1 4 -1, 0 -1 0] and a control parameter of 1/6
to sharpen each frame of the video. The sharpest image is found by comparing the absolute
values of each frame convolved with the sharpening kernel.

Source: Pacheco et al, "Diatom autofocusing in brightfield microscopy: a comparative study," 
IEEE Conference Paper, 2000.
*/

double var_abs_laplacian(Mat image) {
    
    Mat im, dst;
    Mat lp = (Mat_<double>(3, 3) << 0.0, -1.0, 0.0, -1.0, 4.0, -1.0, 0.0, -1.0, 0.0);

    cvtColor(image, im, COLOR_BGR2GRAY);
    filter2D(im, dst, CV_64F, (1.0/6.0)*lp); 
    pow(dst, 2, dst);
    sqrt(dst, dst); 

    return sum(dst)[0];
}

/*
Implement Sum Modified Laplacian - Method 2
Here, instead of defining the Laplacian sharpening kernel outright, we use the
modified Laplacian function ML(x,y), which approximates the Laplacian filter - 
d^2I / dx^2 + d^2I / dy^2.

ML(x,y) = |2I(x,y) - I(x-step,y) - I(x+step,y)| + |2I(x,y) - I(x,y-step) - I(x,y+step)|

We turn this into the below kernels and convolve them with the image to approximate
the partial derivatives in the x and y directions separately. The kernels sum to
the Laplacian sharpening kernel.

MLx = [0 0 0, -1 2 -1, 0 0 0] 
MLy = [0 -1 0, 0 2 0, 0 -1 0]

We apply the absolute value before summing because the partial derivatives of images
are typically nearly equal but with opposite signs. The variance of absolute value
process in the above function handles this by finding squaring and taking the square
root to get the absolute value.

Source: Nayer, Shree K, "Shape From Focus," 1989.
*/ 


double sum_modified_laplacian(Mat image) {
    
    Mat im, dstx, dsty, S;
    cvtColor(image, im, COLOR_BGR2GRAY);
    
    Mat MLx = (Mat_<double>(3, 3) << 0.0, -1.0, 0.0, 0.0, 2.0, 0.0, 0.0, -1.0, 0.0);
    Mat MLy = (Mat_<double>(3, 3) << 0.0, 0.0, 0.0, -1.0, 2.0, -1.0, 0.0, 0.0, 0.0);

    filter2D(im, dstx, CV_64F, MLx);
    filter2D(im, dsty, CV_64F, MLy);

    S = abs(dstx) + abs(dsty);
    
    return sum(S)[0];
}

int main() {
    
    // Read video and first frame
    string filename = "resources/focus-test.mp4";
    VideoCapture cap(filename);

    Mat frame;
    cap >> frame;

    cout << "Total number of frames : " << (int)cap.get(CAP_PROP_FRAME_COUNT);

    // Set up loop variables
    double maxV1 = 0;
    double maxV2 = 0;

    Mat bestFrame1;
    Mat bestFrame2;

    int bestFrameId1 = 0;
    int bestFrameId2 = 0;

    // Get initial measures of focus from both methods
    double val1 = var_abs_laplacian(frame);
    double val2 = sum_modified_laplacian(frame);

    // Specify the ROI for flower in the frame
    int topCorner = 20;
    int leftCorner = 430;
    int bottomCorner = 650;
    int rightCorner = 1050;

    Mat flower;
    flower = frame(Range(topCorner, bottomCorner), Range(leftCorner, rightCorner));

    while (1) {
        // Crop the current frame to just the flower region
        flower = frame(Range(topCorner, bottomCorner), Range(leftCorner, rightCorner));

        // Get focus measures
        val1 = var_abs_laplacian(flower);
        val2 = sum_modified_laplacian(flower);

        // Update stored maximum values and best frame variables
        if (val1 > maxV1) {
            maxV1 = val1;
            bestFrameId1 = (int)cap.get(CAP_PROP_POS_FRAMES);
            bestFrame1 = frame.clone();
        }

        if (val2 > maxV2) {
            maxV2 = val2;
            bestFrameId2 = (int)cap.get(CAP_PROP_POS_FRAMES);
            bestFrame2 = frame.clone();
        }
        cap >> frame;
        if (frame.empty())
            break;
    }

    cout << "================================================" << endl;

    cout << "Frame ID of the best frame [Method 1]: " << bestFrameId1 << endl;
    cout << "Frame ID of the best frame [Method 2]: " << bestFrameId2 << endl;

    cap.release();

    // Concatenate the best frames found by both methods into one output image
    // for comparison
    Mat out;
    hconcat(bestFrame1, bestFrame2, out);

    imwrite("out.png", out);

    return 0;
}