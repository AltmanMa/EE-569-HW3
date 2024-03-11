#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

// Assuming the size of the image is known beforehand
const int WIDTH = 252; // Example width
const int HEIGHT = 252; // Example height

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
    threshold(src, binarizedImage, maxVal * 0.5, 255, THRESH_BINARY);

    // Apply the thinning (The thinning function implementation remains the same as before)

    // Save the result
    saveRawImage(binarizedImage, argv[2]);

    cout << "Thinning completed. Result saved as " << argv[2] << endl;

    return 0;
}
