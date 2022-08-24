#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/core/types_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <vector>
#include <tuple>
#include <iostream>

#include "./color_reductions.hpp"
#include "./dithering.hpp"


namespace pixels {

    class pixel_matrix
    {
        public:

        //constructs pixel_matrix object with input image
        pixel_matrix(const char* input) : _image(cv::imread(input)), size_red_type(0), color_red_type(0), dither_type(0) {}

        //returns true if there is no image data - happens if image file is empty or not found
        bool is_empty() { return !_image.data; }

        //parses a string that contains an given transformation option and it's value if applicable, sets internal option values appropriately
        //returns false if string contains an unrecognized string
        bool set_options(const std::string type_n_size);

        //applies internally set transformation options in order
        void apply_options();

        //outputs final image to a file name that's given
        void write_to_file(const std::string output_file);

        private:
        //underlying matricies
        cv::Mat _image;
 
        //need orig matrix for color sampling and error
        cv::Mat dithered_image;

        //acts as a string enum for the options
        // 1-4 = size_red, 5-8 = color_red, 9-10 = dithering
        const std::vector<std::tuple<std::string, int>> options {   {"nearest_neighbor", 1}, {"bilinear", 2}, {"bicubic", 3}, {"lanczos", 4}, 
                                                                    {"uniform", 5}, {"popularity", 6}, {"median_cut", 7}, {"kmeans", 8},
                                                                    {"bayer", 9}, {"atkinson", 10}  };
        //internal tranformation option values
        //only 3 different transformations can be applied per run
        int size_red_type;
        int size_red_num;
        int color_red_type;
        int color_red_num;
        int dither_type;

        //call appropriate size reduction algorithm depending on internal option value
        //**uses ONLY OpenCV library function and types
        void size_reduction();

        //call appropriate color reduction algorithm depending on internal option value, returns a color palette of final colors
        std::vector<cv::Vec3b> color_reduction();

        //call appropriate dither algorithm depending on internal option value
        void dither();

    };        
}