#include <hdf5_wrapper.h>

#include <iostream>

using namespace H5;
using namespace std;

HDF5Wrapper::HDF5Wrapper(string filename, string dataset_name, int ndims) :
  ndims(ndims) {
  try {
    file = H5File(filename.c_str(), H5F_ACC_TRUNC);
    dims = new hsize_t[ndims]();  // init to 0

    hsize_t *max_dims = new hsize_t[ndims],
            *chunk_dims = new hsize_t[ndims];
    for(int i = 0; i < ndims; i++) {
      max_dims[i] = H5S_UNLIMITED;
      chunk_dims[i] = 10;
    }
    dataspace = new DataSpace(ndims, dims, max_dims);
    DSetCreatPropList prop;
    prop.setChunk(ndims, chunk_dims);
    dataset = new DataSet(file.createDataSet(dataset_name.c_str(),
          PredType::NATIVE_FLOAT, *dataspace, prop));
    delete[] max_dims;
    delete[] chunk_dims;
  } catch(FileIException error) {
    error.printError();
  } catch(DataSetIException error) {
    error.printError();
  } catch(DataSpaceIException error) {
    error.printError();
  }
}

bool HDF5Wrapper::write(vector<float> data, vector<size_t> data_dims) {
  try {
    if(data_dims.size() != ndims) {
      cout << "ndims = " << ndims << ", size of data_dims = " << data_dims.size() << endl;
      return false;
    }
    hsize_t *offset = new hsize_t[ndims](), *ext_dims = new hsize_t[ndims];
    offset[0] = dims[0] ? dims[0] : 0;
    size_t n_elems = 1;
    for(int i = 0; i < ndims; i++) {
      n_elems *= data_dims[i];
      ext_dims[i] = data_dims[i];
      if(i > 0 && data_dims[i] != dims[i] && dims[i] != 0) {
        cout << "Dims don't match" << endl;
        return false;
      }
      dims[i] = (i==0) ? dims[i]+data_dims[i] : data_dims[i];
    }
    if(n_elems != data.size()) {
      cout << "n_elems = " << n_elems << ", data.size() = " << data.size() << endl;
      return false;
    }
    dataset->extend(dims);
    DataSpace *filespace = new DataSpace(dataset->getSpace());
    filespace->selectHyperslab(H5S_SELECT_SET, ext_dims, offset);
    DataSpace *memspace = new DataSpace(ndims, ext_dims, NULL);
    dataset->write(&data[0], PredType::NATIVE_FLOAT, *memspace, *filespace);
    delete filespace;
    delete memspace;
    delete[] offset;
    delete[] ext_dims;
  } catch(FileIException error) {
    error.printError();
    return false;
  } catch(DataSetIException error) {
    error.printError();
    return false;
  } catch(DataSpaceIException error) {
    error.printError();
    return false;
  }
  return true;
}

HDF5Wrapper::~HDF5Wrapper() {
  try {
    delete dataspace;
    delete dataset;
    delete[] dims;
    file.close();
  } catch(FileIException error) {
    error.printError();
  } catch(DataSetIException error) {
    error.printError();
  } catch(DataSpaceIException error) {
    error.printError();
  }
}
