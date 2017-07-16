from geo_utils import Binner
import os.path as osp
import numpy as np
import matplotlib.pyplot as plt
from IPython.core.debugger import Tracer
import sys

def remove_all_plt_margins(ax):
  '''get ax by plt.gca() after plt.show()'''
  '''Also needs savefig() with bbox_inches='tight' and pad_inches=0'''
  '''and ipython'''
  ax.set_axis_off()
  plt.subplots_adjust(top=1, bottom=0, right=1, left=0, hspace=0, wspace=0)
  plt.margins(0, 0)
  ax.xaxis.set_major_locator(plt.NullLocator())
  ax.yaxis.set_major_locator(plt.NullLocator())
 
class EntropyPlotter:
  def __init__(self, city, dst_name, experiment):
    self.city = city
    self.experiment = experiment
    self.dst_name = dst_name
    self.binner = Binner(city)
    self.base_dir = osp.join('../data/dataset', self.city)
    self.entropy_file = osp.join(self.base_dir,
                                 '{:s}_{:s}_entropy.txt'.format(self.dst_name, self.experiment))
    self.dst_file = osp.join(self.base_dir, 'dsts_script.txt')

  def plot_entropy(self):
    im = np.zeros((self.binner.n_bins_y, self.binner.n_bins_x))

    # plot entropy data
    data = np.loadtxt(self.entropy_file, delimiter=',', skiprows=1)
    bin_ids = np.array([self.binner.get_bin_id(d[0], d[1]) for d in data])
    xs = bin_ids % self.binner.n_bins_x
    ys = bin_ids / self.binner.n_bins_x
    entropy_sc = data[:, 2]
    idx = np.logical_and(bin_ids > 0, np.logical_not(np.isnan(entropy_sc)))
    entropy_sc = entropy_sc[idx]
    entropy_sc += entropy_sc.min()
    entropy_sc /= entropy_sc.max()
    im[ys[idx], xs[idx]] = entropy_sc
    plt.imshow(im, cmap='afmhot', interpolation='none')

    # plot dsts
    with open(self.dst_file, 'r') as f:
      lines = [l.rstrip() for l in f]
    bin_ids = np.asarray([int(l.split(' ')[-1]) for l in lines if l.split(' ')[0] == self.dst_name])
    xs = bin_ids % self.binner.n_bins_x
    ys = bin_ids / self.binner.n_bins_x
    plt.scatter(xs, ys, marker='o', c=[[0,1,1]], s=40)

    plt.show(block=False)
    remove_all_plt_margins(plt.gca())
    fn = '../../cvpr17_deepnav/images/' +\
        '{:s}_{:s}_{:s}_entropy.png'.format(self.city, self.dst_name, self.experiment)
    plt.savefig(fn, bbox_inches = 'tight', pad_inches = 0)
    print fn, 'saved'

if __name__ == '__main__':
  if len(sys.argv) != 4:
    print 'Usage: {:s} city dest_name experiment'.format(sys.argv[0])
    sys.exit(-1)

  ep = EntropyPlotter(sys.argv[1], sys.argv[2], sys.argv[3])
  ep.plot_entropy()
