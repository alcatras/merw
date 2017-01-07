//
// Created by kamil on 07.01.17.
//

#ifndef SMART_WALLPAPERS_MERW_H
#define SMART_WALLPAPERS_MERW_H

#include <opencv2/opencv.hpp>
#include <stack>


std::vector<std::pair<int, int>>
get_neighbours(std::pair<int, int>& pixel, bool closed_list[], int width, int height) {
    std::vector<std::pair<int, int>> result;

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

void merw(superpixel_result& superpixels) {
    int width = superpixels.map.cols;
    int height = superpixels.map.rows;

    std::vector<std::pair<int, int>> regions[superpixels.quantity];
    bool* visited_regions = new bool[superpixels.quantity];
    for(int k = 0; k < superpixels.quantity; ++k) {
        visited_regions[k] = false;
    }

    std::stack<std::pair<int, int>> open_list;
    open_list.push({0, 0});

    bool* closed_list = new bool[width * height];
    for(int i = 0; i < width * height; ++i) {
        *(closed_list + i) = false;
    }

    float adjacency_matrix[superpixels.quantity][superpixels.quantity];
    for(int i = 0; i < superpixels.quantity; ++i) {
        for(int j = 0; j < superpixels.quantity; ++j) {
            adjacency_matrix[i][j] = 0;
        }
    }

    while(!open_list.empty()) {
        auto pixel = (std::pair<int, int>&&) open_list.top();
        open_list.pop();

        int current_region = superpixels.map.at<int>(pixel.second, pixel.first);

        std::stack<std::pair<int, int>> region_open_list;
        region_open_list.push(pixel);
        *(closed_list + pixel.first * width + pixel.second) = true;

        while(!region_open_list.empty()) {
            auto current_pixel = (std::pair<int, int>&&) region_open_list.top();
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
            regions[current_region].push_back(current_pixel);
        }
    }

    delete[] closed_list;
    delete[] visited_regions;


}


#endif //SMART_WALLPAPERS_MERW_H
