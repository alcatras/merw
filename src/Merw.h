//
// Created by kamil on 08.01.17.
//

#ifndef SMART_WALLPAPERS_MERW_H
#define SMART_WALLPAPERS_MERW_H

#include <vector>
#include <stack>
#include <opencv2/opencv.hpp>

#include "Superpixel.h"

namespace merw {
    typedef unsigned int uint;
    typedef std::pair<uint, uint> Point;
    typedef std::vector<Point> Points;

    struct Region {
        Points pixels;

        double av_x;
        double av_y;
        double av_z;

        Point center;
    };

    class Merw {

        cv::Mat averagedImage;

        double colorDistance(const Region& lhs, const Region& rhs, double colorValue, double distanceValue);

        Points getNeighbours(Point& pixel, bool closedList[], uint width, uint height);

        void averageRegion(Region& region, cv::Mat& image);

        cv::Mat createAveragedImage(cv::Mat& regionMap, std::vector<merw::Region>& regions);

    public:
        ~Merw();

        void generateGraph(const cv::Mat& image, const Superpixel& superpixel);

        const cv::Mat& getAveragedImage() const;
    };
}


#endif //SMART_WALLPAPERS_MERW_H
