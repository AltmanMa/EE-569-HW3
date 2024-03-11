#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;

// Function to detect and return matched points using SIFT or SURF
vector<Point2f> detectAndMatchFeatures(Mat img1, Mat img2) {
    // 初始化 SIFT 或 SURF 检测器
    // 注意：根据你的 OpenCV 版本和配置，这里可能需要变动
    Ptr<Feature2D> detector = SIFT::create();
    // Ptr<Feature2D> detector = SURF::create(400);

    // 检测特征点
    vector<KeyPoint> keypoints1, keypoints2;
    Mat descriptors1, descriptors2;
    detector->detectAndCompute(img1, Mat(), keypoints1, descriptors1);
    detector->detectAndCompute(img2, Mat(), keypoints2, descriptors2);

    // 匹配描述符
    BFMatcher matcher(NORM_L2);
    vector<DMatch> matches;
    matcher.match(descriptors1, descriptors2, matches);

    // 对匹配进行排序，以便我们可以删除一些不好的匹配
    sort(matches.begin(), matches.end());

    // 选择最佳匹配
    const int numGoodMatches = matches.size() * 0.15;
    matches.erase(matches.begin()+numGoodMatches, matches.end());

    // 抽取匹配点
    vector<Point2f> points1, points2;
    for(size_t i = 0; i < matches.size(); i++){
        points1.push_back(keypoints1[matches[i].queryIdx].pt);
        points2.push_back(keypoints2[matches[i].trainIdx].pt);
    }

    return points1; // 或根据需要返回 points2 或两者都返回
}

// Function to calculate homography matrix
Mat calculateHomography(vector<Point2f>& pts_img1, vector<Point2f>& pts_img2) {
    // ... 使用匹配点计算单应性矩阵 ...
    // 返回计算出的单应性矩阵
}

// Function to stitch images
Mat stitchImages(Mat img1, Mat img2, Mat H) {
    // ... 应用单应性变换并拼接图像 ...
    // 返回拼接后的全景图
}

int main() {
    // 读取图像
    Mat image1 = imread("path_to_image_1.jpg", IMREAD_COLOR);
    Mat image2 = imread("path_to_image_2.jpg", IMREAD_COLOR);

    // 检测和匹配特征点
    vector<Point2f> matched_pts_img1, matched_pts_img2;
    matched_pts_img1 = detectAndMatchFeatures(image1, image2);

    // 计算单应性矩阵
    Mat H = calculateHomography(matched_pts_img1, matched_pts_img2);

    // 应用单应性变换并拼接图像
    Mat panorama = stitchImages(image1, image2, H);

    // 显示和保存结果
    imshow("Panorama", panorama);
    imwrite("panorama.jpg", panorama);

    waitKey(0);
    return 0;
}
