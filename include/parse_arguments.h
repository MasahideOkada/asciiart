#ifndef PARSE_ARGUMENTS_H
#define PARSE_ARGUMENTS_H

#include <iostream>
#include <string>
#include <cstring>

void parse_arguments(int argc, char *argv[], 
                     std::string &path, 
                     int &pixel_per_char, int &line_space,
                     int &num_lines, int &char_width, 
                     bool &output_dir_given, std::string &output_dir,
                     bool &output_name_given, std::string &output_name,
                     bool &do_gamma_correction, double &gamma, std::string &policy, 
                     bool &with_original, double &orig_rescale, std::string &orig_pos, 
                     std::string &font_file, double &font_ratio, 
                     double &reduce, bool &do_reduce);

#endif