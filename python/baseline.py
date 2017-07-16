# implements the SVR baseline from Khosla et al
import numpy as np
import cv2
import sys
import os.path as osp
import lmdb
from multiprocessing import Pool
from gist import GISTExtractor
from deepnav import get_key
from sklearn.svm import SVR, LinearSVR
from sklearn.externals import joblib

def pool_init(*args):
  global ge
  ge = GISTExtractor(*args)

def gist_wrapper(im_filename):
  im = cv2.imread(im_filename)
  if im is None:
    print 'Could not read', im_filename
    sys.exit(-1)
  return ge.extract_gist(im)

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
                                   'train_labels.txt')
    self.train_im_list_filename = osp.join(self.base_dir, 'distance',
                                     'train_im_list.txt')
    self.test_label_filename = osp.join(self.base_dir, 'distance',
                                         'test_labels.txt')
    self.test_im_list_filename = osp.join(self.base_dir, 'distance',
                                           'test_im_list.txt')
    self.svr = SVR(kernel='linear', shrinking=False, cache_size=10000, verbose=True)
    # self.svr = LinearSVR(verbose=1)

  def collect_train_data_parallel(self):
    with open(self.train_im_list_filename, 'r') as train_f_im,\
        open(self.train_label_filename, 'r') as train_f_label:
      train_im_names = [osp.join('../data/dataset', l.rstrip().split(' ')[0])
                        for l in train_f_im]
      train_labels = [float(l.rstrip().split(' ')[self.idx]) for l in train_f_label]

    # get dims
    ge = GISTExtractor(width=256, height=256)
    im = cv2.imread(train_im_names[0])
    gist_features = ge.extract_gist(im)
    self.train_X = np.zeros((len(train_im_names), gist_features.shape[0]), dtype=np.float)
    self.train_y = np.asarray(train_labels)

    # parallel feature extraction!
    print 'Collecting training data'
    pool = Pool(initializer=pool_init, initargs=(256, 256))
    chunksize = len(train_im_names) / 4
    for idx, feat in enumerate(pool.imap(gist_wrapper, train_im_names,
                                         chunksize)):
      self.train_X[idx, :] = feat

    pool.close()
    pool.join()

  def collect_train_data_serial(self):
    with open(self.train_im_list_filename, 'r') as train_f_im,\
        open(self.train_label_filename, 'r') as train_f_label:
      train_im_names = [osp.join('../data/dataset', l.rstrip().split(' ')[0])
                        for l in train_f_im]
      train_labels = [float(l.rstrip().split(' ')[self.idx]) for l in train_f_label]

    # get dims
    ge = GISTExtractor(width=256, height=256)
    im = cv2.imread(train_im_names[0])
    gist_features = ge.extract_gist(im)
    self.train_X = np.zeros((len(train_im_names), gist_features.shape[0]), dtype=np.float)
    self.train_y = np.asarray(train_labels)

    db = lmdb.open('../data/dataset/gist', map_size=int(1e12), readonly=True)
    txn = db.begin()

    # serial feature extraction!
    print 'Collecting training data'
    for idx, im_name in enumerate(train_im_names):
      if idx % 100 == 0:
        print 'Image {:d} / {:d}'.format(idx, len(train_im_names))
      key = get_key(im_name)
      self.train_X[idx, :] = np.fromstring(txn.get(key))

  def collect_test_data_parallel(self):
    with open(self.test_im_list_filename, 'r') as test_f_im,\
        open(self.test_label_filename, 'r') as test_f_label:
      test_im_names = [osp.join('../data/dataset', l.rstrip().split(' ')[0])
                       for l in test_f_im]
      test_labels = [float(l.rstrip().split(' ')[self.idx]) for l in test_f_label]

    # get dims
    ge = GISTExtractor(width=256, height=256)
    im = cv2.imread(test_im_names[0])
    gist_features = ge.extract_gist(im)
    self.test_X = np.zeros((len(test_im_names), gist_features.shape[0]),
                           dtype=np.float)
    self.test_y = np.asarray(test_labels)

    # parallel feature extraction!
    print 'Collecting testing data'
    pool = Pool(initializer=pool_init, initargs=(256, 256))
    chunksize = len(test_im_names) / 4
    for idx, feat in enumerate(pool.imap(gist_wrapper, test_im_names,
                                         chunksize)):
      self.test_X[idx, :] = feat
    pool.close()
    pool.join()

  def collect_test_data_serial(self):
    with open(self.test_im_list_filename, 'r') as test_f_im,\
        open(self.test_label_filename, 'r') as test_f_label:
      test_im_names = [osp.join('../data/dataset', l.rstrip().split(' ')[0])
                        for l in test_f_im]
      test_labels = [float(l.rstrip().split(' ')[self.idx]) for l in test_f_label]

    # get dims
    ge = GISTExtractor(width=256, height=256)
    im = cv2.imread(test_im_names[0])
    gist_features = ge.extract_gist(im)
    self.test_X = np.zeros((len(test_im_names), gist_features.shape[0]),
                           dtype=np.float)
    self.test_y = np.asarray(test_labels)

    db = lmdb.open('../data/dataset/gist', map_size=int(1e12), readonly=True)
    txn = db.begin()

    # serial feature extraction!
    print 'Collecting testing data'
    for idx, im_name in enumerate(test_im_names):
      if idx % 100 == 0:
        print 'Image {:d} / {:d}'.format(idx, len(test_im_names))
      key = get_key(im_name)
      self.test_X[idx, :] = np.fromstring(txn.get(key))

  def train(self, C=1.0, calc_loss=False):
    print 'Training with C = {:f}'.format(C)
    p = self.svr.get_params()
    p['C'] = C
    self.svr.set_params(**p)
    self.svr.fit(self.train_X, self.train_y)
    loss = 0
    if calc_loss:
      test_y_pred = self.svr.predict(self.test_X)
      loss = np.linalg.norm(test_y_pred - self.test_y)
      # score = self.svr.score(self.test_X, self.test_y)
      print 'Loss = {:f}'.format(loss)
    return loss

  def cross_validate(self):
    C = np.power(10.0, xrange(-2, 5))
    losses = np.array([self.train(c, calc_loss=True) for c in C])
    idx = np.argmin(losses)
    print 'Best C = {:f}'.format(C[idx])

  def save_current_model(self):
    model_filename = osp.join(self.base_dir, 'distance',
                              '{:s}.pkl'.format(self.dest_name))
    joblib.dump(self.svr, model_filename)
    print model_filename, 'saved'

if __name__ == '__main__':
    if len(sys.argv) != 3:
      print 'Usage: python {:s} city dest_name'.format(sys.argv[0])
      sys.exit(-1)
    b = Baseline(sys.argv[1], sys.argv[2])
    b.collect_train_data_serial()
    # b.collect_test_data_parallel()
    b.train(10.0)
    b.save_current_model()
    # b.cross_validate()
