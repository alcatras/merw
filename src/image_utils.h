//
// Created by kamil on 06.01.17.
//

#ifndef SMART_WALLPAPERS_PHOTO_LOADER_H
#define SMART_WALLPAPERS_PHOTO_LOADER_H

#include <opencv2/opencv.hpp>
#include <opencv2/ximgproc.hpp>

#include "utils.h"

cv::Mat convert_to_lab(cv::Mat& img) {
    cv::Mat imgLab;
    cv::cvtColor(img, imgLab, CV_RGB2Lab);
    return imgLab;
}

cv::Mat convert_to_rgb(cv::Mat& img) {
    cv::Mat imgRgb;
    cv::cvtColor(img, imgRgb, CV_Lab2RGB);
    return imgRgb;
}


#endif //SMART_WALLPAPERS_PHOTO_LOADER_H
