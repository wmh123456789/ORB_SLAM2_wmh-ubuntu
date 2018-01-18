cmake_minimum_required(VERSION 2.8)
project(CodeTest)

set(CMAKE_CXX_STANDARD 11)
find_package(OpenCV 3.0 QUIET NO_DEFAULT_PATH PATHS "/usr/local/")
find_package(Pangolin REQUIRED)

include_directories(${Pangolin_INCLUDE_DIRS})

add_executable(CodeTest main.cpp QuadTree.cpp QuadTree.h FaceDetection.cpp FaceDetection.h)

target_link_libraries(CodeTest
        ${OpenCV_LIBS}
        ${Pangolin_LIBRARIES})