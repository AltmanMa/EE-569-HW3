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

    return make_pair(points1, points2);
}

// Function to calculate homography matrix
Mat calculateHomography(vector<Point2f>& pts_img1, vector<Point2f>& pts_img2) {
    // 计算单应性矩阵
    Mat H = findHomography(pts_img1, pts_img2, RANSAC);

    return H; // 返回单应性矩阵
}


// Function to stitch images
Mat stitchImages(vector<Mat>& images) {
    // 检测并匹配特征点
    vector<Point2f> pts_img1, pts_img2;
    Mat H;
    Mat panorama = images[0];  // 以第一张图片为基础

    // 循环遍历所有图片（除了第一张）
    for (size_t i = 1; i < images.size(); ++i) {
        // 检测特征点并匹配
        tie(pts_img1, pts_img2) = detectAndMatchFeatures(panorama, images[i]);

        // 计算单应性矩阵
        H = calculateHomography(pts_img2, pts_img1);  // 注意：这里 pts_img2 和 pts_img1 的顺序被颠倒了

        // 更新全景图大小，假设所有图片大小一样
        Size size(panorama.cols + images[i].cols, panorama.rows);
        Mat warpedImage(size, CV_8UC3);

        // 应用单应性变换
        warpPerspective(images[i], warpedImage, H, size);

        // 更新全景图，将旧的全景图拷贝到新的画布上
        Mat roi(warpedImage, Rect(0, 0, panorama.cols, panorama.rows));
        panorama.copyTo(roi);

        // 将变换后的图片复制到全景图画布上，这里需要实现一种方法来融合图像
        // 为简单起见，我们使用重叠区域的简单平均融合
        // 实际应用中可能需要更复杂的融合策略
        Mat half(panorama, Rect(0, 0, warpedImage.cols, warpedImage.rows));
        addWeighted(half, 0.5, warpedImage, 0.5, 0, half);

        // 更新参考全景图
        panorama = warpedImage.clone();
    }

    // 裁剪全景图以去除右侧可能的黑色区域
    Rect crop = boundingRect(pts_img1);  // 使用最后一对匹配点来确定裁剪区域
    panorama = panorama(crop);

    return panorama;
}

Mat readRawImage(const string& filename, int width, int height) {
    ifstream file(filename, ios::binary);
    if (!file) {
        cerr << "Cannot open file: " << filename << endl;
        exit(1);
    }

    Mat image = Mat(height, width, CV_8UC3);
    file.read(reinterpret_cast<char*>(image.data), width * height * 3);

    if (!file) {
        cerr << "Error reading file: " << filename << endl;
        exit(1);
    }

    return image;
}

int main() {
    // 图像的宽度和高度
    const int width = 605;
    const int height = 454;

    // 图像文件名
    vector<string> filenames = {
        "raw_images/toys_left.raw",
        "raw_images/toys_middle.raw",
        "raw_images/toys_right.raw"
    };

    vector<Mat> images;
    for (const string& filename : filenames) {
        Mat img = readRawImage(filename, width, height);
        images.push_back(img);
    }

    // 现在我们有了三个 cv::Mat 图像对象，可以将它们拼接在一起
    Mat panorama = stitchImages(images);

    // 显示和保存结果
    imshow("Panorama", panorama);
    imwrite("panorama.jpg", panorama);

    waitKey(0);
    return 0;
}