#include <iostream>
#include <Python.h>
#include <string>
#include <boost/python.hpp>

using namespace std;
namespace py = boost::python;

int main() {
  Py_Initialize();

  py::object main_module = py::import("__main__");
  py::object main_namespace = main_module.attr("__dict__");

  // add python modules path to sys path
  py::object sys_module = py::import("sys");
  sys_module.attr("path").attr("insert")(0, "../python/");

  // blensor script
  py::object deepnav = py::import("deepnav");
  string obj_filename("~/Downloads/matterport/home_centered.obj");
  py::object blensor = deepnav.attr("BlensorInterface")(obj_filename);
  return 0;
}
