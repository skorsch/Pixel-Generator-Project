#include "../include/dithering.hpp"

void pixels::bayer(cv::Mat &image)
{
    //future TODO: allow bayer size type to be variable
    int b_n = 4;
    //4x4 bayer pattern
    std::vector<std::vector<float> > b_matrix = {{-0.5, 0, -0.375, 0.125},
                                                 {0.25, -0.25, 0.375, -0.125},
                                                 {-0.3125, 0.1875, -0.4375, 0.0625},
                                                 {0.4375, -0.0625, 0.3125, -0.1875}};
    const float b_r = float(255) / float(3);

    for (int i = 0; i < image.rows; i++)
    {
        for (int j = 0; j < image.cols; j++)
        {
            //apply bayer value to shift pixel values
            const float b_value = b_matrix[i % b_n][j % b_n];
            const int add_val = int(round(b_r * b_value));

            const int r_val = int(image.at<cv::Vec3b>(i, j).val[2]) + add_val;
            const int g_val = int(image.at<cv::Vec3b>(i, j).val[1]) + add_val;
            const int b_val = int(image.at<cv::Vec3b>(i, j).val[0]) + add_val;

            //truncate max, min values
            image.at<cv::Vec3b>(i, j).val[2] = r_val < int(0) ? u_char(0) : r_val > int(255) ? u_char(255)
                                                                                             : u_char(r_val);
            image.at<cv::Vec3b>(i, j).val[1] = g_val < int(0) ? u_char(0) : g_val > int(255) ? u_char(255)
                                                                                             : u_char(g_val);
            image.at<cv::Vec3b>(i, j).val[0] = b_val < int(0) ? u_char(0) : b_val > int(255) ? u_char(255)
                                                                                             : u_char(b_val);
        }
    }
}

//helper method for atkinson
cv::Vec3b closest_color(const cv::Vec3b &pixel, const std::vector<cv::Vec3b> &palette)
{
    double curr_min_dist = double(0);
    cv::Vec3b final_vec = palette[0];
    //iterate through chosen colors to find closest one (euclidean distance) and replace as pixel color
    for (int k = 0; k < palette.size(); k++)
    {
        const cv::Vec3i curr_vec = palette[k];
        const double dist = sqrt(double(pow((pixel.val[2] - curr_vec.val[2]), 2)) + double(pow((pixel.val[1] - curr_vec.val[1]), 2)) + double(pow((pixel.val[0] - curr_vec.val[0]), 2))); //calc euc dist
        if (dist < curr_min_dist || k == 0)
        {
            curr_min_dist = dist;
            final_vec = curr_vec;
        }
    }
    return final_vec;
}

void pixels::atkinson(cv::Mat &image, cv::Mat &image_c_reduc, const std::vector<cv::Vec3b> &palette)
{
    cv::Mat error(image.rows, image.cols, CV_32FC3, cv::Scalar(0));
    std::vector<int> plus_pos = {1, 2, image.cols - 1, image.cols, image.cols + 1, 2 * image.cols};

    for (int i = 0; i < image.rows; i++)
    {
        for (int j = 0; j < image.cols; j++)
        {
            //calculate value with error
            const int pos = i * image.cols + j;
            const int r_val = int(image.at<cv::Vec3b>(pos).val[2]) + round(error.at<cv::Vec3f>(pos).val[2]);
            const int g_val = int(image.at<cv::Vec3b>(pos).val[1]) + round(error.at<cv::Vec3f>(pos).val[1]);
            const int b_val = int(image.at<cv::Vec3b>(pos).val[0]) + round(error.at<cv::Vec3f>(pos).val[0]);

            //truncate max, min values
            image.at<cv::Vec3b>(pos).val[2] = r_val < int(0) ? u_char(0) : r_val > int(255) ? u_char(255)
                                                                                            : u_char(r_val);
            image.at<cv::Vec3b>(pos).val[1] = g_val < int(0) ? u_char(0) : g_val > int(255) ? u_char(255)
                                                                                            : u_char(g_val);
            image.at<cv::Vec3b>(pos).val[0] = b_val < int(0) ? u_char(0) : b_val > int(255) ? u_char(255)
                                                                                            : u_char(b_val);
            //create error from orig value - color quant value
            const cv::Vec3b old_pix = image.at<cv::Vec3b>(pos);
            const cv::Vec3b new_pix = closest_color(old_pix, palette);
            image.at<cv::Vec3b>(pos) = new_pix;

            const float diff_r = int(old_pix.val[2]) - int(new_pix.val[2]);
            const float diff_g = int(old_pix.val[1]) - int(new_pix.val[1]);
            const float diff_b = int(old_pix.val[0]) - int(new_pix.val[0]);

            //diffuse error according to algorithm pattern
            for (int k = 0; k < plus_pos.size(); k++)
            {
                if (pos + plus_pos[k] < image.rows * image.cols && pos + plus_pos[k] > 0)
                {
                    error.at<cv::Vec3f>(pos + plus_pos[k]).val[2] += diff_r / float(8);
                    error.at<cv::Vec3f>(pos + plus_pos[k]).val[1] += diff_g / float(8);
                    error.at<cv::Vec3f>(pos + plus_pos[k]).val[0] += diff_b / float(8);
                }
            }
        }
    }
}
