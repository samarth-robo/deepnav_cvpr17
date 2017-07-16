#ifndef HDF5WRAPPER_H
#define HDF5WRAPPER_H

#include <H5Cpp.h>
#include <vector>

class HDF5Wrapper {
  private:
    int ndims;
    hsize_t *dims;
    H5::H5File file;
    H5::DataSpace *dataspace;
    H5::DataSet *dataset;
  public:
    HDF5Wrapper(std::string filename, std::string dataset_name, int ndims);
    bool write(std::vector<float> data, std::vector<size_t> data_dims);
    ~HDF5Wrapper();
};

#endif
