//
// Created by kamil on 08.01.17.
//

#ifndef SMART_WALLPAPERS_SUPERPIXEL_H
#define SMART_WALLPAPERS_SUPERPIXEL_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/ximgproc/slic.hpp>

namespace merw {
    struct SuperpixelResult {
        cv::Mat map;
        int regions;
    };

    class Superpixel {
        typename cv::ximgproc::SLIC type;
        int size;
        float ruler;
        int iterations;

    public:
        Superpixel(typename cv::ximgproc::SLIC type, int size, float ruler, int iterations);

        SuperpixelResult process(cv::Mat mat);
    };
}


#endif //SMART_WALLPAPERS_SUPERPIXEL_H
