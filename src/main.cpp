#include <iostream>
#include <getopt.h>

#include "Merw.h"

int main(int argc, char** argv) {
    std::string photo = "";

    std::string superpixel_method = "slic:4:10:20"; // slic, slico, lsc

    std::string color_space = "bgr2"; // rgb, bgr2
    std::string average_method = "average"; // average, mean, min, max
    float color_weight = 0.9; // 0 .. 1 - dist
    float distance_weight = 0.1; // 0 .. 1 - color

    bool dump_intermediate = false;

    static struct option long_options[] =
            {
                    {"photo",           required_argument, 0, 0},
                    {"superpixel",      required_argument, 0, 1},
                    {"color_space",     required_argument, 0, 2},
                    {"average",         required_argument, 0, 3},
                    {"color_weight",    required_argument, 0, 4},
                    {"distance_weight", required_argument, 0, 5},
                    {"dump",            no_argument,       0, 6},
                    {0, 0,                                 0, 0}
            };

    int ch;
    while((ch = getopt_long_only(argc, argv, "", long_options, NULL)) != -1) {
        switch(ch) {
            case 0:
                photo = optarg;
                break;
            case 1:
                superpixel_method = optarg;
                break;
            case 2:
                color_space = optarg;
                break;
            case 3:
                average_method = optarg;
                break;
            case 4:
                color_weight = atoi(optarg);
                break;
            case 5:
                distance_weight = atoi(optarg);
                break;
            case 6:
                dump_intermediate = true;
                break;
            default:
                break;
        }
    }

    if(photo == "") {
        std::cout << "No photo specified, use option --photo [path]";
        exit(0x1ff);
    }

    cv::Mat image = cv::imread(photo);

    merw::Superpixel superpixel(cv::ximgproc::SLIC, 20, 10.f, 10);

    merw::Merw merw(superpixel, 0.85);

    merw.process(image);

    exit(0);
}

