#ifndef TextRenderer_H
#define TextRenderer_H
#include <opencv2/opencv.hpp>
#include <string>

// ==========================================
// 文字渲染類別
// ==========================================
class TextRenderer {
public:
    // 繪製文字的靜態方法
    static void drawText(cv::Mat& dst, const std::string& str, cv::Point org, cv::Scalar color, int fontSize = 30);
};
#endif