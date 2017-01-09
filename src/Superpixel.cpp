//
// Created by kamil on 08.01.17.
//

#include "Superpixel.h"

merw::Superpixel::Superpixel(typename cv::ximgproc::SLIC t, int s, float r, int i) {
    type = t;
    size = s;
    ruler = r;
    iterations = i;
}

void merw::Superpixel::process(const cv::Mat& mat) {
    cv::Ptr<cv::ximgproc::SuperpixelSLIC> slic = cv::ximgproc::createSuperpixelSLIC(mat, type, size, ruler);
    slic->iterate(iterations);

    map = cv::Mat(mat.size(), CV_8UC1);
    contour = cv::Mat(mat.size(), CV_8UC3);

    slic->enforceLabelConnectivity();

    slic->getLabels(map);

    cv::Mat cnt;
    slic->getLabelContourMask(cnt);
    cv::cvtColor(cnt, contour, CV_GRAY2BGR);

    regions = slic->getNumberOfSuperpixels();
}

const cv::Mat& merw::Superpixel::getMap() const {
    return map;
}

const cv::Mat& merw::Superpixel::getContour() const {
    return contour;
}

int merw::Superpixel::getRegions() const {
    return regions;
}


