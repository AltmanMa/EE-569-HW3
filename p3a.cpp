#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

// Function to binarize image based on the threshold
void binarizeImage(Mat& src, Mat& dst, double max_val) {
    double threshold = max_val * 0.5;
    cv::threshold(src, dst, threshold, 255, THRESH_BINARY);
}

// Thinning iteration function
void thinningIteration(Mat& img, int iter) {
    Mat marker = Mat::zeros(img.size(), CV_8UC1);

    for (int i = 1; i < img.rows - 1; i++) {
        for (int j = 1; j < img.cols - 1; j++) {
            uchar p2 = img.at<uchar>(i-1, j);
            uchar p3 = img.at<uchar>(i-1, j+1);
            uchar p4 = img.at<uchar>(i, j+1);
            uchar p5 = img.at<uchar>(i+1, j+1);
            uchar p6 = img.at<uchar>(i+1, j);
            uchar p7 = img.at<uchar>(i+1, j-1);
            uchar p8 = img.at<uchar>(i, j-1);
            uchar p9 = img.at<uchar>(i-1, j-1);

            int A = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) +
                    (p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) +
                    (p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) +
                    (p8 == 0 && p9 == 1) + (p9 == 0 && p2 == 1);
            int B = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
            int m1 = iter == 0 ? (p2 * p4 * p6) : (p2 * p4 * p8);
            int m2 = iter == 0 ? (p4 * p6 * p8) : (p2 * p6 * p8);

            if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0)
                marker.at<uchar>(i,j) = 1;
        }
    }

    img &= ~marker;
}

// Function to perform the thinning operation on binary images
void thinning(Mat& src, Mat& dst) {
    src.copyTo(dst);
    dst /= 255; // Normalize the image

    Mat prev = Mat::zeros(dst.size(), CV_8UC1);
    Mat diff;

    do {
        thinningIteration(dst, 0);
        thinningIteration(dst, 1);
        absdiff(dst, prev, diff);
        dst.copyTo(prev);
    }
    while (cv::countNonZero(diff) > 0);

    dst *= 255; // De-normalize the image
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <ImagePath>" << endl;
        return -1;
    }

    // Load the image
    Mat src = imread(argv[1], IMREAD_GRAYSCALE);
    if (src.empty()) {
        cout << "Could not read the image: " << argv[1] << endl;
        return -1;
    }

    // Binarize the image
    double maxVal;
    minMaxLoc(src, nullptr, &maxVal);
    Mat binarizedImage;
    binarizeImage(src, binarizedImage, maxVal);

    // Apply the thinning
    Mat thinnedImage;
    thinning(binarizedImage, thinnedImage);

    // Save the result
    imwrite("thinned_image.jpg", thinnedImage);

    cout << "Thinning completed. Result saved as thinned_image.jpg" << endl;

    return 0;
}