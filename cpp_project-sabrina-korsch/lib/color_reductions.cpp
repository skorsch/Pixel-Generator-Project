#include "../include/color_reductions.hpp"

//helper to create a prime factor list for uniform color reduction
void prime_fac(std::vector<int> &factors, int num)
{
    //get all 2 factors
    int i = 2;
    while (num % i == 0)
    {
        factors.push_back(i);
        num /= i;
    }

    //get all other factors
    i++;
    while (i <= sqrt(num))
    {
        while (num % i == 0)
        {
            factors.push_back(i);
            num /= i;
        }
        i += 2;
    }

    //edge case where number is prime
    if (num > 2)
    {
        factors.push_back(num);
    }
}

//helper method for the 0, 1, 2 num_color cases
std::vector<cv::Vec3b> special_cases(cv::Mat &image, const int num_colors)
{
    std::vector<cv::Vec3b> sm_palette;
    switch (num_colors)
    {
    case 0:
        sm_palette.push_back(cv::Vec3b(0, 0, 0));
        break;
    case 1:
        sm_palette.push_back(cv::Vec3b(255, 255, 255));
        break;
    case 2:
        sm_palette.push_back(cv::Vec3b(0, 0, 0));
        sm_palette.push_back(cv::Vec3b(255, 255, 255));
    }
    pixels::external_palette_replacement(image, sm_palette);
    return sm_palette;
}

std::vector<cv::Vec3b> pixels::uniform(cv::Mat &image, const int num_colors)
{
    //0, 1, 2 num_colors cases
    if (num_colors < 3)
    {
        return special_cases(image, num_colors);
    }

    //get sorted prime factors
    std::vector<int> factors;
    prime_fac(factors, num_colors);

    //split factors to create 3 factors of original number, use to divide RGB color space
    //TODO: find some better way to distribute prime factor values?
    int partition = factors.size() / 3;
    int partition_first = factors.size() - (partition * 2); //the larger partition gets the smaller numbers
    int r_num_colors = 1;
    int g_num_colors = 1;
    int b_num_colors = 1;

    for (int i = 0; i < partition_first; i++)
    {
        r_num_colors *= factors[i];

        if (i < partition)
        {
            g_num_colors *= factors[i + partition_first];
            b_num_colors *= factors[i + partition_first + partition];
        }
    }

    //color regions to calc representative color
    //tuples of <sum, num> for later computing averages for each region
    std::vector<std::tuple<int, int> > r_regions(r_num_colors, std::tuple<int, int>(0, 0));
    std::vector<std::tuple<int, int> > g_regions(g_num_colors, std::tuple<int, int>(0, 0));
    std::vector<std::tuple<int, int> > b_regions(b_num_colors, std::tuple<int, int>(0, 0));

    //the *_num_colors vars now rep how many times the given channel is partitioned, partitions 0 to *_num_colors
    //can calculate section number for red via round(double(r_value) / double(255) * (r_num_colors - 1))
    for (int i = 0; i < image.rows; i++)
    {
        for (int j = 0; j < image.cols; j++)
        {
            const int r_value = image.at<cv::Vec3b>(i, j).val[2];
            const int g_value = image.at<cv::Vec3b>(i, j).val[1];
            const int b_value = image.at<cv::Vec3b>(i, j).val[0];

            //compute region index
            const int r_reg = round(double(r_value) / double(255) * (r_num_colors - 1));
            const int g_reg = round(double(g_value) / double(255) * (g_num_colors - 1));
            const int b_reg = round(double(b_value) / double(255) * (b_num_colors - 1));

            //add values to region average arrays
            std::get<0>(r_regions[r_reg]) += r_value;
            std::get<1>(r_regions[r_reg])++;
            std::get<0>(g_regions[g_reg]) += g_value;
            std::get<1>(g_regions[g_reg])++;
            std::get<0>(b_regions[b_reg]) += b_value;
            std::get<1>(b_regions[b_reg])++;

            //convert image values to region index values (to be transformed back into color values once averages are found)
            image.at<cv::Vec3b>(i, j).val[2] = r_reg;
            image.at<cv::Vec3b>(i, j).val[1] = g_reg;
            image.at<cv::Vec3b>(i, j).val[0] = b_reg;
        }
    }

    //find averages -- values the image will get converted to
    for (int i = 0; i < r_regions.size(); i++)
    {
        if (std::get<1>(r_regions[i]) != 0) //check for division by 0
        {
            std::get<0>(r_regions[i]) = round(double(std::get<0>(r_regions[i])) / double(std::get<1>(r_regions[i])));
        }
    }
    for (int i = 0; i < g_regions.size(); i++)
    {
        if (std::get<1>(g_regions[i]) != 0)
        {
            std::get<0>(g_regions[i]) = round(double(std::get<0>(g_regions[i])) / double(std::get<1>(g_regions[i])));
        }
    }
    for (int i = 0; i < b_regions.size(); i++)
    {
        if (std::get<1>(r_regions[i]) != 0)
        {
            std::get<0>(b_regions[i]) = round(double(std::get<0>(b_regions[i])) / double(std::get<1>(b_regions[i])));
        }
    }

    //convert img back to color value
    std::vector<cv::Vec3b> palette;
    for (int i = 0; i < image.rows; i++)
    {
        for (int j = 0; j < image.cols; j++)
        {
            auto r = std::get<0>(r_regions[image.at<cv::Vec3b>(i, j).val[2]]);
            auto g = std::get<0>(g_regions[image.at<cv::Vec3b>(i, j).val[1]]);
            auto b = std::get<0>(b_regions[image.at<cv::Vec3b>(i, j).val[0]]);
            image.at<cv::Vec3b>(i, j).val[2] = r;
            image.at<cv::Vec3b>(i, j).val[1] = g;
            image.at<cv::Vec3b>(i, j).val[0] = b;

            const cv::Vec3b combo(b, g, r);
            if (std::find(palette.begin(), palette.end(), combo) == palette.end())
            {
                palette.push_back(combo);
            }
        }
    }
    return palette;
}

