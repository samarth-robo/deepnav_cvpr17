import numpy as np
import sys
import os.path as osp
import pickle
from IPython.core.debugger import Tracer

def get_key(im_filename):
  s = im_filename.split('/')
  city = s[3]
  idx = s[-1][:-4]
  key = '{:s}/{:s}'.format(city, idx)
  return key

class SKLearnInterface:
  def __init__(self, city, dest_name):
    # self.city = city
    self.city = 'combine'
    self.dest_name = dest_name
    self.base_dir = osp.join('../data/dataset', self.city)
    dict_filename = osp.join('../data/dataset/test_preds_{:s}.pk'.format(self.dest_name))

    print('Loading {:s}...'.format(dict_filename))
    with open(dict_filename, 'r') as f:
      self.pred_dict = pickle.load(f)
    print('SKLearn init done')

  def get_pred(self, im_filename):
    key = get_key(im_filename)
    try:
      pred = self.pred_dict[key]
    except KeyError:
      pred = 0
    return pred

if __name__ == '__main__':
    if len(sys.argv) != 3:
      print('Usage: python {:s} city dest_name'.format(sys.argv[0]))
      sys.exit(-1)

    ski = SKLearnInterface(sys.argv[1], sys.argv[2])
    pred = ski.get_pred('../data/dataset/dallas/images/0000141573_1.jpg')
    print(pred)
