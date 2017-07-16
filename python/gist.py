import cv2
import numpy as np
import imfeat

class GISTExtractor:
  def __init__(self, width=256, height=256):
    self.gist_func = imfeat.GIST(nblocks=4, orientations_per_scale=(8,8,8,8))
    self.width = width
    self.height = height

  def imresizecrop(self, im):
    im = np.mean(im, axis=2).astype(np.uint8)
    im = np.tile(im[:, :, np.newaxis], (1,1,3))
    scaling = max(float(self.width)/im.shape[1], float(self.height)/im.shape[0])
    newsize = (int(round(im.shape[1]*scaling)), int(round(im.shape[0]*scaling)))
    im = cv2.resize(im, newsize, interpolation=cv2.INTER_LINEAR)
    sr = int(np.floor((im.shape[0]-self.height)/2.0))
    sc = int(np.floor((im.shape[1]-self.width) /2.0))
    im = im[sr:sr+self.height, sc:sc+self.width, :]
    return im

  def extract_gist(self, im):
    if (im.shape[0] != self.height) or (im.shape[1] != self.width):
      im = self.imresizecrop(im)
    f = self.gist_func(im)
    return f[:f.shape[0]/3]

if __name__ == '__main__':
    im = cv2.imread('/home/samarth/Documents/MATLAB/GIST/demo2.jpg')
    g = GISTExtractor(width=256, height=256)
    f = g.extract_gist(im)
