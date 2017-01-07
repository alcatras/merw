//
// Created by kamil on 07.01.17.
//

#ifndef SMART_WALLPAPERS_UTILS_H
#define SMART_WALLPAPERS_UTILS_H

#include <vector>
#include <sstream>

std::vector<std::string> parse_args(const std::string& argv) {
    std::vector<std::string> args;

    std::stringstream ss;
    ss << argv;

    while(!ss.eof()) {
        std::string arg;
        std::getline(ss, arg, ':');
        args.emplace_back(arg);
    }

    return args;
}


#endif //SMART_WALLPAPERS_UTILS_H
