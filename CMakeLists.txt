cmake_minimum_required(VERSION 3.3)
project(smart_wallpapers)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

find_package(OpenCV REQUIRED)
set(SOURCE_FILES src/main.cpp src/Merw.cpp src/Merw.h src/Superpixel.cpp src/Superpixel.h)
add_executable(smart_wallpapers ${SOURCE_FILES})
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(smart_wallpapers ${OpenCV_LIBS})