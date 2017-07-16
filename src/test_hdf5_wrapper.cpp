#include <hdf5_wrapper.h>

#include <iostream>
#include <vector>

using namespace std;

int main(int argc, char **argv) {
  HDF5Wrapper *f = new HDF5Wrapper("../data/test_hdf5.h5", "test", 2);

  vector<float> data{1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 18.f};
  vector<size_t> data_dims{2, 4};

  bool done = f->write(data, data_dims);
  done &= f->write(data, data_dims);
  delete f;
  return (done ? 0 : -1);
}
