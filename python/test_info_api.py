import json
import sys
from url_utils import save_url
from IPython.core.debugger import Tracer

info_url_latlng = 'http://maps.google.com/cbk?output=json&ll=%f,%f'

if __name__ == '__main__':
  if len(sys.argv) is 3:
    lat = float(sys.argv[1])
    lng = float(sys.argv[2])
  else:
    lat = 33.746736
    lng = -84.373379

  done = save_url(info_url_latlng%(lat, lng), '/tmp/tmp.json')
  if done:
    with open('/tmp/tmp.json', 'r') as f:
      pano_info = json.load(f)
    print pano_info['Location']['panoId']
    for l in pano_info['Links']:
      print l['panoId'], l['yawDeg']
  else:
    sys.exit(-1)
