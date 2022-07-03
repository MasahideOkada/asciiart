#include "parse_arguments.h"
#include "ascii_functions.h"
#include "gamma_correction.h"
#include "make_char_image.h"
#include "process_file.h"

int main(int argc, char *argv[]){
    std::string dir_sep = "/";
    #if defined(_WIN64) || defined(_WIN32)
        dir_sep = "\\";
    #endif
    std::string input_path, output_dir, output_name, policy = "average", orig_pos = "bottom_right";
    std::string font_file = "font" + dir_sep + "NotoSansMono-Regular.ttf";
    int pixel_per_char, line_space = 2, num_lines = 60, char_width = 6;
    double gamma = 1.0, orig_rescale = 0.3, font_ratio = 2.0;
    bool output_dir_given = false, output_name_given = false, do_gamma_correction = false, with_original = false;
    double reduce = 0.1;
    bool do_reduce = true;
    parse_arguments(argc, argv, input_path, 
                    pixel_per_char, line_space, num_lines, char_width,
                    output_dir_given, output_dir, 
                    output_name_given, output_name,
                    do_gamma_correction, gamma, policy, 
                    with_original, orig_rescale, orig_pos, 
                    font_file, font_ratio, 
                    reduce, do_reduce);
    pixel_per_char = char_width;
    std::vector<uchar> lut1(256), lut2(256);
    if (do_gamma_correction){
        for (int i=0; i<256; ++i){
            lut1[i] = (uchar)255 * std::pow((double)i / 255.0, gamma);
            lut2[i] = (uchar)255 * std::pow((double)i / 255.0, 1 / gamma);
        }
    }
    std::string chars_to_use = ".,-~:;=!*#$@";
    std::vector<char> char_list(chars_to_use.size());
    for (int i=0; i<(int)chars_to_use.size(); ++i){
        char_list[i] = chars_to_use[i];
    }
    std::vector<int> intensity_to_char_idx(256);
    make_intensity_to_char_idx(intensity_to_char_idx, (int)char_list.size());
    int char_height = round(font_ratio * (double)char_width);
    std::vector<std::vector<uchar>> char_image_data;
    char_image_data.reserve((int)char_list.size());
    make_char_image(char_image_data, font_file.c_str(), char_width, char_list, 
                    font_ratio, line_space);

    if (argc > 1){
        auto ext_pos = input_path.rfind('.');
        if (ext_pos == std::string::npos){
            std::cout << "File format is unknown" << std::endl;
        }
        else{
            std::string ext = input_path.substr(ext_pos);
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            std::cout << input_path << std::endl;
            
            std::string base_name, output_path;
            get_basename(base_name, input_path, dir_sep);
            std::string file_name = base_name;
            get_filename(file_name);
            if (!output_name_given){
                output_name = file_name + "_ascii";
            }
            if (output_dir_given){
                output_path = output_dir + dir_sep + output_name + ext;
            }
            else{
                output_path = output_name + ext;
            }
            if (ext == ".jpg" || ext == ".jpeg" || ext == ".png"){
                cv::Mat image = cv::imread(input_path, cv::IMREAD_COLOR), orig_image = cv::imread(input_path, cv::IMREAD_COLOR);
                cv::Mat *sub_image = new cv::Mat(pixel_per_char, pixel_per_char, CV_8UC1);
                if (image.empty()){
                    std::cout << "Could not open or find the image" << std::endl;
                }
                else{
                    if (do_gamma_correction){
                        gamma_correction(image, image, lut1, lut2);
                    }
                    else{
                        cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);
                    }
                    int num_rows = image.rows / char_height, num_cols = image.cols / char_width;
                    cv::Mat ascii_image(num_rows * char_height, num_cols * char_width, CV_8UC1, cv::Scalar(0));
                    make_ascii_image(ascii_image, image, *sub_image, 
                                     num_rows, num_cols, char_height, char_width,
                                     char_image_data, intensity_to_char_idx, policy,
                                     with_original, orig_image, orig_rescale, orig_pos);
                    cv::imshow("test", ascii_image);
                    cv::waitKey(0);
                    cv::imwrite(output_path, ascii_image);
                    std::cout << "write file " << output_path << std::endl;
                }
            }
            else if (ext == ".mp4"){
                int remove = 0;
                std::string ascii_path_raw, ascii_path_to_mux;
                if (output_dir_given){
                    ascii_path_raw = output_dir + dir_sep + output_name + "_without_audio" + ext;
                    ascii_path_to_mux = output_dir + dir_sep + output_name + "_without_audio_to_mux" + ext;
                }
                else{
                    ascii_path_raw = output_name + "_without_audio" + ext;
                    ascii_path_to_mux = output_name + "_without_audio_to_mux" + ext;
                }
                make_ascii_video(ascii_path_raw, input_path, num_lines, char_height, char_width, 
                                 char_image_data, intensity_to_char_idx, 
                                 do_gamma_correction, gamma, lut1, lut2, policy, 
                                 with_original, orig_rescale, orig_pos);
                if (do_reduce){
                    std::cout << "reducing file size" << "\n";
                    reduce_video_size(ascii_path_raw.c_str(), ascii_path_to_mux.c_str(), reduce);
                    remove = std::remove(ascii_path_raw.c_str());
                    if (remove == 0){
                        std::cout << ascii_path_raw << " removed" << "\n";
                    }
                }
                else{
                    ascii_path_to_mux = ascii_path_raw;
                }
                mux_video_audio(output_path.c_str(), input_path.c_str(), ascii_path_to_mux.c_str());
                remove = std::remove(ascii_path_to_mux.c_str());
                if (remove == 0){
                    std::cout << ascii_path_to_mux << " removed" << "\n";
                }
                std::cout << "write file " << output_path << "\n";
                std::cout << "done" << std::endl;
            }
        }
    }
    return 0;
}