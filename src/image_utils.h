//
// Created by kamil on 06.01.17.
//

#ifndef SMART_WALLPAPERS_PHOTO_LOADER_H
#define SMART_WALLPAPERS_PHOTO_LOADER_H

#include <opencv2/opencv.hpp>
#include <opencv2/ximgproc.hpp>

#include "utils.h"

struct superpixel_settings {
    typename cv::ximgproc::SLIC method;
    int region_size;
    float ruler;
    int iterations;
    bool enforce_connectivity;
    bool dump_data;
};

struct superpixel_result {
    int number_of_regions;
    cv::Mat map;
};

superpixel_result slic_superpixel(const superpixel_settings& settings, const cv::Mat& img) {
    cv::Ptr<cv::ximgproc::SuperpixelSLIC> slic = cv::ximgproc::createSuperpixelSLIC(img, settings.method,
                                                                                    settings.region_size,
                                                                                    settings.ruler);
    slic->iterate(settings.iterations);

    if(settings.enforce_connectivity)
        slic->enforceLabelConnectivity();

    cv::Mat labels(img.size(), CV_32SC1);
    slic->getLabels(labels);

    if(settings.dump_data) {
        std::cout << "slic finished\nsuperpixels: " << slic->getNumberOfSuperpixels() << std::endl;

        cv::Mat contour;
        slic->getLabelContourMask(contour);

        cv::imwrite("dump/superpixel_contour.png", contour);
        cv::imwrite("dump/superpixel_labels.png", labels);
    }

    return {slic->getNumberOfSuperpixels(), labels};
}

cv::Mat convert_to_lab(cv::Mat& img) {
    cv::Mat imgLab;
    cv::cvtColor(img, imgLab, CV_BGR2Lab);
    return imgLab;
}


#endif //SMART_WALLPAPERS_PHOTO_LOADER_H
