#include "parse_arguments.h"

void parse_arguments(int argc, char *argv[], 
                     std::string &path, 
                     int &pixel_per_char, int &line_space,
                     int &num_lines, int &char_width, 
                     bool &output_dir_given, std::string &output_dir,
                     bool &output_name_given, std::string &output_name,
                     bool &do_gamma_correction, double &gamma, std::string &policy, 
                     bool &with_original, double &orig_rescale, std::string &orig_pos, 
                     std::string &font_file, double &font_ratio, 
                     double &reduce, bool &do_reduce){
    if (argc == 1){
        std::cout << "give a path to an image or a video" << std::endl;
    }
    else{
        path = argv[1];
        for (int i=0; i<argc; ++i){
            if (strcmp(argv[i], "-p") == 0 && argv[i+1]){
                pixel_per_char = atoi(argv[i+1]);
            }
            if (strcmp(argv[i], "-ls") == 0 && argv[i+1]){
                line_space = atoi(argv[i+1]);
            }
            if (strcmp(argv[i], "-ln") == 0 && argv[i+1]){
                num_lines = atoi(argv[i+1]);
            }
            if (strcmp(argv[i], "-o") == 0 && argv[i+1]){
                output_dir_given = true;
                output_dir = argv[i+1];
            }
            if (strcmp(argv[i], "-n") == 0 && argv[i+1]){
                output_name_given = true;
                output_name = argv[i+1];
            }
            if (strcmp(argv[i], "-char_width") == 0 && argv[i+1]){
                char_width = atoi(argv[i+1]);
            }
            if (strcmp(argv[i], "-gamma") == 0 && argv[i+1]){
                do_gamma_correction = true;
                gamma = strtod(argv[i+1], NULL);
            }
            if (strcmp(argv[i], "-policy") == 0 && argv[i+1]){
                policy = argv[i+1];
            }
            if (strcmp(argv[i], "-with_original") == 0){
                with_original = true;
            }
            if (strcmp(argv[i], "-orig_rescale") == 0 && argv[i+1]){
                orig_rescale = strtod(argv[i+1], NULL);
            }
            if (strcmp(argv[i], "-pos") == 0 && argv[i+1]){
                orig_pos = argv[i+1];
            }
            if (strcmp(argv[i], "-font") == 0 && argv[i+1]){
                font_file = argv[i+1];
            }
            if (strcmp(argv[i], "-font_ratio") == 0 && argv[i+1]){
                double aspect_ratio = strtod(argv[i+1], NULL);
                font_ratio = 1 / aspect_ratio;
            }
            if (strcmp(argv[i], "-reduce") == 0 && argv[i+1]){
                if (strcmp(argv[i+1], "no") == 0){
                    do_reduce = false;
                }
                else{
                    reduce = strtod(argv[i+1], NULL);
                }
            }
        }
    }
}