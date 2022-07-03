#ifndef MAKE_CHAR_IMAGE_H
#define MAKE_CHAR_IMAGE_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cassert>
#include <ft2build.h>
#include <freetype/ftbitmap.h>
#include FT_FREETYPE_H
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

void make_char_image(std::vector<std::vector<uchar>> &char_image_data,
                     const char *font_file, const int &width, const std::vector<char> char_list,
                     const double ratio, const int line_space, const int scale=20);

#endif