cmake_minimum_required(VERSION 3.18)
project(wxpolygon)

find_package(wxWidgets REQUIRED COMPONENTS net core base)
include(${wxWidgets_USE_FILE})
add_executable(wxpolygon wxpolygon.cpp)
target_link_libraries(wxpolygon ${wxWidgets_LIBRARIES})
