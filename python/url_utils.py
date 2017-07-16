from urllib2 import Request, urlopen, URLError
import time
import json
from IPython.core.debugger import Tracer

def write_response(response, filename):
  with open(filename, 'wb') as f:
    while True:
      data = response.read(4096)
      if data:
        f.write(data)
      else:
        break

def save_url(url, filename):
  req = Request(url)

  success = False
  for _ in xrange(5):
    try:
      response = urlopen(req)
    except URLError as e:
      if hasattr(e, 'reason'):
        print 'We failed to reach a server.'
        print 'Reason: ', e.reason
      elif hasattr(e, 'code'):
        print 'The server couldn\'t fulfill the request.'
        print 'Error code: ', e.code
    except:
      print 'Unknown error'
    else:
      if response.code == 200:  # OK code
        write_response(response, filename)
        success = True
        break
      else:
        print 'Response code was not 200, re-trying'
    print 'Sleeping 1'
    time.sleep(1)
  return success

def get_json(url, filename=None):
  if filename is None:
    filename = '/tmp/tmp.json'
  success = False
  data = {}
  for _ in xrange(3):
    done = save_url(url, filename)
    if not done:
      time.sleep(1)
      continue
    with open(filename, 'r') as f:
      try:
        data = json.load(f)
      except:
        data = {}
    if len(data) != 0:
      success = True
      break
    else:
      time.sleep(1)
  return data, success
