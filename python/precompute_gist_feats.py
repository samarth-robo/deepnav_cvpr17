import numpy as np
import os.path as osp
import lmdb
import cv2
from gist import GISTExtractor
from baseline import pool_init, gist_wrapper
from multiprocessing import Pool
import glob

class GISTPrecompute:
  def __init__(self, city):
    self.city = city
    self.base_dir = osp.join('../data/dataset', city)
    self.im_dir = osp.join(self.base_dir, 'images')
    lmdb_path = '../data/dataset/gist'
    self.db = lmdb.open(lmdb_path, map_size=int(1e12))

  def collect_data_parallel(self):
    im_filenames = glob.glob(osp.join(self.im_dir, '*.jpg'))
    block_size = 4000
    n_blocks = np.ceil(len(im_filenames) / float(block_size)).astype(np.int)

    ge = GISTExtractor(width=256, height=256)
    im = cv2.imread(im_filenames[0])
    gist_feat_size = ge.extract_gist(im).shape[0]

    pool = Pool(initializer=pool_init, initargs=(256, 256))
    txn = self.db.begin(write=True)

    for b in xrange(n_blocks):
      print '##### Block {:d} / {:d}'.format(b, n_blocks)

      im_fs = im_filenames[b*block_size : (b+1)*block_size]
      gist_feats = np.zeros((len(im_fs), gist_feat_size), dtype=np.float)

      chunksize = len(im_fs) / 4
      for idx, feat in enumerate(pool.imap(gist_wrapper, im_fs, chunksize)):
        gist_feats[idx, :] = feat

      for im_name, feat in zip(im_fs, gist_feats):
        key = '{:s}/{:s}'.format(self.city, im_name.split('/')[-1][:-4])
        txn.put(key, feat.tostring())
      txn.commit()
      txn = lmdb.Transaction(env=self.db, write=True)
    
    # terminate
    pool.close()
    pool.join()
    txn.commit()
    self.db.close()

if __name__ == '__main__':
    cities = ['atlanta', 'boston', 'chicago', 'dallas', 'houston', 'los_angeles',
              'new_york', 'philadelphia', 'phoenix', 'san_francisco']
    # cities = ['san_francisco']
    for city in cities:
      print '@@@@@ {:s} @@@@@'.format(city)
      gp = GISTPrecompute(city)
      gp.collect_data_parallel()
