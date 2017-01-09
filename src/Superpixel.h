//
// Created by kamil on 08.01.17.
//

#ifndef SMART_WALLPAPERS_SUPERPIXEL_H
#define SMART_WALLPAPERS_SUPERPIXEL_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/ximgproc/slic.hpp>

namespace merw {
    class Superpixel {
        typename cv::ximgproc::SLIC type;
        int size;
        float ruler;
        int iterations;

        cv::Mat map;
        cv::Mat contour;
        int regions;

    public:
        Superpixel(typename cv::ximgproc::SLIC type, int size, float ruler, int iterations);

        void process(const cv::Mat& mat);

        const cv::Mat& getMap() const;

        const cv::Mat& getContour() const;

        int getRegions() const;
    };
}


#endif //SMART_WALLPAPERS_SUPERPIXEL_H