std::vector<cv::Vec3b> pixels::popularity(cv::Mat &image, const int num_colors)
{
    //0, 1, 2 num_colors cases
    if (num_colors < 3)
    {
        return special_cases(image, num_colors);
    }

    //divides 256*256*256 color space into preset 8*8*8 regions
    //will be used for averages and popularity later <sum, num, index, chosen>
    const int region_size = 8;
    const int region_num = 256 / region_size;
    std::vector<std::tuple<cv::Vec3i, int, int, bool> > regions(region_num * region_num * region_num, std::tuple<cv::Vec3i, int, int, bool>(cv::Vec3b(0, 0, 0), 0, 0, false));

    for (int i = 0; i < image.rows; i++)
    {
        for (int j = 0; j < image.cols; j++)
        {
            const int r_value = image.at<cv::Vec3b>(i, j).val[2];
            const int g_value = image.at<cv::Vec3b>(i, j).val[1];
            const int b_value = image.at<cv::Vec3b>(i, j).val[0];

            //compute region index
            const int r_reg = int(double(r_value) / double(256) * (region_num));
            const int g_reg = int(double(g_value) / double(256) * (region_num));
            const int b_reg = int(double(b_value) / double(256) * (region_num));

            //add values to region average arrays -- indexing 3d array for color space
            std::get<0>(regions[r_reg + (g_reg * region_num) + (b_reg * region_num * region_num)]) += image.at<cv::Vec3b>(i, j);
            std::get<1>(regions[r_reg + (g_reg * region_num) + (b_reg * region_num * region_num)])++;
            std::get<2>(regions[r_reg + (g_reg * region_num) + (b_reg * region_num * region_num)]) = r_reg + (g_reg * region_num) + (b_reg * region_num * region_num);

            //convert image values to region index values (to be transformed back into color values once averages are found)
            //works with uchar since index values never go above 31
            image.at<cv::Vec3b>(i, j).val[2] = r_reg;
            image.at<cv::Vec3b>(i, j).val[1] = g_reg;
            image.at<cv::Vec3b>(i, j).val[0] = b_reg;
        }
    }

    for (int i = 0; i < regions.size(); i++)
    {
        if (std::get<1>(regions[i]) != 0)
        {
            //find averages of regions
            std::get<0>(regions[i]).val[2] = round(double(std::get<0>(regions[i]).val[2]) / double(std::get<1>(regions[i])));
            std::get<0>(regions[i]).val[1] = round(double(std::get<0>(regions[i]).val[1]) / double(std::get<1>(regions[i])));
            std::get<0>(regions[i]).val[0] = round(double(std::get<0>(regions[i]).val[0]) / double(std::get<1>(regions[i])));
        }
    }

    //make copy to perserve indicies
    std::vector<std::tuple<cv::Vec3i, int, int, bool> > regions_sorted = regions;

    //sort by popularity
    sort(regions_sorted.begin(), regions_sorted.end(), [](const std::tuple<cv::Vec3i, int, int, bool> a, const std::tuple<cv::Vec3i, int, int, bool> b)
         { return std::get<1>(a) > std::get<1>(b); });

    //set chosen regions in original array
    std::vector<cv::Vec3b> palette;
    for (int i = 0; i < num_colors; i++)
    {
        palette.push_back(std::get<0>(regions_sorted[i]));
        std::get<3>(regions[std::get<2>(regions_sorted[i])]) = true;
    }

    //convert image matrix back to color values
    //if region index is chosen, replace img index with that regions vector value
    //if not chosen, find closest euclidian chosen, replace that region with those values, mark as chosen, replace img index with that vector
    for (int i = 0; i < image.rows; i++)
    {
        for (int j = 0; j < image.cols; j++)
        {
            const int r_reg = image.at<cv::Vec3b>(i, j).val[2];
            const int g_reg = image.at<cv::Vec3b>(i, j).val[1];
            const int b_reg = image.at<cv::Vec3b>(i, j).val[0];
            const int region_index = r_reg + (g_reg * region_num) + (b_reg * region_num * region_num); //can just use one of the rgb values (all the same)
            if (std::get<3>(regions[region_index]) == true)                                            //chosen value
            {
                image.at<cv::Vec3b>(i, j) = std::get<0>(regions[region_index]);
            }
            else //not chosen value
            {
                const cv::Vec3i old_vec = std::get<0>(regions[region_index]);
                double curr_min_dist = double(0);
                //iterate through chosen colors to find closest one (euclidean distance)
                for (int k = 0; k < num_colors; k++)
                {
                    const cv::Vec3i curr_vec = std::get<0>(regions_sorted[k]);
                    const double dist = sqrt(double(pow((old_vec.val[2] - curr_vec.val[2]), 2)) + double(pow((old_vec.val[1] - curr_vec.val[1]), 2)) + double(pow((old_vec.val[0] - curr_vec.val[0]), 2))); //calc euc dist
                    if (dist < curr_min_dist || k == 0)
                    {
                        curr_min_dist = dist;
                        std::get<0>(regions[region_index]) = curr_vec;
                    }
                }

                std::get<3>(regions[region_index]) = true;
                image.at<cv::Vec3b>(i, j) = std::get<0>(regions[region_index]);
            }
        }
    }
    return palette;
}

