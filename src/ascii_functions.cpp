#include "ascii_functions.h"

void make_intensity_to_char_idx(std::vector<int> &intensity_to_char_idx, const int &num_chars){
    int unit = 256 / num_chars, *idx = new int[1];
    for (int i=0; i<256; ++i){
        *idx = i / unit;
        if (*idx > num_chars - 1){
            *idx = num_chars - 1;
        }
        intensity_to_char_idx[i] = *idx;
    }
}

void make_ascii_image(cv::Mat &ascii_image, cv::Mat &input_image, cv::Mat &sub_image, 
                      const int &num_rows, const int &num_cols, const int &char_height, const int &char_width,
                      std::vector<std::vector<uchar>> &char_image_data, const std::vector<int> &intensity_to_char_idx,
                      std::string_view policy, const bool &with_original, cv::Mat &orig_image, const double &orig_rescale,
                      std::string_view orig_pos){
    int *x = new int[1], *y = new int[1], *intensity = new int[1], *char_idx = new int[1];
    double *max = new double[1], *min = new double[1];
    std::string_view average_policy{"average"}, max_policy{"max"}, min_policy{"min"};
    std::string_view top_left{"top_left"}, top_right{"top_right"}, bottom_left{"bottom_left"}, bottom_right{"bottom_right"};
    for (int r=0; r<num_rows; ++r){
        *y = char_height * r;
        for (int c=0; c<num_cols; ++c){
            *x = char_width * c;
            sub_image = input_image(cv::Rect(*x, *y, char_width, char_height));
            if (policy == average_policy){
                *intensity = cv::mean(sub_image).val[0];
            }
            else if (policy == max_policy){
                cv::minMaxLoc(sub_image, NULL, max, NULL, NULL);
                *intensity = (int)*max;
            }
            else{
                cv::minMaxLoc(sub_image, min, NULL, NULL, NULL);
                *intensity = (int)*min;
            }
            *char_idx = intensity_to_char_idx[*intensity];
            cv::Mat ascii_sub_image = ascii_image.rowRange(char_height * r, char_height * (r + 1)).colRange(char_width * c, char_width * (c + 1));
            cv::Mat char_image(char_height, char_width, CV_8UC1, char_image_data[*char_idx].data());
            char_image.copyTo(ascii_sub_image);
        }
    }
    cv::cvtColor(ascii_image, ascii_image, cv::COLOR_GRAY2BGR);
    if (with_original){
        cv::Mat ascii_orig_image;
        int ascii_rows = ascii_image.rows, ascii_cols = ascii_image.cols;
        int orig_rows= round(orig_rescale * (double)ascii_rows), orig_cols = round(orig_rescale * (double)ascii_cols);
        cv::resize(orig_image, orig_image, cv::Size(orig_cols, orig_rows), 0, 0, cv::INTER_CUBIC);
        if (orig_pos == top_left){
            ascii_orig_image = ascii_image.rowRange(0, orig_rows).colRange(0, orig_cols);
        }
        else if(orig_pos == top_right){
            ascii_orig_image = ascii_image.rowRange(0, orig_rows).colRange(ascii_cols - orig_cols, ascii_cols);
        }
        else if(orig_pos == bottom_left){
            ascii_orig_image = ascii_image.rowRange(ascii_rows - orig_rows, ascii_rows).colRange(0, orig_cols);
        }
        else if(orig_pos == bottom_right){
            ascii_orig_image = ascii_image.rowRange(ascii_rows - orig_rows, ascii_rows).colRange(ascii_cols - orig_cols, ascii_cols);
        }
        orig_image.copyTo(ascii_orig_image);
    }
}

void make_ascii_video(const std::string &output_path, const std::string &video_path, const int &num_lines,
                      const int &char_height, const int &char_width,
                      std::vector<std::vector<uchar>> &char_image_data, const std::vector<int> &intensity_to_char_idx,
                      const bool &do_gamma_correction, const double &gamma, const std::vector<uchar> &lut1, const std::vector<uchar> &lut2,
                      std::string_view policy, const bool &with_original, const double &orig_rescale,
                      std::string_view orig_pos){
    cv::VideoCapture cap(video_path);
    double fps = cap.get(cv::CAP_PROP_FPS);
    int rows_orig = cap.get(cv::CAP_PROP_FRAME_HEIGHT), cols_orig = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int num_rows = rows_orig / char_height, num_cols = cols_orig / char_width;
    cv::Mat frame, ascii_frame(num_rows * char_height, num_cols * char_width, CV_8UC1, cv::Scalar(0));
    cv::Mat *sub_frame = new cv::Mat(char_height, char_width, CV_8UC1);
    cv::Mat orig_frame;
    std::vector<std::string> pbar_list(51);
    make_pbar(pbar_list);
    int *progress = new int[1], frame_count = 0, total = cap.get(cv::CAP_PROP_FRAME_COUNT);
    int count = 0;
    cv::VideoWriter output;
    int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    output.open(output_path, fourcc, fps, ascii_frame.size());
    while (true){
        cap.read(frame);
        if (frame.empty()){
            break;
        }
        ++count;
        frame.copyTo(orig_frame);
        ++frame_count;
        *progress = 100 * frame_count / total;
        std::cout << "rendering ";
        if (*progress < 10){
            std::cout << "  ";
        }
        if (*progress >= 10 && *progress < 100){
            std::cout << " ";
        }
        std::cout << *progress << "% " << std::string_view(pbar_list[*progress / 2]) << "\r";

        if (do_gamma_correction){
            gamma_correction(frame, frame, lut1, lut2);
        }
        else{
            cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
        }
        make_ascii_image(ascii_frame, frame, *sub_frame, num_rows, num_cols, char_height, char_width,
                         char_image_data, intensity_to_char_idx, policy, 
                         with_original, orig_frame, orig_rescale, orig_pos);
        output.write(ascii_frame);
        cv::cvtColor(ascii_frame, ascii_frame, cv::COLOR_BGR2GRAY, cv::INTER_CUBIC);
    }
    cap.release();
    output.release();
    std::cout << "\nwrite file " << output_path << std::endl;
}

void make_pbar(std::vector<std::string> &pbar_list){
    std::string *p = new std::string;
    for (int i=0; i<51; ++i){
        (*p).push_back('|');
        for (int j=0; j<i; ++j){
            (*p).push_back('#');
        }
        for (int j=i; j<50; ++j){
            (*p).push_back(' ');
        }
        (*p).push_back('|');
        pbar_list[i] = *p;
        (*p).clear();
    }
}