//
// Created by kamil on 08.01.17.
//

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

    region.av_x /= region.pixels.size();
    region.av_y /= region.pixels.size();
    region.av_z /= region.pixels.size();

    region.center = {min_x + (max_x - min_x) / 2, min_y + (max_y - min_y) / 2};
}

merw::Merw::~Merw() {

}

void merw::Merw::generateGraph(const cv::Mat& image, const Superpixel& superpixel) {

    uint width = (uint) image.cols;
    uint height = (uint) image.rows;

    // initialize regions
    regions = std::vector<Region>();
    for(int i = 0; i < superpixel.getRegions(); ++i)
        regions.push_back(Region());

    // initialize auxilary visited regions list
    bool* visited_regions = new bool[superpixel.getRegions()];
    for(int k = 0; k < superpixel.getRegions(); ++k)
        visited_regions[k] = false;

    // initialize open list
    std::stack<merw::Point> open_list;
    open_list.push({0, 0});

    // initialize closed list
    bool* closed_list = new bool[width * height];
    for(int i = 0; i < width * height; ++i)
        closed_list[i] = false;

    // create adjacency matrix
    adjacency_matrix = cv::Mat(superpixel.getRegions(), superpixel.getRegions(), CV_64F, cv::Scalar(0));

    while(!open_list.empty()) {
        auto pixel = (std::pair<uint, uint>&&) open_list.top();
        open_list.pop();

        int current_region = superpixel.getMap().at<int>(pixel.second, pixel.first);
        visited_regions[current_region] = true;

        std::stack<std::pair<uint, uint>> region_open_list;
        region_open_list.push(pixel);

        closed_list[pixel.second * width + pixel.first] = true;

        while(!region_open_list.empty()) {
            auto current_pixel = (std::pair<uint, uint>&&) region_open_list.top();
            region_open_list.pop();

            auto neighbours = getNeighbours(current_pixel, closed_list, width, height);

            for(auto it = neighbours.begin(); it != neighbours.end(); ++it) {
                int region = superpixel.getMap().at<int>(it->second, it->first);

                if(region == current_region) {
                    region_open_list.push(*it);
                    closed_list[it->second * width + it->first] = true;
                } else {
                    if(!(adjacency_matrix.at<double>(current_region, region) != 0 || adjacency_matrix.at<double>(region, current_region)) != 0) {
                        adjacency_matrix.at<double>(current_region, region) = adjacency_matrix.at<double>(region, current_region) = 1.0;

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

    cv::Mat tempImage(image.size(), CV_8UC3);

    cv::cvtColor(image, tempImage, CV_BGR2Lab);

    for(auto& region : regions) {
        averageRegion(region, tempImage);
    }

    regionMap = superpixel.getMap();

    tempImage = createAveragedImage();
    cv::cvtColor(tempImage, averagedImage, CV_Lab2BGR);
}

void merw::Merw::calculateStationaryDistribution(const double colorValue) {
    const double distanceValue = 1 - colorValue;

    const double MATH_E = std::exp(1);

    for(int i = 0; i < adjacency_matrix.rows; ++i) {
        for(int j = i + 1; j < adjacency_matrix.rows; ++j) {
            if(adjacency_matrix.at<double>(i, j) != 0) {
                double value = std::pow(MATH_E, -colorDistance(regions[i], regions[j], colorValue, distanceValue));
                adjacency_matrix.at<double>(i, j) = value;
                adjacency_matrix.at<double>(j, i) = value;
            }
        }
    }

    cv::Mat eigen_values, eigen_vectors;
    cv::eigen(adjacency_matrix, eigen_values, eigen_vectors);

    stationaryImage = cv::Mat(regionMap.rows, regionMap.cols, CV_8UC3);

    double eigen_min = std::numeric_limits<double>::max();
    double eigen_max = -std::numeric_limits<double>::max();

    for(int i = 0; i < adjacency_matrix.rows; ++i) {
        double eigen_squared = std::pow(eigen_vectors.at<double>(0, i), 2.);
        eigen_vectors.at<double>(0, i) = eigen_squared;

        if(eigen_squared < eigen_min)
            eigen_min = eigen_squared;
        else if(eigen_squared > eigen_max)
            eigen_max = eigen_squared;
    }

    for(int i = 0; i < adjacency_matrix.rows; ++i) {
        eigen_vectors.at<double>(0, i) = (eigen_vectors.at<double>(0, i) - eigen_min) / (eigen_max - eigen_min);
    }

    for(int i = 0; i < regionMap.rows; ++i) {
        for (int j = 0; j < regionMap.cols; ++j) {
            cv::Vec3b &pix_bgr = stationaryImage.ptr<cv::Vec3b>(i)[j];
            int region = regionMap.at<int>(i, j);

            pix_bgr.val[0] = (unsigned char) (eigen_vectors.at<double>(0, region) * 255);
            pix_bgr.val[1] = (unsigned char) (eigen_vectors.at<double>(0, region) * 255);
            pix_bgr.val[2] = (unsigned char) (eigen_vectors.at<double>(0, region) * 255);
        }
    }
    cv::imwrite("ddudud.png", stationaryImage);

//    }
//
//    for(unsigned i = 0; i < regions.size(); ++i) {
//        Region &region = regions[i];
//
//        double region_distribution = (eigen_vectors.at<double>(0, i) - eigen_min) / (eigen_max - eigen_min);
//
//        for(auto pi = region.pixels.begin(); pi != region.pixels.end(); ++pi) {
//            cv::Vec3b& pixel = stationaryImage.at<cv::Vec3b>(pi->second, pi->first);
//
//            pixel.val[0] = (unsigned char) (region_distribution * 255);
//            pixel.val[1] = (unsigned char) (region_distribution * 255);
//            pixel.val[2] = (unsigned char) (region_distribution * 255);
//        }
//    }
}

cv::Mat merw::Merw::createAveragedImage() {
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

const cv::Mat& merw::Merw::getAveragedImage() const {
    return averagedImage;
}

const cv::Mat &merw::Merw::getStationaryDistributionImage() const {
    return stationaryImage;
}
