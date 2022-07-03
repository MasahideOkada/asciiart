#include "make_char_image.h"

void make_char_image(std::vector<std::vector<uchar>> &char_image_data,
                     const char *font_file, const int &width, const std::vector<char> char_list,
                     const double ratio, const int line_space, const int scale){
    int target_width = width;
    int target_height = round(ratio * (double)target_width);
    int WIDTH = scale * target_width;
    int HEIGHT = round(ratio * (double)WIDTH);
    FT_Library library;
    FT_Face face;
    FT_GlyphSlot slot;
    FT_Error error;
    error = FT_Init_FreeType(&library);
    if (error){
        std::cout << "error" << std::endl;
    }
    error = FT_New_Face(library, font_file, 0, &face);
    if (error == FT_Err_Unknown_File_Format){
        std::cout << "unknown format" << std::endl;
    }
    else if(error){
        std::cout << "error" << std::endl;
    }
    error = FT_Set_Pixel_Sizes(face, WIDTH, HEIGHT);
    //check metrics 
    int num_char = (int)char_list.size();
    int max_width, max_height, min_left;
    std::vector<int> width_list(num_char), height_list(num_char);
    std::vector<int> top_list(num_char), lef_list(num_char);
    for (int i=0; i<num_char; ++i){
        FT_ULong ch = char_list[i];
        FT_UInt index = FT_Get_Char_Index(face, ch);
        error = FT_Load_Glyph(face, index, FT_LOAD_RENDER);
        if (error){
            std::cout << "glyph loading error" << std::endl;
        }
        slot = face->glyph;
        FT_Render_Glyph(slot, FT_RENDER_MODE_MONO);
        int char_width = slot->bitmap.width, char_height = slot->bitmap.rows;
        int char_top = slot->bitmap_top, char_lef = slot->bitmap_left;
        width_list[i] = char_width;
        height_list[i] = char_height;
        top_list[i] = char_top;
        lef_list[i] = char_lef;
        FT_Bitmap_Done(library, &slot->bitmap);
    }
    // make a model bitmap of each charcter
    max_width = *std::max_element(width_list.begin(), width_list.end());
    max_height = *std::max_element(height_list.begin(), height_list.end());
    min_left = *std::min_element(lef_list.begin(), lef_list.end());
    int base_char = std::distance(height_list.begin(), std::find(height_list.begin(), height_list.end(), max_height));
    int base_line = top_list[base_char];
    int base_bottom = max_height - base_line;
    std::vector<std::vector<uchar>> bitmap_vector;
    bitmap_vector.reserve(num_char);
    for (int i=0; i<num_char; ++i){
        FT_ULong ch = char_list[i];
        FT_UInt index = FT_Get_Char_Index(face, ch);
        error = FT_Load_Glyph(face, index, FT_LOAD_RENDER);
        if (error){
            std::cout << "glyph loading error" << std::endl;
        }
        slot = face->glyph;
        FT_Render_Glyph(slot, FT_RENDER_MODE_MONO);
        int trans_x = 0, trans_y = 0;
        int top = slot->bitmap_top, left = slot->bitmap_left;
        int width = slot->bitmap.width, height = slot->bitmap.rows;
        int bottom = height - top;
        int descender = bottom - base_bottom;
        trans_x = left - min_left;
        if (descender > 0){
            trans_y = - descender;
        }
        std::vector<uchar> render_vector(height * width);
        for (int pixel=0; pixel<width*height; ++pixel){
            if (slot->bitmap.buffer[pixel] == 0){
                render_vector[pixel] = (uchar)0;
            }
            else{
                render_vector[pixel] = (uchar)255;
            }
        }
        std::vector<uchar> char_vector(max_height * max_width, 0);
        int start_row = base_line - top + trans_y;
        int end_row = base_line - top + height + trans_y;
        int start_col = trans_x;
        int end_col = trans_x + width;
        int idx = 0, IDX = 0;
        for (int row=0; row<max_height; ++row){
            for (int col=0; col<max_width; ++col){
                if ((col >= start_col) && (col < end_col) && (row >= start_row) && (row < end_row)){
                    char_vector[IDX] = render_vector[idx];
                    ++idx;
                }
                ++IDX;
            }
        }
        bitmap_vector.push_back(char_vector);
        FT_Bitmap_Done(library, &slot->bitmap);
    }
    // resize those model bitmaps to the target size
    int top_space = line_space / 2;
    int bot_space = line_space - top_space;
    for (int i=0; i<num_char; ++i){
        cv::Mat C = cv::Mat(max_height, max_width, CV_8UC1, bitmap_vector[i].data());
        cv::resize(C, C, cv::Size(target_width, target_height - line_space), 0, 0, cv::INTER_AREA);
        cv::Mat T = cv::Mat(target_height, target_width, CV_8UC1, cv::Scalar(0));
        C.copyTo(T.rowRange(top_space, target_height - bot_space));
        int pixel_idx = 0;
        std::vector<uchar> bitmap_data(target_height * target_width, 0);
        for (int r=0; r<target_height; ++r){
            for (int c=0; c<target_width; ++c){
                uchar value = T.at<uchar>(r, c);
                bitmap_data[pixel_idx] = value;
                ++pixel_idx;
            }
        }
        char_image_data.push_back(bitmap_data);
    }
    FT_Done_Face(face);
    FT_Done_FreeType(library);
}