//
// Created by kamil on 06.01.17.
//

#ifndef SMART_WALLPAPERS_PHOTO_LOADER_H
#define SMART_WALLPAPERS_PHOTO_LOADER_H

#include <opencv2/opencv.hpp>
#include <opencv2/ximgproc.hpp>

class image_helper {
    cv::Mat img;

public:
    bool load(const std::string& path)
    {
        img = cv::imread(path);
        return !img.empty();
    }

    cv::Mat superpixel(const std::string& method)
    {
        if(method == "slic")
        {
            cv::Ptr<cv::ximgproc::SuperpixelSLIC> slic = cv::ximgproc::createSuperpixelSLIC(img, cv::ximgproc::SLIC, 15, 10.f);
            slic->iterate(10);

            slic->enforceLabelConnectivity();

            cv::Mat labels(img.size(), CV_32SC1);
            slic->getLabels(labels);

            return labels;
        }

        return NULL;
    }

    void convert_to_lab(const std::string& type)
    {
        if(type == "bgr2") {
            cv::Mat imgLab;
            cv::cvtColor(img, imgLab, CV_BGR2Lab);

            img = imgLab;
        }
    }
};


#endif //SMART_WALLPAPERS_PHOTO_LOADER_H
