import numpy as np
import sys
from geo_utils import Binner

class TestLocGenerator(Binner):
  def __init__(self, city):
    Binner.__init__(self, city)

    # read destinations
    self.destinations = {}
    with open('{:s}/dsts_script.txt'.format(self.base_dir)) as f:
      for line in f:
        name, node_id = line.rstrip().split(' ')
        node_id = int(node_id)
        if self.destinations.has_key(name):
          self.destinations[name].append(node_id)
        else:
          self.destinations[name] = [node_id]

  def id_to_xy(self, id):
    ny = id / self.n_bins_x
    nx = id % self.n_bins_x
    return nx, ny

  def xy_to_id(self, x, y):
    idx = y * self.n_bins_x + x
    rm_idx = np.logical_or.reduce((x >= self.n_bins_x, y >= self.n_bins_y,
                                   x < 0, y < 0))
    idx[rm_idx] = -1
    return idx

  def generate_locs(self, r_m):
    r_bins = np.ceil(r_m * self.n_bins_x / self.width_m)
    for dest_name, dest_ids in self.destinations.items():
      x_dest, y_dest = self.id_to_xy(np.array(dest_ids))
      theta = np.linspace(0, 2*np.pi, 10)
      x = np.round(x_dest[:, np.newaxis] + r_bins * np.cos(theta))
      y = np.round(y_dest[:, np.newaxis] + r_bins * np.sin(theta))
      x = x.reshape((-1, 1))
      y = y.reshape((-1, 1))
      r_ids = self.xy_to_id(x, y)
      r_ids = r_ids[r_ids[:,0] > 0, :]
      r_lats, r_lngs = self.get_bin_loc(r_ids)
      filename = '{:s}/{:s}_test_locs_850.txt'.format(self.base_dir, dest_name)
      np.savetxt(filename, np.hstack((r_ids, r_lats, r_lngs)),
                 fmt=['%d', '%10.7f', '%10.7f'], delimiter=',',
                 header='bin_id,latitude,longitude')
      print 'Saved', filename

if __name__ == '__main__':
    if len(sys.argv) != 2:
      print 'Usage: python {:s} city'.format(sys.argv[0])
      sys.exit(-1)

    tg = TestLocGenerator(sys.argv[1])
    tg.generate_locs(850)
