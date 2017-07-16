cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

find_package(GeographicLib REQUIRED)

add_library(utils SHARED
  src/graph.cpp
  src/graph_maker.cpp
  src/search.cpp
  src/dataset_maker.cpp
  src/tester.cpp
  src/eval.cpp)
target_include_directories(utils PUBLIC include ${GeographicLib_INCLUDE_DIRS})
target_link_libraries(utils common ${GeographicLib_LIBRARIES} ${Boost_LIBRARIES})

# add_library(utils SHARED
#   src/dataset_maker.cpp
#   src/eval.cpp
#   src/graph.cpp
#   src/graph_maker.cpp
#   src/plane_estimator.cpp
#   src/search.cpp
#   src/tester.cpp)
# target_link_libraries(utils common vis)
# target_include_directories(utils PUBLIC include)
