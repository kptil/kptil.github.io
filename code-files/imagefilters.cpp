#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

/*
Uses the Laplacian to detect edges in the image. A Gaussian blur is used to reduce noise beforehand. Then the
image is normalized to remove negative values and a threshold function is used to reduce the number of lines. 
Using a smaller kernel in the Laplacian function will give you finer edges with less distinction between them,
so I used a slightly larger kernel than normal.
*/
Mat pencilSketch(Mat image, int arguments = 0) {

    Mat imGray, imBlur, imLP, imNorm, imFlipped, imOut;
    cvtColor(image, imGray, COLOR_BGR2GRAY);
    GaussianBlur(imGray, imBlur, Size(3, 3), 0, 0);
    Laplacian(imBlur, imLP, CV_32F, 5, 1, 0); 

    normalize(imLP, imNorm, 0, 255, NORM_MINMAX, CV_8U);

    bitwise_not(imNorm, imFlipped);
    threshold(imFlipped, imOut, 106, 255, THRESH_BINARY); 

    return imOut;
}

/*
* The cartoonify filter applies regions of constant color within identified edges. I use the pencil sketch
* function first to identify the edges, then divide by 255 so that the lines are 0s and the white space is 1s,
* creating a mask. Then I use a bilater filter to reduce noise while preserving edges. The blurred image is split 
* into color channels, each of which is multiplied by pencil sketch mask to leave the black lines and replace
* the white space with color from the original image.
*/
Mat cartoonify(Mat image, int arguments = 0) {

    Mat pencil = pencilSketch(image);
    pencil = pencil / 255; 

    Mat blur;
    bilateralFilter(image, blur, -1, 50, 50);

    Mat cartoonImage, blurChannels[3], blurMaskChannels[3];
    split(blur, blurChannels);
    for (int i = 0; i < 3; i++) {
        multiply(pencil, blurChannels[i], blurMaskChannels[i]);
    }
    merge(blurMaskChannels, 3, cartoonImage);

    return cartoonImage;
}

int main() {

    Mat image = imread("images/SofiaChristmas25.jpg");

    Mat cartoonImage = cartoonify(image);
    Mat pencilSketchImage = pencilSketch(image);

    imwrite("images/pencilSketchImage.jpg", pencilSketchImage);
    imwrite("images/cartoonImage.jpg", cartoonImage);

    return 0;
}