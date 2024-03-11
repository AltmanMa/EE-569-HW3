#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

// Assuming the size of the image is known beforehand
const int WIDTH = 256; // Example width
const int HEIGHT = 256; // Example height

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

Mat loadRawImage(const string& filename) {
    ifstream file(filename, ios::binary);

    if (!file.is_open()) {
        cerr << "Could not open the file - '" << filename << "'" << endl;
        exit(-1);
    }

    Mat image(HEIGHT, WIDTH, CV_8UC1); // Create a single channel 8 bit depth image
    file.read((char*)image.data, WIDTH * HEIGHT);

    if (!file) {
        cerr << "Error reading the file - '" << filename << "'" << endl;
        exit(-1);
    }

    file.close();
    return image;
}

void saveRawImage(const Mat& image, const string& filename) {
    ofstream file(filename, ios::binary);

    if (!file.is_open()) {
        cerr << "Could not open the file for writing - '" << filename << "'" << endl;
        exit(-1);
    }

    file.write((char*)image.data, image.total() * image.elemSize());

    if (!file) {
        cerr << "Error writing the file - '" << filename << "'" << endl;
        exit(-1);
    }

    file.close();
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        cout << "Usage: " << argv[0] << " <InputImagePath.raw> <OutputImagePath.raw>" << endl;
        return -1;
    }

    // Load the .raw image
    Mat src = loadRawImage(argv[1]);

    // Binarize the image
    double maxVal = 255; // Assuming the image is 8-bit
    Mat binarizedImage;
    Mat thinnedImage;
    threshold(src, binarizedImage, maxVal * 0.5, 255, THRESH_BINARY);

    thinning(binarizedImage, thinnedImage); // Ensure this line applies thinning

    // Save the result
    saveRawImage(thinnedImage, argv[2]);

    cout << "Thinning completed. Result saved as " << argv[2] << endl;

    return 0;
}
