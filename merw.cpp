#include "merw.h"

Merw::Merw(cv::Mat& _img, cv::Mat& _regionMap, int _numberOfRegions) :
    img(_img), regionMap(_regionMap), numberOfRegions(_numberOfRegions)
{
    regions.resize(_numberOfRegions);
}

void Merw::createAveragedImage(cv::Mat* mat)
{
    cv::Mat temp(img.size(), CV_8UC3);
    cv::cvtColor(img, temp, CV_BGR2Lab);

    for(auto& region : regions) {
        averageRegion(region);
    }

    uint width = (uint) regionMap.cols;
    uint height = (uint) regionMap.rows;

    temp = cv::Mat(height, width, CV_8UC3);

    for(uint i = 0; i < height; ++i) {
        for(uint j = 0; j < width; ++j) {
            cv::Vec3b& pix_bgr = temp.ptr<cv::Vec3b>(i)[j];
            int region = regionMap.at<int>(i, j);

            pix_bgr.val[0] = (unsigned char) regions[region].av_x;
            pix_bgr.val[1] = (unsigned char) regions[region].av_y;
            pix_bgr.val[2] = (unsigned char) regions[region].av_z;
        }
    }
    temp.copyTo(*mat);
}

double Merw::colorDistance(const Region& lhs, const Region& rhs, double colorValue)
{
    double distanceValue = 1 - colorValue;

    double diff_x = lhs.av_x - rhs.av_x;
    double diff_y = lhs.av_y - rhs.av_y;
    double diff_z = lhs.av_z - rhs.av_z;

    double color_diff = sqrt(diff_x * diff_x + diff_y * diff_y + diff_z * diff_z);

    uint dist_x = lhs.center.first - rhs.center.first;
    uint dist_y = lhs.center.second - rhs.center.second;

    double dist_center = sqrt(dist_x * dist_x + dist_y * dist_y);

    return color_diff * colorValue + dist_center * distanceValue;
}

Points Merw::getNeighbours(Point& pixel, bool closedList[], uint width, uint height)
{
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

void Merw::averageRegion(Region& region)
{
    region.av_x = 0;
    region.av_y = 0;
    region.av_z = 0;

    uint min_x = UINT_MAX;
    uint max_x = 0;
    uint min_y = UINT_MAX;
    uint max_y = 0;

    for(auto pi = region.pixels.begin(); pi != region.pixels.end(); ++pi) {
        cv::Vec3b pix_bgr = img.at<cv::Vec3b>(pi->second, pi->first);
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

void Merw::calculateGraph(bool localizeGraph)
{
    uint width = (uint) img.cols;
    uint height = (uint) img.rows;

    // initialize auxilary visited regions list
    bool* visited_regions = new bool[numberOfRegions];
    for(int k = 0; k < numberOfRegions; ++k)
        visited_regions[k] = false;

    // initialize open list
    std::stack<Point> open_list;
    open_list.push({0, 0});

    // initialize closed list
    bool* closed_list = new bool[width * height];
    for(uint i = 0; i < width * height; ++i)
        closed_list[i] = false;

    adjacencyMatrix = localizeGraph ? cv::Mat::zeros(numberOfRegions, numberOfRegions, CV_64F) : cv::Mat::ones(numberOfRegions, numberOfRegions, CV_64F);

    while(!open_list.empty()) {
        auto pixel = (std::pair<uint, uint>&&) open_list.top();
        open_list.pop();

        int current_region = regionMap.at<int>(pixel.second, pixel.first);
        visited_regions[current_region] = true;

        std::stack<std::pair<uint, uint>> region_open_list;
        region_open_list.push(pixel);

        closed_list[pixel.second * width + pixel.first] = true;

        while(!region_open_list.empty()) {
            auto current_pixel = (std::pair<uint, uint>&&) region_open_list.top();
            region_open_list.pop();

            auto neighbours = getNeighbours(current_pixel, closed_list, width, height);

            for(auto it = neighbours.begin(); it != neighbours.end(); ++it) {
                int region = regionMap.at<int>(it->second, it->first);

                if(region == current_region) {
                    region_open_list.push(*it);
                    closed_list[it->second * width + it->first] = true;
                } else {
                    if(localizeGraph)
                    {
                        if(!(adjacencyMatrix.at<double>(current_region, region) != 0 || adjacencyMatrix.at<double>(region, current_region)) != 0)
                        {
                            adjacencyMatrix.at<double>(current_region, region) = adjacencyMatrix.at<double>(region, current_region) = 1.0;
                            if(!visited_regions[region]) {
                                open_list.push(*it);
                                visited_regions[region] = true;
                            }
                        }
                    }
                    else
                    {
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
}

void Merw::calculateStationaryDistribution(cv::Mat* mat, double colorValue, double epsilon)
{
    const double MATH_E = std::exp(1);

    for(int i = 0; i < adjacencyMatrix.rows; ++i) {
        double sum = 0;

        for(int j = 0; j < adjacencyMatrix.rows; ++j) {
            if(adjacencyMatrix.at<double>(i, j) != 0) {
                double value = std::pow(MATH_E, -colorDistance(regions[i], regions[j], colorValue));
                sum += value;
                adjacencyMatrix.at<double>(i, j) = value;
            }
        }

        double adjustValue = 1. / sum;

        for(int j = 0; j < adjacencyMatrix.rows; ++j) {
            if(adjacencyMatrix.at<double>(i, j) != 0) {
                adjacencyMatrix.at<double>(i, j) *= adjustValue;
            }
        }
    }

    cv::Mat eigen_values, eigen_vectors;
    cv::eigen(adjacencyMatrix, eigen_values, eigen_vectors);

    *mat = cv::Mat(regionMap.rows, regionMap.cols, CV_8UC1);

    double eigen_min = std::numeric_limits<double>::max();
    double eigen_max = -std::numeric_limits<double>::max();

    for(int i = 0; i < adjacencyMatrix.rows; ++i) {
        double eigen_squared = std::pow(eigen_vectors.at<double>(0, i), 2.);
        eigen_vectors.at<double>(0, i) = eigen_squared;

        if(eigen_squared < eigen_min)
            eigen_min = eigen_squared;
        else if(eigen_squared > eigen_max)
            eigen_max = eigen_squared;
    }

    qDebug() << "eigen_min: " << eigen_min << "eigen max: " << eigen_max;

//    for(int i = 0; i < adjacencyMatrix.rows; ++i) {
//        eigen_vectors.at<double>(0, i) = (eigen_vectors.at<double>(0, i) - eigen_min) / (eigen_max - eigen_min);
//    }

    for(int i = 0; i < regionMap.rows; ++i) {
        for (int j = 0; j < regionMap.cols; ++j) {
            int region = regionMap.at<int>(i, j);
            double value = eigen_vectors.at<double>(0, region);

            mat->at<unsigned char>(i, j) = (value > epsilon) ? 255 : (value > 0 ? 100 : 0);
        }
    }
}

