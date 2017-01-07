//
// Created by kamil on 07.01.17.
//

#ifndef SMART_WALLPAPERS_MERW_H
#define SMART_WALLPAPERS_MERW_H

#include <opencv2/opencv.hpp>
#include <stack>
#include <limits>
#include <cmath>

struct region_data {
    std::vector<std::pair<unsigned, unsigned>> pixels;

    double av_l;
    double av_a;
    double av_b;

    std::pair<unsigned, unsigned> center;
};

double color_distance(const region_data& lhs, const region_data& rhs, double color_weight, double distance_weight) {

    double dist_l = fabs(lhs.av_l - rhs.av_l);
    double dist_a = fabs(lhs.av_a - rhs.av_a);
    double dist_b = fabs(lhs.av_b - rhs.av_b);

    double dist_color = sqrt(dist_l * dist_l + dist_a * dist_a + dist_b * dist_b);

    unsigned dist_x = lhs.center.first - rhs.center.first;
    unsigned dist_y = lhs.center.second - rhs.center.second;

    double dist_center = sqrt(dist_x * dist_x + dist_y * dist_y);

    return 1. / (dist_color * color_weight + dist_center * distance_weight);
}

std::vector<std::pair<unsigned, unsigned>>
get_neighbours(std::pair<unsigned, unsigned>& pixel, bool closed_list[], int width, int height) {
    std::vector<std::pair<unsigned, unsigned>> result;

    if(pixel.first > 0 && !closed_list[(pixel.first - 1) * width + pixel.second])
        result.push_back({pixel.first - 1, pixel.second});

    if(pixel.first < width - 1 && !closed_list[(pixel.first + 1) * width + pixel.second])
        result.push_back({pixel.first + 1, pixel.second});

    if(pixel.second > 0 && !closed_list[pixel.first * width + pixel.second - 1])
        result.push_back({pixel.first, pixel.second - 1});

    if(pixel.second < height - 1 && !closed_list[pixel.first * width + pixel.second + 1])
        result.push_back({pixel.first, pixel.second + 1});

    return result;
};

void merw(cv::Mat& image, superpixel_result& superpixels) {
    int width = superpixels.map.cols;
    int height = superpixels.map.rows;

    std::vector<region_data> regions;
    for(int i = 0; i < superpixels.number_of_regions; ++i)
        regions.push_back(region_data());

    bool* visited_regions = new bool[superpixels.number_of_regions];
    for(int k = 0; k < superpixels.number_of_regions; ++k) {
        visited_regions[k] = false;
    }

    std::stack<std::pair<unsigned, unsigned>> open_list;
    open_list.push({0, 0});

    bool* closed_list = new bool[width * height];
    for(int i = 0; i < width * height; ++i) {
        *(closed_list + i) = false;
    }

    double adjacency_matrix[superpixels.number_of_regions][superpixels.number_of_regions];
    for(int i = 0; i < superpixels.number_of_regions; ++i) {
        for(int j = 0; j < superpixels.number_of_regions; ++j) {
            adjacency_matrix[i][j] = 0;
        }
    }

    while(!open_list.empty()) {
        auto pixel = (std::pair<unsigned, unsigned>&&) open_list.top();
        open_list.pop();

        int current_region = superpixels.map.at<int>(pixel.second, pixel.first);

        std::stack<std::pair<unsigned, unsigned>> region_open_list;
        region_open_list.push(pixel);
        *(closed_list + pixel.first * width + pixel.second) = true;

        while(!region_open_list.empty()) {
            auto current_pixel = (std::pair<unsigned, unsigned>&&) region_open_list.top();
            region_open_list.pop();

            auto neighbours = get_neighbours(current_pixel, closed_list, width, height);

            for(auto it = neighbours.begin(); it != neighbours.end(); ++it) {
                int region = superpixels.map.at<int>(it->second, it->first);

                if(region == current_region) {
                    region_open_list.push(*it);
                    closed_list[it->first * width + it->second] = true;
                } else {
                    if(!(adjacency_matrix[current_region][region] || adjacency_matrix[region][current_region])) {
                        adjacency_matrix[current_region][region] = adjacency_matrix[region][current_region] = 1;
                        if(!visited_regions[region]) {
                            open_list.push(*it);
                            visited_regions[region] = true;
                        }
                    }
                }
            }
            regions[current_region].pixels.push_back(current_pixel);
        }
    }

    delete[] closed_list;
    delete[] visited_regions;

    cv::Mat labSpaceImage;
    cv::cvtColor(image, labSpaceImage, CV_BGR2Lab);

    for(auto it = regions.begin(); it != regions.end(); ++it) {

        it->av_l = 0;
        it->av_a = 0;
        it->av_b = 0;

        unsigned min_x = UINT_MAX;
        unsigned max_x = 0;
        unsigned min_y = UINT_MAX;
        unsigned max_y = 0;

        for(auto pi = it->pixels.begin(); pi != it->pixels.end(); ++pi) {
            cv::Vec3d pix_bgr = labSpaceImage.ptr<cv::Vec3d>(pi->second)[pi->first];
            it->av_l += std::isfinite(pix_bgr.val[0]) ? 0 : pix_bgr.val[0] + 10;
            it->av_a += std::isfinite(pix_bgr.val[1]) ? 0 : pix_bgr.val[1] + 10;
            it->av_b += std::isfinite(pix_bgr.val[2]) ? 0 : pix_bgr.val[2] + 10;

            if(pi->first < min_x)
                min_x = pi->first;
            if(pi->first > max_x)
                max_x = pi->first;
            if(pi->second < min_y)
                min_y = pi->second;
            if(pi->second > max_y)
                max_y = pi->second;
        }

        it->av_l /= it->pixels.size();
        it->av_a /= it->pixels.size();
        it->av_b /= it->pixels.size();

        it->center = {min_x + (max_x - min_x) / 2, min_y + (max_y - min_y) / 2};
    }

    for(int i = 0; i < superpixels.number_of_regions; ++i) {
        for(int j = i + 1; j < superpixels.number_of_regions; ++j) {
            if(adjacency_matrix[i][j] == 1) {
                adjacency_matrix[i][j] = color_distance(regions[i], regions[j], 0.9, 0.1);
            }
            std::cout << adjacency_matrix[i][j] << " ";
        }
        std::cout << std::endl;
    }

    int nop = 0;
    for(auto it = regions.begin(); it != regions.end(); ++it) {
        std::cout << it->pixels.size() << ":" << it->center.first << ":" << it->center.second << " ";
        nop += it->pixels.size();
    }
    std::cout << std::endl << nop << std::endl;
}


#endif //SMART_WALLPAPERS_MERW_H
