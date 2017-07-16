cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(Python_ADDITIONAL_VERSIONS 2.7)
find_package(PythonLibs 2.7 REQUIRED)
find_package(Boost REQUIRED python)
find_package(OpenCV REQUIRED)

add_library(common SHARED
  src/caffe_wrapper.cpp
  src/common.cpp
  src/hdf5_wrapper.cpp src/sklearn_wrapper.cpp)

target_link_libraries(common
  ~/libraries/caffe/build_2.7/lib/libcaffe.so
  hdf5
  hdf5_cpp
  ${OpenCV_LIBS} ${PYTHON_LIBRARIES})

target_include_directories(common PUBLIC
  ~/libraries/caffe/include
  ~/libraries/caffe/build_2.7/include
  ${CUDA_INCLUDE_DIRS}
  include
  ../utils/include ${PYTHON_INCLUDE_DIRS})
