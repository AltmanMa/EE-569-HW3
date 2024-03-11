#include <algorithm>
#include <iomanip>
#include <string_view>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <vector>
#include <utility>

using namespace std;
constexpr int Size = 328;
constexpr int BytesPerPixel = 3; // 假设是24位RGB图像

struct Point2f {
    float x, y;
    Point2f(float px, float py) : x(px), y(py) {}
};

Point2f pixelToCartesian(int i, int j) {
    float x = j - 0.5f;
    float y = Size - i + 0.5f;
    return Point2f(x, y);
}

Point2f distortCoordinates(float x, float y) {
    const float halfSize = Size / 2.0f;
    const float arcThickness = 64.0f;

    float dx = x - halfSize;
    float dy = y - halfSize;

    float distance = hypot(dx, dy);
    float angle = atan2(dy, dx);

    float newDistance =  distance * (halfSize - arcThickness * abs(cos(2 * angle))) / halfSize;

    float newX = halfSize + newDistance * cos(angle);
    float newY = halfSize + newDistance * sin(angle);

    return Point2f(newX, newY);
}

unsigned char nearestNeighborInterpolation(unsigned char Imagedata[Size][Size][BytesPerPixel], float x, float y, int channel) {
    int nearestX = floor(x);
    int nearestY = floor(y);

	bool hasValidNeighbor = false;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            int checkX = nearestX + dx;
            int checkY = nearestY + dy;
            if (checkX >= 0 && checkX < Size && checkY >= 0 && checkY < Size) {
                hasValidNeighbor = true;
                break;
            }
        }
        if (hasValidNeighbor) {
            break;
        }
    }

    if (!hasValidNeighbor) {
        return 0; 
    }
	
    nearestX = max(0, min(nearestX, Size - 1));
    nearestY = max(0, min(nearestY, Size - 1));

    return Imagedata[nearestY][nearestX][channel];
}



int main(int argc, char *argv[])

{
	// Define file pointer and variables
	FILE *file;	
	
	// Check for proper syntax
	if (argc < 3){
		cout << "Syntax Error - Incorrect Parameter Usage:" << endl;
		cout << "program_name input_image.raw output_image.raw [BytesPerPixel = 1] [Size = 256]" << endl;
		return 0;
	}
	
	// Allocate image data array
	unsigned char Imagedata[Size][Size][BytesPerPixel];
    unsigned char ProcessedData[Size][Size][BytesPerPixel]; 

	// Read image (filename specified by first argument) into image data matrix
	if (!(file=fopen(argv[1],"rb"))) {
		cout << "Cannot open file: " << argv[1] <<endl;
		exit(1);
	}
	fread(Imagedata, sizeof(unsigned char), Size*Size*BytesPerPixel, file);
	fclose(file);

	vector<pair<Point2f, Point2f>> coordinateMapping;

	for (int i = 0; i < Size; ++i) {
    	for (int j = 0; j < Size; ++j) {
        // Convert to Cartesian coordinates
       		Point2f cartesian = pixelToCartesian(i, j);
        
        // Apply distortion
        	Point2f distorted = distortCoordinates(cartesian.x, cartesian.y);

        // Save the mapping
        	coordinateMapping.push_back({distorted, Point2f(i, j)});
    	}
	}


	for (int i = 0; i < Size; ++i) {
    	for (int j = 0; j < Size; ++j) {
        	for (int k = 0; k < BytesPerPixel; ++k) {
            	ProcessedData[i][j][k] = 0; // Set to black
        	}
    	}
	}
// Iterate over the mapping and copy pixels from Imagedata to ProcessedData
	for (const auto& map : coordinateMapping) {
    	Point2f distorted = map.first;
    	Point2f original = map.second;

    // Convert distorted back to pixel coordinates, rounding as necessary
	int iPrime = Size - round(distorted.y) - 1;    	
	int jPrime = round(distorted.x);

    // Check bounds
    	if (iPrime >= 0 && iPrime < Size && jPrime >= 0 && jPrime < Size) {
        // Copy pixel data from original to distorted position
        	for (int k = 0; k < BytesPerPixel; ++k) {
            	ProcessedData[iPrime][jPrime][k] = Imagedata[static_cast<int>(original.x)][static_cast<int>(original.y)][k];
        	}
    	}
	}

	// Write image data (filename specified by second argument) from image data matrix

	if (!(file=fopen(argv[2],"wb"))) {
		cout << "Cannot open file: " << argv[2] << endl;
		exit(1);
	}
	fwrite(ProcessedData, sizeof(unsigned char), Size*Size*BytesPerPixel, file);
	fclose(file);

	return 0;
}
