#ifndef MERW_H
#define MERW_H

#include <QDebug>

#include <opencv2/opencv.hpp>

#include <stack>

typedef unsigned int uint;
typedef std::pair<uint, uint> Point;
typedef std::vector<Point> Points;

struct Region {
    Points pixels;

    double av_x;
    double av_y;
    double av_z;

    Point center;
};

class Merw
{
public:
    Merw(cv::Mat& _img, cv::Mat& _regionMap, int numberOfRegions);

    void calculateGraph(bool localizeGraph);
    void createAveragedImage(cv::Mat* mat);
    void calculateStationaryDistribution(cv::Mat* mat, double colorValue, double epsilon);

private:
    cv::Mat img;
    cv::Mat regionMap;
    cv::Mat adjacencyMatrix;

    int numberOfRegions;
    std::vector<Region> regions;

    double colorDistance(const Region& lhs, const Region& rhs, double colorValue);
    Points getNeighbours(Point& pixel, bool closedList[], uint width, uint height);
    void averageRegion(Region& region);
};

#endif // MERW_H
