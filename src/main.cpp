#include <iostream>

#include "Merw.h"

namespace merw {
    cv::Mat compound;

    cv::Mat image;

    cv::Mat image1;
    cv::Mat image2;
    cv::Mat image3;
    cv::Mat image4;

    const int size_slider_max = 100;
    int size_value = 20;

    const int ruler_slider_max = 200;
    int ruler_value = 100;

    const int iterations_slider_max = 50;
    int iterations_value = 10;

    void update(int) {
        Superpixel superpixel(cv::ximgproc::SLIC, size_value, ruler_value / 10.f, iterations_value);
        superpixel.process(image);

        cv::imwrite("dump/contour.png", superpixel.getContour());

        superpixel.getContour().copyTo(image2);
        std::cout << "number of regions: " << superpixel.getRegions() << std::endl;

        Merw merw;
        merw.generateGraph(image, superpixel);

        merw.getAveragedImage().copyTo(image3);

        cv::imshow("merw", compound);
    }
}

int main(int argc, char** argv) {

    cv::namedWindow("merw");
    cvCreateTrackbar("superpixel size", "merw", &merw::size_value, merw::size_slider_max, merw::update);
    cvCreateTrackbar("ruler", "merw", &merw::ruler_value, merw::ruler_slider_max, merw::update);
    cvCreateTrackbar("iterations", "merw", &merw::iterations_value, merw::iterations_slider_max, merw::update);

    using namespace merw;

    image = cv::imread("example_photos/sunflower.jpg");

    cv::Size size = image.size();
    compound = cv::Mat(size.height, size.width * 4, CV_8UC3);

    image1 = cv::Mat(compound, cv::Rect(0, 0, size.width, size.height));
    image2 = cv::Mat(compound, cv::Rect(size.width, 0, size.width, size.height));
    image3 = cv::Mat(compound, cv::Rect(size.width * 2, 0, size.width, size.height));
    image4 = cv::Mat(compound, cv::Rect(size.width * 3, 0, size.width, size.height));

    image.copyTo(image1);

    update(0);

    cvWaitKey(0);

//    std::string photo = "";
//
//    std::string superpixel_method = "slic:4:10:20"; // slic, slico, lsc
//
//    std::string color_space = "bgr2"; // rgb, bgr2
//    std::string average_method = "average"; // average, mean, min, max
//    float color_weight = 0.9; // 0 .. 1 - dist
//    float distance_weight = 0.1; // 0 .. 1 - color
//
//    bool dump_intermediate = false;
//
//    static struct option long_options[] =
//            {
//                    {"photo",           required_argument, 0, 0},
//                    {"superpixel",      required_argument, 0, 1},
//                    {"color_space",     required_argument, 0, 2},
//                    {"average",         required_argument, 0, 3},
//                    {"color_weight",    required_argument, 0, 4},
//                    {"distance_weight", required_argument, 0, 5},
//                    {"dump",            no_argument,       0, 6},
//                    {0, 0,                                 0, 0}
//            };
//
//    int ch;
//    while((ch = getopt_long_only(argc, argv, "", long_options, NULL)) != -1) {
//        switch(ch) {
//            case 0:
//                photo = optarg;
//                break;
//            case 1:
//                superpixel_method = optarg;
//                break;
//            case 2:
//                color_space = optarg;
//                break;
//            case 3:
//                average_method = optarg;
//                break;
//            case 4:
//                color_weight = atoi(optarg);
//                break;
//            case 5:
//                distance_weight = atoi(optarg);
//                break;
//            case 6:
//                dump_intermediate = true;
//                break;
//            default:
//                break;
//        }
//    }
//
//    if(photo == "") {
//        std::cout << "No photo specified, use option --photo [path]";
//        exit(0x1ff);
//    }
//
//    cv::Mat image = cv::imread(photo);
//
//    merw::Superpixel superpixel(cv::ximgproc::SLIC, 10, 7.f, 30);
//
//    merw::Merw merw(superpixel, 0.85);
//
//    merw.process(image);
//
//    exit(0);
}

