# script to combine datasets
import numpy as np
import os.path as osp
import h5py
import sys
from IPython.core.debugger import Tracer

class DatasetCombiner:
  '''
  output    : name of output folder in ../data/dataset/
  experiment: distance / direction / pair
  input_file: TXT file containing input datasets e.g. <city>_<train/test>
  '''
  def __init__(self, output, experiment, train_input_file, test_input_file):
    self.experiment = experiment
    self.output_dir = osp.join('../data/dataset', output, self.experiment)
    self.input_dir  = osp.join('../data/dataset', '{:s}', self.experiment)
    self.train_cities = []
    self.train_splits = []
    with open(train_input_file, 'r') as f:
      for line in f:
        s = line.strip().split('_')
        split = s[-1]
        city = ''
        for ss in s[:-1]:
          city += '{:s}_'.format(ss)
        city = city[:-1]
        self.train_cities.append(city)
        self.train_splits.append(split)
    self.test_cities = []
    self.test_splits = []
    with open(test_input_file, 'r') as f:
      for line in f:
        s = line.strip().split('_')
        split = s[-1]
        city = ''
        for ss in s[:-1]:
          city += '{:s}_'.format(ss)
        city = city[:-1]
        self.test_cities.append(city)
        self.test_splits.append(split)

  def combine(self):
    # generate filenames
    train_im_list_filename    = osp.join(self.output_dir, 'train_im_list.txt')
    test_im_list_filename     = osp.join(self.output_dir, 'test_im_list.txt')
    train_label_list_filename = osp.join(self.output_dir, 'train_label_list.txt')
    test_label_list_filename  = osp.join(self.output_dir, 'test_label_list.txt')
    train_label_txt_filename  = osp.join(self.output_dir, 'train_labels.txt')
    test_label_txt_filename   = osp.join(self.output_dir, 'test_labels.txt')
    train_label_hdf5_filename = osp.join(self.output_dir, 'train_labels.h5')
    test_label_hdf5_filename  = osp.join(self.output_dir, 'test_labels.h5')
    train_lw_hdf5_filename    = osp.join(self.output_dir, 'train_loss_weights.h5')
    test_lw_hdf5_filename     = osp.join(self.output_dir, 'test_loss_weights.h5')
    train_lw_list_filename    = osp.join(self.output_dir,
                                         'train_loss_weight_list.txt')
    test_lw_list_filename     = osp.join(self.output_dir,
                                         'test_loss_weight_list.txt')
    if self.experiment == 'pair':
      train_im_list_p_filename= osp.join(self.output_dir, 'train_im_list_p.txt')
      test_im_list_p_filename = osp.join(self.output_dir, 'test_im_list_p.txt')

    # HDF5 label list files
    with open(train_label_list_filename, 'w') as fout:
      fout.write('../{:s}\n'.format(train_label_hdf5_filename))
    with open(test_label_list_filename, 'w') as fout:
      fout.write('../{:s}\n'.format(test_label_hdf5_filename))

    # HDF5 loss weight list files
    with open(train_lw_list_filename, 'w') as fout:
      fout.write('../{:s}\n'.format(train_lw_hdf5_filename))
    with open(test_lw_list_filename, 'w') as fout:
      fout.write('../{:s}\n'.format(test_lw_hdf5_filename))

    # Image list files
    with open(train_im_list_filename, 'w') as fout:
      for city, split in zip(self.train_cities, self.train_splits):
        filename = osp.join(self.input_dir.format(city),
            '{:s}_im_list.txt'.format(split))
        print 'Reading {:s}...'.format(filename)
        with open(filename, 'r') as fin:
          fout.write(fin.read())
    print 'Created {:s}.'.format(train_im_list_filename)
    with open(test_im_list_filename, 'w') as fout:
      for city, split in zip(self.test_cities, self.test_splits):
        filename = osp.join(self.input_dir.format(city),
            '{:s}_im_list.txt'.format(split))
        print 'Reading {:s}...'.format(filename)
        with open(filename, 'r') as fin:
          fout.write(fin.read())
    print 'Created {:s}.'.format(test_im_list_filename)
    if self.experiment == 'pair':
      with open(train_im_list_p_filename, 'w') as fout:
        for city, split in zip(self.train_cities, self.train_splits):
          filename = osp.join(self.input_dir.format(city),
              '{:s}_im_list_p.txt'.format(split))
          print 'Reading {:s}...'.format(filename)
          with open(filename, 'r') as fin:
            fout.write(fin.read())
      print 'Created {:s}.'.format(train_im_list_p_filename)
      with open(test_im_list_p_filename, 'w') as fout:
        for city, split in zip(self.test_cities, self.test_splits):
          filename = osp.join(self.input_dir.format(city),
              '{:s}_im_list_p.txt'.format(split))
          print 'Reading {:s}...'.format(filename)
          with open(filename, 'r') as fin:
            fout.write(fin.read())
      print 'Created {:s}.'.format(test_im_list_p_filename)

    # Label TXT files
    with open(train_label_txt_filename, 'w') as fout:
      for city, split in zip(self.train_cities, self.train_splits):
        filename = osp.join(self.input_dir.format(city),
            '{:s}_labels.txt'.format(split))
        print 'Reading {:s}...'.format(filename)
        with open(filename, 'r') as fin:
          fout.write(fin.read())
    print 'Created {:s}.'.format(train_label_txt_filename)
    with open(test_label_txt_filename, 'w') as fout:
      for city, split in zip(self.test_cities, self.test_splits):
        filename = osp.join(self.input_dir.format(city),
            '{:s}_labels.txt'.format(split))
        print 'Reading {:s}...'.format(filename)
        with open(filename, 'r') as fin:
          fout.write(fin.read())
    print 'Created {:s}.'.format(test_label_txt_filename)

    # label HDF5 files
    with h5py.File(train_label_hdf5_filename, 'w') as fout:
      dout = fout.create_dataset('label', dtype='f',
          shape=(0, 0), maxshape=(None, 10))
      for city, split in zip(self.train_cities, self.train_splits):
        filename = osp.join(self.input_dir.format(city),
            '{:s}_labels.h5'.format(split))
        print 'Reading {:s}...'.format(filename)
        with h5py.File(filename, 'r') as fin:
          data_in = np.array(fin.get('label'))
        print 'Read {:d} entries'.format(data_in.shape[0])
        if dout.shape == (0, 0):
          offset = 0
          dout.resize((data_in.shape[0], data_in.shape[1]))
        else:
          offset = dout.shape[0]
          dout.resize(dout.shape[0]+data_in.shape[0], axis=0)
        dout[offset:, :] = data_in
    print 'Created {:s}.'.format(train_label_hdf5_filename)
    with h5py.File(test_label_hdf5_filename, 'w') as fout:
      dout = fout.create_dataset('label', dtype='f',
          shape=(0, 0), maxshape=(None, 10))
      for city, split in zip(self.test_cities, self.test_splits):
        filename = osp.join(self.input_dir.format(city),
            '{:s}_labels.h5'.format(split))
        print 'Reading {:s}...'.format(filename)
        with h5py.File(filename, 'r') as fin:
          data_in = np.array(fin.get('label'))
        print 'Read {:d} entries'.format(data_in.shape[0])
        if dout.shape == (0, 0):
          offset = 0
          dout.resize((data_in.shape[0], data_in.shape[1]))
        else:
          offset = dout.shape[0]
          dout.resize(dout.shape[0]+data_in.shape[0], axis=0)
        dout[offset:, :] = data_in
    print 'Created {:s}.'.format(test_label_hdf5_filename)

    # loss weight HDF5 files
    with h5py.File(train_lw_hdf5_filename, 'w') as fout:
      dout = fout.create_dataset('loss_weight', dtype='f',
          shape=(0, 0), maxshape=(None, 10))
      for city, split in zip(self.train_cities, self.train_splits):
        filename = osp.join(self.input_dir.format(city),
            '{:s}_loss_weights.h5'.format(split))
        print 'Reading {:s}...'.format(filename)
        with h5py.File(filename, 'r') as fin:
          data_in = np.array(fin.get('loss_weight'))
        print 'Read {:d} entries'.format(data_in.shape[0])
        if dout.shape == (0, 0):
          offset = 0
          dout.resize((data_in.shape[0], data_in.shape[1]))
        else:
          offset = dout.shape[0]
          dout.resize(dout.shape[0]+data_in.shape[0], axis=0)
        dout[offset:, :] = data_in
    print 'Created {:s}.'.format(train_lw_hdf5_filename)
    with h5py.File(test_lw_hdf5_filename, 'w') as fout:
      dout = fout.create_dataset('loss_weight', dtype='f',
          shape=(0, 0), maxshape=(None, 10))
      for city, split in zip(self.test_cities, self.test_splits):
        filename = osp.join(self.input_dir.format(city),
            '{:s}_loss_weights.h5'.format(split))
        print 'Reading {:s}...'.format(filename)
        with h5py.File(filename, 'r') as fin:
          data_in = np.array(fin.get('loss_weight'))
        print 'Read {:d} entries'.format(data_in.shape[0])
        if dout.shape == (0, 0):
          offset = 0
          dout.resize((data_in.shape[0], data_in.shape[1]))
        else:
          offset = dout.shape[0]
          dout.resize(dout.shape[0]+data_in.shape[0], axis=0)
        dout[offset:, :] = data_in
    print 'Created {:s}.'.format(test_lw_hdf5_filename)

if __name__ == '__main__':
  if len(sys.argv) != 5:
    print 'Usage: python {:s} output experiment'.format(sys.argv[0]) +\
        ' train_input_file test_input_file'
    sys.exit(-1)

  dc = DatasetCombiner(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])
  dc.combine()
