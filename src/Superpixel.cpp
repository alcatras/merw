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

merw::SuperpixelResult merw::Superpixel::process(cv::Mat mat) {
    cv::Ptr<cv::ximgproc::SuperpixelSLIC> slic = cv::ximgproc::createSuperpixelSLIC(mat, type, size, ruler);
    slic->iterate(iterations);

    slic->enforceLabelConnectivity();

    cv::Mat labels(mat.size(), CV_32SC1);
    slic->getLabels(labels);

    std::cout << "slic finished\nsuperpixels: " << slic->getNumberOfSuperpixels() << std::endl;

    cv::Mat contour;
    slic->getLabelContourMask(contour);

    cv::imwrite("dump/superpixel_contour.png", contour);
    cv::imwrite("dump/superpixel_labels.png", labels);

    return {labels, slic->getNumberOfSuperpixels()};
}


