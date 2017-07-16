cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(deepnav)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "-std=c++0x")

find_package(OpenCV REQUIRED)
find_package(Boost COMPONENTS filesystem system REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(common)
include_directories(${PROJECT_SOUCE_DIR}/common/include)

add_subdirectory(utils)
include_directories(${PROJECT_SOURCE_DIR}/utils/include)

add_executable(test_cleaner src/test_cleaner.cpp)
target_link_libraries(test_cleaner common)

add_executable(test_caffe_wrapper src/test_caffe_wrapper.cpp)
target_link_libraries(test_caffe_wrapper common ${Boost_LIBRARIES})

add_executable(test_hdf5_wrapper src/test_hdf5_wrapper.cpp)
target_link_libraries(test_hdf5_wrapper common)
 
add_executable(test_graph_maker src/test_graph_maker.cpp)
target_link_libraries(test_graph_maker utils)

add_executable(test_search src/test_search.cpp)
target_link_libraries(test_search utils common)

add_executable(test_dataset_maker src/test_dataset_maker.cpp)
target_link_libraries(test_dataset_maker utils common)

add_executable(test_chance src/test_chance.cpp)
target_link_libraries(test_chance utils)

add_executable(test_tester src/test_tester.cpp)
target_link_libraries(test_tester utils common)

add_executable(test_eval src/test_eval.cpp)
target_link_libraries(test_eval utils)

add_executable(test_sklearn_wrapper src/test_sklearn_wrapper.cpp)
target_link_libraries(test_sklearn_wrapper common)

add_executable(test_entropy src/test_entropy.cpp)
target_link_libraries(test_entropy utils common)

add_executable(test_sort_nodes src/test_sort_nodes.cpp)
target_link_libraries(test_sort_nodes utils common)
