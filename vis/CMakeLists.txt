cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../utils/include")

add_library(vis SHARED src/vis.cpp ../utils/src/graph.cpp)
target_link_libraries(vis ${PCL_LIBRARIES})