//only call with num_colors >=2
void rec_median_cut(std::vector<std::tuple<cv::Vec3i, int, int> > &image, std::vector<cv::Vec3b> &palette, int num_colors, const int start, const int end)
{
    if (num_colors <= 1)
    {
        //compute averages and replace colors
        const int size = end - start;
        cv::Vec3i sum_vec(0, 0, 0);
        for (int i = start; i < end; i++)
        {
            sum_vec += std::get<0>(image[i]);
        }

        //computer av vector
        sum_vec /= size;
        palette.push_back(sum_vec);

        for (int i = start; i < end; i++)
        {
            //replace
            std::get<0>(image[i]) = sum_vec;
        }
    }
    else
    {
        auto sub_start = image.begin() + start;
        auto sub_end = image.begin() + end;
        const int max_r = std::get<0>(*max_element(sub_start, sub_end, [](const std::tuple<cv::Vec3i, int, int> a, const std::tuple<cv::Vec3i, int, int> b)
                                                   { return std::get<0>(a).val[2] < std::get<0>(b).val[2]; }))
                              .val[2];
        const int min_r = std::get<0>(*min_element(sub_start, sub_end, [](const std::tuple<cv::Vec3i, int, int> a, const std::tuple<cv::Vec3i, int, int> b)
                                                   { return std::get<0>(a).val[2] < std::get<0>(b).val[2]; }))
                              .val[2];
        const int max_g = std::get<0>(*max_element(sub_start, sub_end, [](const std::tuple<cv::Vec3i, int, int> a, const std::tuple<cv::Vec3i, int, int> b)
                                                   { return std::get<0>(a).val[1] < std::get<0>(b).val[1]; }))
                              .val[1];
        const int min_g = std::get<0>(*min_element(sub_start, sub_end, [](const std::tuple<cv::Vec3i, int, int> a, const std::tuple<cv::Vec3i, int, int> b)
                                                   { return std::get<0>(a).val[1] < std::get<0>(b).val[1]; }))
                              .val[1];
        const int max_b = std::get<0>(*max_element(sub_start, sub_end, [](const std::tuple<cv::Vec3i, int, int> a, const std::tuple<cv::Vec3i, int, int> b)
                                                   { return std::get<0>(a).val[0] < std::get<0>(b).val[0]; }))
                              .val[0];
        const int min_b = std::get<0>(*min_element(sub_start, sub_end, [](const std::tuple<cv::Vec3i, int, int> a, const std::tuple<cv::Vec3i, int, int> b)
                                                   { return std::get<0>(a).val[0] < std::get<0>(b).val[0]; }))
                              .val[0];

        const int range_r = max_r - min_r;
        const int range_g = max_g - min_g;
        const int range_b = max_b - min_b;
        if (range_r >= range_g && range_r >= range_b)
        {
            //sort by red
            sort(sub_start, sub_end, [](const std::tuple<cv::Vec3i, int, int> a, const std::tuple<cv::Vec3i, int, int> b)
                 { return std::get<0>(a).val[2] < std::get<0>(b).val[2]; });
        }
        else if (range_g >= range_r && range_g >= range_b)
        {
            //sort by green
            sort(sub_start, sub_end, [](const std::tuple<cv::Vec3i, int, int> a, const std::tuple<cv::Vec3i, int, int> b)
                 { return std::get<0>(a).val[1] < std::get<0>(b).val[1]; });
        }
        else
        {
            //sort by blue
            sort(sub_start, sub_end, [](const std::tuple<cv::Vec3i, int, int> a, const std::tuple<cv::Vec3i, int, int> b)
                 { return std::get<0>(a).val[0] < std::get<0>(b).val[0]; });
        }

        const int median = ((end - start) / 2) + start;
        num_colors = num_colors / 2;

        //recursive calls
        rec_median_cut(image, palette, num_colors, start, median);
        rec_median_cut(image, palette, num_colors, median, end);
    }
}

