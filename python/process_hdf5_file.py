import numpy as np
import h5py
import sys

class HDF5Processor:
  def __init__(self, input_file, output_file):
    self.input_file = input_file
    self.output_file = output_file

  def process(self):
    with h5py.File(self.input_file, 'r') as f:
      data_in = np.array(f.get('label'))
    print 'Read ', self.input_file
    
    data_out = self.fn(data_in)
    
    with h5py.File(self.output_file, 'w') as f:
      f.create_dataset('label', data=data_out)
    print 'Written ', self.output_file
  
  def fn(self, data_in):
    data_out = np.log(data_in + np.finfo(float).eps)
    return data_out

if __name__ == '__main__':
  if len(sys.argv) != 3:
    print 'Usage: python process_hdf5_file.py in.h5 out.h5'
    sys.exit(-1)

  p = HDF5Processor(sys.argv[1], sys.argv[2])
  p.process()
