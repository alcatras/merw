//
// Created by kamil on 06.01.17.
//

#ifndef SMART_WALLPAPERS_PHOTO_LOADER_H
#define SMART_WALLPAPERS_PHOTO_LOADER_H

#include <opencv2/opencv.hpp>
#include <opencv2/ximgproc.hpp>

#include "utils.h"

struct superpixel_result {
    int quantity;
    cv::Mat map;
};

superpixel_result superpixel(const std::string& method, cv::Mat& img, const bool dump) {
    std::vector<std::string> args = parse_args(method);
    assert(args.size() > 0);

    if(args[0] == "slic") {
        assert(args.size() == 4);
        cv::Ptr<cv::ximgproc::SuperpixelSLIC> slic = cv::ximgproc::createSuperpixelSLIC(img, cv::ximgproc::SLIC,
                                                                                        atoi(args[1].c_str()),
                                                                                        atof(args[2].c_str()));
        slic->iterate(atoi(args[3].c_str()));

        slic->enforceLabelConnectivity();

        cv::Mat labels(img.size(), CV_32SC1);
        slic->getLabels(labels);

        std::cout << "slic finished\nsuperpixels: " << slic->getNumberOfSuperpixels() << std::endl;

        if(dump) {
            cv::Mat contour;
            slic->getLabelContourMask(contour);
            cv::imwrite("dump/superpixel_contour.png", contour);
            cv::imwrite("dump/superpixel_labels.png", labels);
        }

        return {slic->getNumberOfSuperpixels(), labels};
    }
    return {};
}

cv::Mat convert_to_lab(const std::string& type, cv::Mat img) {
    if(type == "bgr2") {
        cv::Mat imgLab;
        cv::cvtColor(img, imgLab, CV_BGR2Lab);

        return imgLab;
    }
    return cv::Mat();
}


#endif //SMART_WALLPAPERS_PHOTO_LOADER_H
