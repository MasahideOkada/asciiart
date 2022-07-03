#include "gamma_correction.h"

void gamma_correction(cv::Mat &src, cv::Mat &dst, const std::vector<uchar> &lut1, const std::vector<uchar> &lut2){
    cv::LUT(src, lut1, src);
    cv::cvtColor(src, dst, cv::COLOR_BGR2GRAY);
    cv::LUT(dst, lut2, dst);
}