#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include <vector>
#include <tuple>
#include <iostream>
#include <math.h>
#include <algorithm>

namespace pixels
{  
    //Taking into consideration the use case of this software where artists will most likely continue modification in drawing software afterwards:
    //num_colors = 1 and 0 does not make much sense as an input, but will return a white and black image respectively
    //num_colors = 2 will be black and white (special case)

    //For more detailed descriptions of these algorithms visit: https://web.cs.wpi.edu/~matt/courses/cs563/talks/color_quant/CQindex.html

    //  Uniform
    //Works best when the prime factorization of num_colors results in a set of numbers whose length is a multiple of 3 and are all the same number, ex. 2 2 2 2 2 2 
    //RGB color space will try to be divided evenly but for non even divides:
        //get list of sorted prime factors
        //group list into 3, last group may have less numbers if array length not divisible by 3
        //where r * g * b = num_colors
    //Uneven divides results in color skewing
    //Since color space is divided uniformly, if a color region has no representative colors then it will be empty, resulting in an output with less colors than num_colors
    std::vector<cv::Vec3b> uniform(cv::Mat &image, const int num_colors);

    //  Popularity
    //Doesn't experience color skewing like uniform does, can use any input for num_colors
    //Has the most overhead
    //Doesn't work well at low num_colors if there are is a lot of light area with a bunch of similar colors, vs less pixels of dark area
    std::vector<cv::Vec3b> popularity(cv::Mat &image, const int num_colors);

    //  Median_cut
    //Requires a num_colors that is a power of 2
    //If not provided, a warning will choose the first power of 2 smaller than the given value
    std::vector<cv::Vec3b> median_cut(cv::Mat &image, int num_colors);

    //  Kmeans
    //Implemented according to OpenCV documentation where the kmeans algorithm is already implemented
    std::vector<cv::Vec3b> kmeans(cv::Mat &image, const int num_colors);

    //future TODO: Octree
    //void octree (cv::Mat& image, const int num_colors);

    //Takes an image and palette and replaces pixels in the images with their closest (euclidian distance) palette color
    //Helper method for both dithering and color_reduction methods
    void external_palette_replacement(cv::Mat &image, std::vector<cv::Vec3b> &palette);
}