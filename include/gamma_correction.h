#ifndef GAMMA_CORRECTION_H
#define GAMMA_CORRECTION_H

#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

void gamma_correction(cv::Mat &src, cv::Mat &dst, const std::vector<uchar> &lut1, const std::vector<uchar> &lut2);

#endif