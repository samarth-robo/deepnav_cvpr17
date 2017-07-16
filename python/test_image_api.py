import os
import sys
import cv2
from url_utils import save_url
from IPython.core.debugger import Tracer

def test_image(pano_id=None, lat=None, lng=None):
  if lat is None:
    image_url = 'https://maps.googleapis.com/maps/api/streetview?pano=%s&size=640x480&heading=0&key='
    image_url %= pano_id
  else:
    image_url = 'https://maps.googleapis.com/maps/api/streetview?location=%f,%f&size=640x480&heading=0&key='
    image_url %= (lat, lng)
  api_key = os.environ['GOOGLE_KEY']
  image_url += api_key
  img_filename = '/tmp/tmp.jpg'
  done = save_url(image_url, img_filename)
  if done:
    im = cv2.imread('/tmp/tmp.jpg')
    cv2.imshow('.', im)
    cv2.waitKey(-1)
  else:
    sys.exit(-1)

if __name__ == '__main__':
  if len(sys.argv) == 2:
    pano_id = sys.argv[1]
    test_image(pano_id=pano_id)
  elif len(sys.argv) == 3:
    lat = float(sys.argv[1])
    lng = float(sys.argv[2])
    test_image(lat=lat, lng=lng)
  else:
    pano_id = '0w9j9ISHBayhOMjtJM8Ecw'
    test_image(pano_id=pano_id)
