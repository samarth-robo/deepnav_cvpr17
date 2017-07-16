from url_utils import get_json
from geo_utils import Binner
import os
import numpy as np
import sys
from IPython.core.debugger import Tracer

class Place:
  def __init__(self, dataset_name, full_name, bin_id, lat, lng):
    self.dataset_name = dataset_name
    self.full_name = full_name
    self.bin_id = bin_id
    self.lat = lat
    self.lng = lng

class PlaceFinder:
  def __init__(self, city):
    self.city = city
    self.base_dir = '../data/dataset/{:s}/'.format(self.city)
    self.bin_id = Binner(self.city)

    # create the URLs
    url = 'https://maps.googleapis.com/maps/api/place/nearbysearch/json?' +\
        'location={:f},{:f}&radius=10000&type={:s}&key='
    limits = np.loadtxt(self.base_dir + '/box.txt')
    top_left = limits[0]
    bottom_right = limits[1]
    start_lat = (top_left[0] + bottom_right[0])/2.0
    start_lng = (top_left[1] + bottom_right[1])/2.0
    url = url.format(start_lat, start_lng, '{:s}', '{:s}')
    api_key = os.environ['GOOGLE_KEY']
    self.place_url = url + api_key

    url = 'https://roads.googleapis.com/v1/nearestRoads?key='
    self.road_url = url + os.environ['GOOGLE_KEY1']

  def get_road_points(self, places):
    use_url = self.road_url + '&points='
    r_lat = []
    r_lng = []
    if len(places) >= 100:
      print 'WARN: places array has more than 100 members'
    for p in places[:100]:
      lat = float(p['geometry']['location']['lat'])
      r_lat.append(lat)
      lng = float(p['geometry']['location']['lng'])
      r_lng.append(lng)
      use_url += '{:.6f},{:.6f}|'.format(lat, lng)
    use_url = use_url[:-1]  # remove trailing |
    r, done = get_json(use_url, '/tmp/tmp_roads.json')
    if not done:
      print 'Could not get info from Google Roads'
      return None, None
    for p in r['snappedPoints']:
      idx = int(p['originalIndex'])
      r_lat[idx] = float(p['location']['latitude'])
      r_lng[idx] = float(p['location']['longitude'])
    return r_lat, r_lng

  def find_place(self, place_search_name, place_dataset_name, place_type):
    next_page_exists = True
    use_url = self.place_url.format(place_type)
    if place_search_name is not None:
      use_url += '&name={:s}'.format(place_search_name)
    places = []
    while next_page_exists:
      p, done = get_json(use_url, '/tmp/tmp_places.json')
      if not done:
        print 'Could not get info from Google Places'
        return None

      if len(p['results']) == 0:
        continue
      # snap to road
      r_lat, r_lng = self.get_road_points(p['results'])
      if (r_lat is None) or (r_lng is None):
        continue
      for i, pp in enumerate(p['results']):
        lat = float(pp['geometry']['location']['lat'])
        lng = float(pp['geometry']['location']['lng'])
        bin_id = self.bin_id.get_bin_id(r_lat[i], r_lng[i])
        if (bin_id > 0):
          full_name = pp['name'].encode('ascii', 'ignore')
          valid = False
          if place_search_name is not None:
            if place_search_name in full_name:
              valid = True
          else:
            valid = True
          if valid:
            places.append(Place(place_dataset_name, full_name, bin_id, lat,
              lng))
            print 'Found a {:s}'.format(full_name)
      
      if 'next_page_token' in p.keys():
        use_url = self.place_url.format(place_search_name, place_type) +\
            '&pagetoken={:s}'.format(p['next_page_token'])
      else:
        next_page_exists = False
    return places

  def save_places(self, places):
    with open('{:s}/dsts_script.txt'.format(self.base_dir), 'w') as f,\
        open('{:s}/{:s}_dsts_plot_data.txt'.format(self.base_dir, self.city),\
        'w') as f_plot:
      f_plot.write('#name,bin_id,lat,lng\n');
      for place in places:
        f.write('{:s} {:d}\n'.format(place.dataset_name, place.bin_id))
        f_plot.write('{:s},{:d},{:f},{:f}\n'.format(place.dataset_name,\
            place.bin_id, place.lat, place.lng))


if __name__ == '__main__':
  if len(sys.argv) != 2:
    print 'Usage: python {:s} city'.format(sys.argv[0])
    sys.exit(-1)

  place_search_names  = ['McDonald', 'Church', 'America', 'High', None]
  place_dataset_names = ['mcdonalds', 'church', 'bofa', 'high_school', 'gas_station']
  place_types         = ['restaurant', 'church', 'bank', 'school', 'gas_station']

  pf = PlaceFinder(sys.argv[1])

  places = []
  for place_search_name, place_dataset_name, place_type in\
      zip(place_search_names, place_dataset_names, place_types):
    p = pf.find_place(place_search_name, place_dataset_name, place_type)
    if p is not None:
      places.extend(p)
    else:
      print 'Could not get places for {:s}'.format(place_search_name)
  pf.save_places(places)
