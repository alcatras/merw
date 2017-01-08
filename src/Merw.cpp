//
// Created by kamil on 08.01.17.
//

#include <stack>
#include "Merw.h"

double
merw::Merw::colorDistance(const merw::Region& lhs, const merw::Region& rhs, double colorValue, double distanceValue) {
    double diff_x = fabs(lhs.av_x - rhs.av_x);
    double diff_y = fabs(lhs.av_y - rhs.av_y);
    double diff_z = fabs(lhs.av_z - rhs.av_z);

    double color_diff = sqrt(diff_x * diff_x + diff_y * diff_y + diff_z * diff_z);

    uint dist_x = lhs.center.first - rhs.center.first;
    uint dist_y = lhs.center.second - rhs.center.second;

    double dist_center = sqrt(dist_x * dist_x + dist_y * dist_y);

    return 1. / (color_diff * colorValue + dist_center * distanceValue);
}

merw::Points merw::Merw::getNeighbours(merw::Point& pixel, bool* closedList, uint width, uint height) {
    Points result;

    for(int x = std::max<int>(pixel.first - 1, 0); x <= std::min<int>(pixel.first + 1, width - 1); ++x) {
        for(int y = std::max<int>(pixel.second - 1, 0); y <= std::min<int>(pixel.second + 1, height - 1); ++y) {
            if(!closedList[y * width + x]) {
                result.push_back({x, y});
            }
        }
    }
    return result;
}

void merw::Merw::averageRegion(merw::Region& region, cv::Mat& image) {
    region.av_x = 0;
    region.av_y = 0;
    region.av_z = 0;

    uint min_x = UINT_MAX;
    uint max_x = 0;
    uint min_y = UINT_MAX;
    uint max_y = 0;

    for(auto pi = region.pixels.begin(); pi != region.pixels.end(); ++pi) {
        cv::Vec3b pix_bgr = image.at<cv::Vec3b>(pi->second, pi->first);
        region.av_x += pix_bgr.val[0];
        region.av_y += pix_bgr.val[1];
        region.av_z += pix_bgr.val[2];

        if(pi->first < min_x)
            min_x = pi->first;
        if(pi->first > max_x)
            max_x = pi->first;
        if(pi->second < min_y)
            min_y = pi->second;
        if(pi->second > max_y)
            max_y = pi->second;
    }

    region.av_x /= region.pixels.size() * 255;
    region.av_y /= region.pixels.size() * 255;
    region.av_z /= region.pixels.size() * 255;

    region.center = {min_x + (max_x - min_x) / 2, min_y + (max_y - min_y) / 2};
}

merw::Merw::Merw(merw::Superpixel s, double acv) : superpixel(s) {
    averageColorValue = acv;
}

merw::Merw::~Merw() {

}

void merw::Merw::process(cv::Mat image) {
    // superpixel
    SuperpixelResult super_res = superpixel.process(image);

    uint width = (uint) image.cols;
    uint height = (uint) image.rows;

    // initialize regions
    std::vector<Region> regions;
    for(int i = 0; i < super_res.regions; ++i)
        regions.push_back(Region());

    // initialize auxilary visited regions list
    bool* visited_regions = new bool[super_res.regions];
    for(int k = 0; k < super_res.regions; ++k)
        visited_regions[k] = false;

    // initialize open list
    std::stack<merw::Point> open_list;
    open_list.push({0, 0});

    // initialize closed list
    bool* closed_list = new bool[width * height];
    for(int i = 0; i < width * height; ++i)
        closed_list[i] = false;

    // initialize adjacency matrix
    double adjacency_matrix[super_res.regions][super_res.regions];
    for(int i = 0; i < super_res.regions; ++i)
        for(int j = 0; j < super_res.regions; ++j)
            adjacency_matrix[i][j] = 0;

    while(!open_list.empty()) {
        auto pixel = (std::pair<uint, uint>&&) open_list.top();
        open_list.pop();

        int current_region = super_res.map.at<int>(pixel.second, pixel.first);
        visited_regions[current_region] = true;

        std::stack<std::pair<uint, uint>> region_open_list;
        region_open_list.push(pixel);

        closed_list[pixel.second * width + pixel.first] = true;

        while(!region_open_list.empty()) {
            auto current_pixel = (std::pair<uint, uint>&&) region_open_list.top();
            region_open_list.pop();

            auto neighbours = getNeighbours(current_pixel, closed_list, width, height);

            for(auto it = neighbours.begin(); it != neighbours.end(); ++it) {
                int region = super_res.map.at<int>(it->second, it->first);

                if(region == current_region) {
                    region_open_list.push(*it);
                    closed_list[it->second * width + it->first] = true;
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

    for(auto&& region : regions) {
        averageRegion(region, image);
    }

    cv::imwrite("dump/avr.png", createAveragedImage(super_res.map, regions));

    double averageDistanceValue = 1 - averageColorValue;

    for(int i = 0; i < super_res.regions; ++i) {
        for(int j = i + 1; j < super_res.regions; ++j) {
            if(adjacency_matrix[i][j] == 1) {
                adjacency_matrix[i][j] = adjacency_matrix[j][i] = colorDistance(regions[i], regions[j],
                                                                                averageColorValue,
                                                                                averageDistanceValue);
                std::cout << adjacency_matrix[i][j] << std::endl;
            }
        }
    }
}

cv::Mat merw::Merw::createAveragedImage(cv::Mat& regionMap, std::vector<merw::Region>& regions) {
    uint width = (uint) regionMap.cols;
    uint height = (uint) regionMap.rows;

    cv::Mat averagedImage(height, width, CV_8UC3);

    for(int i = 0; i < height; ++i) {
        for(int j = 0; j < width; ++j) {
            cv::Vec3b& pix_bgr = averagedImage.ptr<cv::Vec3b>(i)[j];
            int region = regionMap.at<int>(i, j);

            pix_bgr.val[0] = (unsigned char) regions[region].av_x;
            pix_bgr.val[1] = (unsigned char) regions[region].av_y;
            pix_bgr.val[2] = (unsigned char) regions[region].av_z;
        }
    }
    return averagedImage;
}