std::vector<cv::Vec3b> pixels::median_cut(cv::Mat &image, int num_colors)
{
    //0, 1, 2 num_colors cases
    if (num_colors < 3)
    {
        return special_cases(image, num_colors);
    }

    if ((num_colors & (num_colors - 1)) == 0)
    {
        //num_colors is a power of two, no modifications needed
    }
    else
    {
        //choose next smaller power of 2
        int temp = int(log2(num_colors));
        num_colors = int(pow(2, temp));
        std::cout << "Warning: Median_cut transformation requires a power of 2 input; choosing next smallest power of 2 as: " << num_colors << "\n";
    }

    //convert matrix to vector for sorting, store original index from matrix for conversion back
    // <vec, i_index, j_index>
    std::vector<std::tuple<cv::Vec3i, int, int> > im_array;
    for (int i = 0; i < image.rows; i++)
    {
        for (int j = 0; j < image.cols; j++)
        {
            im_array.push_back(std::tuple<cv::Vec3i, int, int>(image.at<cv::Vec3b>(i, j), i, j));
        }
    }

    //recursive median cut to convert colors
    std::vector<cv::Vec3b> palette;
    rec_median_cut(im_array, palette, num_colors, 0, image.cols * image.rows);

    //convert image matrix depending on array values and stored index values
    for (int i = 0; i < im_array.size(); i++)
    {
        image.at<cv::Vec3b>(std::get<1>(im_array[i]), std::get<2>(im_array[i])) = std::get<0>(im_array[i]);
    }

    return palette;
}

std::vector<cv::Vec3b> pixels::kmeans(cv::Mat &image, const int num_colors)
{
    //0, 1, 2 num_colors cases
    if (num_colors < 3)
    {
        return special_cases(image, num_colors);
    }

    //convert for kmeans method
    cv::Mat samples;
    image.convertTo(samples, CV_32F);
    samples = samples.reshape(1, samples.total());

    cv::Mat labels;
    cv::Mat centers;
    //call kmeans appropriately
    cv::kmeans(samples, num_colors, labels, cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 10, 1.0), 10, cv::KMEANS_RANDOM_CENTERS, centers);

    //convert back and replace colors
    centers = centers.reshape(3, centers.rows);
    samples = samples.reshape(3, samples.rows);

    cv::Vec3f *point = samples.ptr<cv::Vec3f>();
    for (int i = 0; i < samples.rows; i++)
    {
        int center_id = labels.at<int>(i);
        point[i] = centers.at<cv::Vec3f>(center_id);
    }

    image = samples.reshape(3, image.rows);
    centers = centers.reshape(3, centers.rows);
    centers.convertTo(centers, CV_8U);
    image.convertTo(image, CV_8U);

    //create palette
    std::vector<cv::Vec3b> palette;
    for (int j = 0; j < centers.rows; j++)
    {
        palette.push_back(centers.at<cv::Vec3b>(0, j));
    }

    return palette;
}

//future TODO: Octree

//costly but some dithering algs need it
//future TODO: better way to do this?
void pixels::external_palette_replacement(cv::Mat &image, std::vector<cv::Vec3b> &palette)
{
    for (int i = 0; i < image.rows; i++)
    {
        for (int j = 0; j < image.cols; j++)
        {
            const cv::Vec3i old_vec = image.at<cv::Vec3b>(i, j);
            double curr_min_dist = double(0);
            //iterate through chosen colors to find closest one (euclidean distance) and replace as pixel color
            for (int k = 0; k < palette.size(); k++)
            {
                const cv::Vec3i curr_vec = palette[k];
                const double dist = sqrt(double(pow((old_vec.val[2] - curr_vec.val[2]), 2)) + double(pow((old_vec.val[1] - curr_vec.val[1]), 2)) + double(pow((old_vec.val[0] - curr_vec.val[0]), 2))); //calc euc dist
                if (dist < curr_min_dist || k == 0)
                {
                    curr_min_dist = dist;
                    image.at<cv::Vec3b>(i, j) = curr_vec;
                }
            }
        }
    }
}
