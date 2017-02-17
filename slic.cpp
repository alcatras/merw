#include "slic.h"

void Slic::process(const cv::Mat& original, cv::Mat* map, cv::Mat* contour, int* regions)
{
    cv::Ptr<cv::ximgproc::SuperpixelSLIC> slic = cv::ximgproc::createSuperpixelSLIC(original, cv::ximgproc::SLIC::SLIC, size, ruler);

    slic->iterate(iterations);
    if(enforceConnectivity) {
        slic->enforceLabelConnectivity(size);
    }
    slic->getLabels(*map);
    slic->getLabelContourMask(*contour);

    *regions = slic->getNumberOfSuperpixels();
}
