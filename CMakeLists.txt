cmake_minimum_required (VERSION 2.8.11)
project (project4)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
find_package( OpenCV REQUIRED )

add_executable (project4 main.cpp)
target_link_libraries( project4 ${OpenCV_LIBS} )
