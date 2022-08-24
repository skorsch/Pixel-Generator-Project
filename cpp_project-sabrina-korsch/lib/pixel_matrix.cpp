#include "../include/pixel_matrix.hpp"

using namespace pixels;

bool pixel_matrix::set_options(const std::string type_n_size)
{
    int pos = type_n_size.find("-");
    int type;

    //get option index to use as enum
    for (int i = 0; i < options.size(); i++)
    {
        //future TODO: make case insensitive?
        if (std::get<0>(options[i]) == type_n_size.substr(0, pos))
        {
            type = std::get<1>(options[i]);
            break;
        }
    }

    //string-like enum use to determine which option to set along with any given value
    switch (type)
    {
    case 1:
    case 2:
    case 3:
    case 4: //cases 1-4 are size reductions
        if (size_red_type == 0)
        {
            size_red_type = type;
            try 
            {
                size_red_num = stoi(type_n_size.substr(pos + 1));
            } 
            catch (...) 
            {
                std::cout << "Error: Numerical input for size reduction unrecognized or not given\n";
                return false;
            }

            if(size_red_num <= 0) 
            {
                std::cout << "Error: Numerical input for size reduction needs to be greater than 0\n";
                return false;
            }
        }
        else
        {
            std::cout << "Warning: Multiple size reduction transformations given, only the first will be applied\n";
        }
        break;
    case 5:
    case 6:
    case 7:
    case 8: //cases 5-8 are color reductions
        if (color_red_type == 0)
        {
            color_red_type = type;
            try 
            {
                color_red_num = stoi(type_n_size.substr(pos + 1));
            } 
            catch (...) 
            {
                std::cout << "Error: Numerical input for color reduction unrecognized or not given\n";
                return false;
            }

            if(color_red_num < 0) 
            {
                std::cout << "Error: Numerical input for color reduction needs to be greater than or equal to 0\n";
                return false;
            }
        }
        else
        {
            std::cout << "Warning: Multiple color reduction transformations given, only the first will be applied\n";
        }
        break;
    case 9:
    case 10: //cases 9-10 are dithers
        if (dither_type == 0)
        {
            dither_type = type;
        }
        else
        {
            std::cout << "Warning: Multiple dither transformations given, only the first will be applied\n";
        }
        break;
    default:
        return false;
    }
    return true;
}

void pixel_matrix::apply_options()
{
    size_reduction();
    //dithering actually happens after color quant palette selection and before color quant conversion
    if (dither_type != 0)
    {
        //dithering requires a color_reduction and thus calls it within the dither method
        dither();
    }
    else
    {
        color_reduction();
    }

    //future TODO: Anti-dither? Remove single pixels
}

void pixel_matrix::write_to_file(const std::string output_file)
{
    if (!imwrite(output_file, _image)){
        std::cout << "Error: Encountered a problem when writing to output file, please try again\n";
    }
}

void pixel_matrix::size_reduction()
{
    cv::Size_ org_size = _image.size();
    double scale_fac = double(size_red_num) / double(org_size.width);
    cv::Size_ new_size(round(scale_fac * _image.cols), round(scale_fac * _image.rows));

    switch (size_red_type)
    {
    case 1:
        cv::resize(_image, _image, new_size, cv::INTER_NEAREST);
        break;
    case 2:
        cv::resize(_image, _image, new_size, cv::INTER_LINEAR);
        break;
    case 3:
        cv::resize(_image, _image, new_size, cv::INTER_CUBIC);
        break;
    case 4:
        cv::resize(_image, _image, new_size, cv::INTER_LANCZOS4);
        break;
    }
}

//color reduction with arg
std::vector<cv::Vec3b> pixel_matrix::color_reduction()
{

    switch (color_red_type)
    {
    case 5:
        return uniform(_image, color_red_num);
    case 6:
        return popularity(_image, color_red_num);
    case 7:
        return median_cut(_image, color_red_num);
    case 8:
        return kmeans(_image, color_red_num);
    }
    std::vector<cv::Vec3b> pal;
    return pal;
}

//dither with arg
void pixel_matrix::dither()
{
    if (color_red_type == 0)
    {
        std::cout << "Warning: A dithering transformation requires a color reduction, but none was given, using a default of kmeans-8\n";
        color_red_type = 8;
        color_red_num = 8;
    }

    std::vector<cv::Vec3b> pal;
    dithered_image = _image.clone();
    switch (dither_type)
    {
    case 9:
        //better results when color reduce original image and apply that palette to dithered image
        pal = color_reduction();
        bayer(dithered_image);
        external_palette_replacement(dithered_image, pal);
        _image = dithered_image;
        break;
    case 10:
        //for atkinson color reduction happens as we dither so we can determine error and diffuse it
        pal = color_reduction();
        atkinson(dithered_image, _image, pal);
        _image = dithered_image;
        break;
    }
}