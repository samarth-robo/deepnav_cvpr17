# implements the SVR baseline from Khosla et al
import numpy as np
import sys
import os.path as osp
import h5py
import pickle
from sklearn.svm import LinearSVR
from sklearn.preprocessing import StandardScaler
from sklearn.externals import joblib
from IPython.core.debugger import Tracer

def get_key(im_filename):
  s = im_filename.split('/')
  city = s[-3]
  idx = s[-1][:-4]
  key = '{:s}/{:s}'.format(city, idx)
  return key

class Baseline:
  def __init__(self, city, dest_name):
    self.city = city
    self.dest_name = dest_name
    print 'Baseline implementation for {:s} : {:s}'.format(self.city,
                                                           self.dest_name)
    dest_to_idx = {'bofa': 0, 'church': 1, 'gas_station': 3,
                   'high_school': 3, 'mcdonalds': 4}
    self.idx = dest_to_idx[self.dest_name]
    self.base_dir = osp.join('../data/dataset', city)
    self.train_label_filename = osp.join(self.base_dir, 'distance',
                                   'train_labels.h5')
    self.train_im_list_filename = osp.join(self.base_dir, 'distance',
                                     'train_im_list.txt')
    self.test_label_filename = osp.join(self.base_dir, 'distance',
                                         'test_labels.h5')
    self.test_im_list_filename = osp.join(self.base_dir, 'distance',
                                           'test_im_list.txt')
    self.svr = LinearSVR(verbose=1, epsilon=0, dual=False, tol=1e-3, max_iter=50000,
        loss='squared_epsilon_insensitive')
    self.scaler = StandardScaler(copy=False)
    self.model_filename = osp.join(self.base_dir, 'distance',
                              '{:s}.pkl'.format(self.dest_name))

  def collect_train_data(self):
    with open(self.train_im_list_filename, 'r') as train_f_im:
      train_im_names = [l.rstrip() for l in train_f_im]

    print 'Loading train data...'
    with h5py.File('../data/dataset/train_feats1.mat', 'r') as f:
      self.train_X = np.asarray(f['train_features'], dtype=np.float32).T

    with h5py.File(self.train_label_filename, 'r') as train_f_label:
      self.train_y = train_f_label['label'][:, self.idx].astype(np.float32)

    # select cities and remove rogue labels
    # idx = [i for i,n in enumerate(train_im_names) if ((('boston' in n)) and self.train_y[i] < 1e3)]
    idx = [i for i,n in enumerate(train_im_names) if self.train_y[i] < 1e3]

    self.train_X = self.train_X[idx, :]
    self.train_y = self.train_y[idx]

    assert(self.train_y.shape[0] == self.train_X.shape[0])
    print 'Done, using {:d} images for training'.format(self.train_X.shape[0])

  def train(self, C=1.0):
    print 'Scaling...'
    self.train_X = self.scaler.fit_transform(self.train_X)
    print 'Training with C = {:f}'.format(C)
    p = self.svr.get_params()
    p['C'] = C
    self.svr.set_params(**p)
    self.svr.fit(self.train_X, self.train_y)

  def save_predictions(self):
    with h5py.File('../data/dataset/test_feats.mat', 'r') as f:
      print 'Loading feats...'
      self.test_X = np.asarray(f['test_features'], dtype=np.float32).T

    with open('../data/dataset/test_filenames.txt', 'r') as f:
      im_names = [n.rstrip() for n in f]
    keys = [get_key(im_name) for im_name in im_names]

    assert(len(im_names) == self.test_X.shape[0])

    print 'Loading models...'
    d = joblib.load(self.model_filename)
    self.svr = d['svr']
    self.scaler = d['scaler']

    print 'Scaling...'
    self.test_X = self.scaler.transform(self.test_X)
    print 'Predicting...'
    preds = self.svr.predict(self.test_X)
    print 'Done!'
    pred_dict = {key: pred for (key, pred) in zip(keys, preds)}
    fn = '../data/dataset/test_preds_{:s}.pk'.format(self.dest_name)
    with open(fn, 'w') as f:
      pickle.dump(pred_dict, f)
    print 'Saved', fn

  def save_current_model(self):
    joblib.dump({'svr': self.svr, 'scaler': self.scaler}, self.model_filename)
    print self.model_filename, 'saved'

if __name__ == '__main__':
    if len(sys.argv) != 3:
      print 'Usage: python {:s} city dest_name'.format(sys.argv[0])
      sys.exit(-1)
    b = Baseline(sys.argv[1], sys.argv[2])
    b.collect_train_data()
    b.train(10.0)
    b.save_current_model()
    b.save_predictions()
