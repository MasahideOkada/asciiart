#ifndef ASCII_FUNCTIONS_H
#define ASCII_FUNCTIONS_H

#include <iostream>
#include <vector>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include "gamma_correction.h"

void make_intensity_to_char_idx(std::vector<int> &intensity_to_char_idx, const int &num_chars);

void make_ascii_image(cv::Mat &ascii_image, cv::Mat &input_image, cv::Mat &sub_image,
                      const int &num_rows, const int &num_cols, const int &char_height, const int &char_width,
                      std::vector<std::vector<uchar>> &char_image_data, const std::vector<int> &intensity_to_char_idx,
                      std::string_view policy, const bool &with_original, cv::Mat &orig_image, const double &orig_rescale,
                      std::string_view orig_pos);

void make_ascii_video(const std::string &output_path, const std::string &video_path, const int &num_lines,
                      const int &char_height, const int &char_width,
                      std::vector<std::vector<uchar>> &char_image_data, const std::vector<int> &intensity_to_char_idx,
                      const bool &do_gamma_correction, const double &gamma, const std::vector<uchar> &lut1, const std::vector<uchar> &lut2,
                      std::string_view policy, const bool &with_original, const double &orig_rescale, std::string_view orig_pos);

void make_pbar(std::vector<std::string> &pbar_list);

#endif