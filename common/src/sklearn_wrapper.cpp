#include <sklearn_wrapper.h>

using namespace std;
using namespace cv;
namespace py = boost::python;

SKLearnWrapper::SKLearnWrapper(string city, string dest_name) {
  try {
    // init Python
    Py_Initialize();
    char *pyargv = (char *)"deepnav.py";
    PySys_SetArgv(1, &pyargv);
    py::object main_module = py::import("__main__");
    py::object main_namespace = main_module.attr("__dict__");
    // add python modules path to sys path
    py::object sys_module = py::import("sys");
    sys_module.attr("path").attr("insert")(0, "../python/");
    // import python module
    py::object deepnav = py::import("deepnav");
    sklearn_interface = deepnav.attr("SKLearnInterface")(city, dest_name);
  } catch(py::error_already_set) {
    PyErr_Print();
  }
}

float SKLearnWrapper::get_pred(string im_filename) {
  float out = 0;
  try {
    out = py::extract<float>(sklearn_interface.attr("get_pred")
        (im_filename));
  } catch(py::error_already_set) {
    PyErr_Print();
  }
  return out;
}
