#ifndef PROCESS_FILE_H
#define PROCESS_FILE_H

#include <iostream>
#include <string>
extern "C"{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
}

void get_basename(std::string &base_name, std::string_view file_path, std::string_view dir_sep);

void get_filename(std::string &file_name);

void mux_video_audio(const char *output_file, const char *orig_file, const char *ascii_file);

void reduce_video_size(const char *input_file, const char *output_file, const double &reduce);

#endif