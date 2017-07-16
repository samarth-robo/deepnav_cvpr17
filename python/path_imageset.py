import os.path as osp
import cv2
import sys

class PathImageset:
  def __init__(self, city, dst_name):
    self.dst_name = dst_name
    self.city = city
    self.base_dir = osp.join('../data/dataset/', city)
    
  def make_video(self, idx):
    path_fn = osp.join(self.base_dir, '{:d}_{:s}_pair_path.txt'.format(idx, self.dst_name))
    with open(path_fn, 'r') as f:
      lines = [l.rstrip() for l in f]

    fourcc = cv2.cv.CV_FOURCC(*'XVID')
    vid_fn = osp.join(self.base_dir, '{:d}_{:s}_pair_path.avi'.format(idx, self.dst_name))
    vid = cv2.VideoWriter(vid_fn, fourcc, 1.0, (640, 480))

    for line in lines[1:]:
      s = line.split(',')
      idx = int(s[0])
      im_idx = int(s[-1])
      im_fn = osp.join(self.base_dir, 'images', '{:>010d}_{:d}.jpg'.format(idx, im_idx))
      im = cv2.imread(im_fn)
      if im is not None:
        vid.write(im)
    vid.release()
    print vid_fn, 'written with {:d} frames'.format(len(lines)-1)

if __name__ == '__main__':
    if len(sys.argv) != 3:
      print 'Usage: {:s} city dest_name'.format(sys.argv[0])
      sys.exit(-1)

    p = PathImageset(sys.argv[1], sys.argv[2])
    p.make_video(166660)
