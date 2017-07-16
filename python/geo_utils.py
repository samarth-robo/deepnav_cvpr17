import numpy as np
import os
import sys
from pyproj import Geod
from IPython.core.debugger import Tracer

class Binner:
  def __init__(self, city):
      self.geo = Geod(ellps='clrk66')
      
      # files
      self.base_dir = '../data/dataset/{:s}/'.format(city)
      limits = np.loadtxt(self.base_dir + '/box.txt')
      self.top_left = limits[0]
      self.bottom_right = limits[1]

      # resolution in meters and in grid units
      res_m = 25.0  # meters
      _, _, self.width_m  = self.geo.inv(self.top_left[1], self.top_left[0],
          self.bottom_right[1], self.top_left[0])
      _, _, self.height_m = self.geo.inv(self.top_left[1], self.top_left[0],
          self.top_left[1], self.bottom_right[0])
      self.n_bins_x = self.width_m / res_m
      self.n_bins_y = self.height_m / res_m
      self.res_lat = abs(self.top_left[0] - self.bottom_right[0]) / self.n_bins_y
      self.res_lng = abs(self.top_left[1] - self.bottom_right[1]) / self.n_bins_x

      # bins
      self.n_bins_x = np.ceil(self.n_bins_x).astype(int)
      self.n_bins_y = np.ceil(self.n_bins_y).astype(int)
      self.n_bins = self.n_bins_x * self.n_bins_y
      
  def get_bin_id(self, lat, lng):
    nx = np.floor((lng-self.top_left[1]) / self.res_lng).astype(int)
    ny = np.floor((self.top_left[0]-lat) / self.res_lat).astype(int)
    idx = ny * self.n_bins_x + nx
    if(nx >= self.n_bins_x) or (ny >= self.n_bins_y) or (nx < 0) or (ny < 0):
      idx = -1
    return idx

  def get_bin_loc(self, bin_id):
    ny = bin_id / self.n_bins_x
    nx = bin_id % self.n_bins_x
    lat = self.top_left[0] - (ny+0.5)*self.res_lat
    lng = self.top_left[1] + (nx+0.5)*self.res_lng
    return lat, lng

  def calc_distance(self, lat0, lng0, lat1, lng1):
    _, _, d  = self.geo.inv(lng0, lat0, lng1, lat1)
    return d
 
if __name__ == '__main__':
  if len(sys.argv) != 4:
    print 'Usage: python {:s} city lat lng'.format(sys.argv[0])
    sys.exit(-1)

  city = sys.argv[1]
  lat = float(sys.argv[2])
  lng = float(sys.argv[3])
  b = Binner(city)
  idx = b.get_bin_id(lat, lng)
  print idx
