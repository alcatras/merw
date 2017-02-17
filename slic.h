#ifndef SLIC_H
#define SLIC_H

#include<opencv2/opencv.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/ximgproc/slic.hpp>

class Slic
{
public:
    Slic(int _size, float _ruler, int _iterations, bool _enforceConnectivity) :
        size(_size), ruler(_ruler), iterations(_iterations), enforceConnectivity(_enforceConnectivity) {
    }

    void process(const cv::Mat& original, cv::Mat* map, cv::Mat* contour, int* regions);

private:
    int size;
    float ruler;
    int iterations;
    bool enforceConnectivity;
};

#endif // SLIC_H
