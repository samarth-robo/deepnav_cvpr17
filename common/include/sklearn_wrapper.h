#ifndef SKLEARN_WRAPPER_H
#define SKLEARN_WRAPPER_H

#include <Python.h>
#include <boost/python.hpp>
#include <opencv2/opencv.hpp>

class SKLearnWrapper {
  public:
    SKLearnWrapper(std::string city, std::string dest_name);
    float get_pred(std::string im_filename);
  private:
    boost::python::object sklearn_interface;
};

#endif