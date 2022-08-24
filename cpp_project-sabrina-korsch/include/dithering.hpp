#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include <vector>
#include <tuple>
#include <iostream>
#include <math.h>
#include <algorithm>

namespace pixels
{  
    //  Bayer -- currently just the 4x4 bayer ordered dithering type
    //Skews the pixel colors using a specific pattern such that mapping the palette, created from color quantization, to the skewed matrix results in some pixels mapping to a different color in the palette
    //For more details visit: https://en.wikipedia.org/wiki/Ordered_dithering
    void bayer(cv::Mat &image);

    // Atkinson -- a type  of error diffusion dithering
    //Spreads out the per pixel error (difference between true color and palette color from color quant) to the surrounding pixels in a specific pattern; preserves 3/4ths of this error
    //This will cause pixel colors to skew and get mapped to a different color in the palette
    //For more details visit: https://beyondloom.com/blog/dither.html
    void atkinson(cv::Mat &image, cv::Mat &image_c_reduc, const std::vector<cv::Vec3b> &palette);

}