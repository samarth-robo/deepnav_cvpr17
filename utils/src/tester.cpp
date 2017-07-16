#include <common.h>
#include <tester.h>
#include <chrono>

using namespace std;
using namespace cv;

BaseTester::BaseTester(string city, string dst_name, string experiment,
                       int max_path_length, float dest_radius):
  city(city), dst_name(dst_name), experiment(experiment),
  graph_maker(GraphMaker(city)), max_path_length(max_path_length),
  net_idx(-1), dest_radius(dest_radius),
  geod(GeographicLib::Geodesic::WGS84()), searcher(new AStarSearcher()) {
    mean_filename  = "../data/mean_image.binaryproto";
    base_dir = string("../data/dataset/") + city + string("/");
    im_base_dir = base_dir + string("images/");

    // make graph
    bool done = graph_maker.construct_graph();
    if(!done) {
      cout << "Could not construct graph" << endl;
      exit(-1);
    }
    graph = graph_maker.get_graph();
    searcher->set_graph(graph);

    // read destination file
    string dst_filename = base_dir + string("/dsts_script.txt");
    ifstream dst_file(dst_filename.c_str());
    if(!dst_file.is_open()) {
      cout << "Could not open " << dst_filename << " for reading" << endl;
      return;
    }
    dsts.clear();
    set<string> dst_names;
    string name;
    size_t id;
    for(; dst_file >> name >> id;) {
      dst_names.insert(name);
      if(dst_name.compare(name) != 0) continue;
      Node::Ptr n = graph->get_valid_node(id);
      if(!n) {
        cout << "Destination " << id << " is not valid for " << dst_name << endl;
      } else {
        dsts.push_back(n);
      }
    }
    dst_file.close();
    cout << "Destinations for " << dst_name << ": ";
    for(auto const &dst : dsts) cout << dst << " ";
    cout << endl;

    // get idx in CNN output for the destination
    int i = 0;
    for(auto it = dst_names.cbegin(); it != dst_names.cend(); it++, i++) {
      if(dst_name.compare(*it) == 0) {
        net_idx = i;
      }
    }
    if(net_idx < 0) {
      cout << "Could not find correct CNN output idx" << endl;
      exit(-1);
    }
}

bool BaseTester::is_node_in_path(Node::Ptr const &n) {
  bool found = false;
  for(int i = 0; i < path.size(); i++) {
    if(n->equald(path[i])) {
      found = true;
      break;
    }
  }
  return found;
}

Node::Ptr BaseTester::backtrack(Node::Ptr const &r, float current_dist_est) {
  if(!r) return Node::Ptr();

  // find node to backtrack to
  Searcher::Ptr s(new AStarSearcher(graph));
  float best_option_cost = +FLT_MAX;
  Node::Ptr best_option = Node::Ptr();
  for(auto const &op : open_options) {
    bool path_found = s->search(r, op);
    if(!path_found) continue;
    float option_cost = s->get_cost(r, op);
    if(option_cost < best_option_cost) {
      best_option_cost = option_cost;
      best_option = op;
    }
  }
  return best_option;
  /*
  if(best_option_cost < current_dist_est) {
    return best_option;
  } else {
    return Node::Ptr();
  }
  */
}

size_t BaseTester::get_path_length() {
  size_t len = 0;
  for(int i = 0; i < path.size()-1; i++) {
    if(!path[i]->equal(path[i+1])) len++;
  }
  return len;
}

string BaseTester::get_im_name(Node::Ptr const &n) {
  stringstream ss;
  ss << im_base_dir
     << setw(10) << setfill('0') << n->id << "_"
     << n->im_idx << ".jpg";
  string im_filename = ss.str();
  return im_filename;
}

// Function to read image from the dataset
Mat BaseTester::get_im(Node::Ptr const &n) {
  string im_filename = get_im_name(n);
  Mat im = cv::imread(im_filename);
  if(im.empty()) {
    cout << "could not read " << im_filename << endl;
    return Mat();
  }
  return im;
}

// function to test completion of search
bool BaseTester::near_dst(Node::Ptr const &n) {
  bool found = false;
  for(auto const &d : dsts) {
    double distance;
    geod.Inverse(n->lat, n->lng, d->lat, d->lng, distance);
    if(distance < dest_radius) {
      found = true;
      break;
    }
  }
  return found;
}

bool compare_pairs(pair<float, int> const &a, pair<float, int> const &b) {
  return a.first < b.first;
}

bool BaseTester::gather_astar_stats(size_t start) {
  Node::Ptr start_node = graph->get_valid_node(start);
  if(!start_node) {
    cout << "Start node " << start << " is not valid" << endl;
    return false;
  }
  shortest_path_cost = +FLT_MAX;
  shortest_path_length = +INT_MAX;
  for(auto const &dst : dsts) {
    if(searcher->search(start_node, dst)) {
      float cost = searcher->get_cost(start_node, dst);
      if(cost < shortest_path_cost) {
        shortest_path_cost = cost;
        shortest_path_length = searcher->get_path_length();
      }
    }
  }
  return true;
}

DistanceTester::DistanceTester(string city, string dst_name) :
  mean(-1.f), stdev(-1.f), BaseTester(city, dst_name, "distance") {
    proto_filename = "../ml/distance/deploy.prototxt";
    stringstream ss;
    ss << string("../output/streetview/") << "combine"
       << "_vgg_distance_iter_352000.caffemodel";
    model_filename = ss.str();

    caffe = new CaffeWrapper(proto_filename, model_filename, mean_filename);
    cout << "Finished caffe initialization" << endl;

    string stats_filename = base_dir + string("/distance/label_stats.txt");
    ifstream stats_file(stats_filename);
    if(!stats_file.is_open()) {
      cout << "Could not open " << stats_filename << endl;
      exit(-1);
    }
    string dest;
    float m, s;
    for(; stats_file >> dest >> m >> s;) {
      if(dst_name.compare(dest) != 0) continue;
      mean = m;
      stdev = s;
      break;
    }
    if(mean < 0.f || stdev < 0.f) {
      cout << "Label stats mean = " << mean << " std = " << stdev
           << " are wrong" << endl;
      exit(-1);
    }
}

bool DistanceTester::find(size_t start) {
  Node::Ptr start_node = graph->get_valid_node(start);
  if(!start_node) {
    cout << "Start node " << start << " is not valid" << endl;
    return false;
  }

  // current robot location
  Node::Ptr r = start_node;
  float r_dist_est = +FLT_MAX;

  // recoded path
  path.clear();
  open_options.clear();
  path.push_back(r);
  unordered_map<size_t, unordered_set<Node::Ptr, NodeHash,
                                      NodeEqual> > prev_actions;

  bool found = false;
  while(path.size() < max_path_length) {
    if(near_dst(r)) {
      found = true;
      break;
    }
    cout << city << " : " << dst_name << " : distance " << " robot location = " << r << endl;

    // determine open directions
    vector<Node::Ptr> dirs = graph->get_nodes_at_loc(r->id);

    // get distance estimates for directions
    vector<pair<float, int> > dist_est;
    for(int i = 0; i < dirs.size(); i++) {
      if(dirs[i]->im_idx < 0) continue;
      Mat im = get_im(dirs[i]);
      vector<float> pred = caffe->get_pred(im);
      dist_est.push_back(make_pair(pred[net_idx]*stdev + mean, i));
      cout << "Dir " << dirs[i] << " distance est = " << dist_est.back().first << endl;
    }
    cout << endl;

    // sort the directions
    sort(dist_est.begin(), dist_est.end(), compare_pairs);
     
    // pick the next robot location
    bool moved = false;
    Node::Ptr next_r = Node::Ptr();
    float next_r_dist_est = 0;
    for(int i = 0; i < dist_est.size(); i++) {
      Node::Ptr nbr = graph->get_nbrd(r, dirs[dist_est[i].second]->d).first;
      if(!nbr || prev_actions[r->id].count(nbr)) continue;
      if(!moved) {
        prev_actions[r->id].insert(nbr);
        next_r = nbr;
        next_r_dist_est = dist_est[i].first;
        moved = true;
      } else {
        open_options.insert(nbr);
      }
    }
    if(moved) {
      r = next_r;
      r_dist_est = next_r_dist_est;
    } else {
      open_options.erase(r);
      cout << "No option to move, backtracking..." << endl;
      r = backtrack(r, r_dist_est);
      if(!r) {
        cout << "No valid backtracking options" << endl;
        return false;
      }
    }
    path.push_back(r);
  }
  /*
  stringstream ss;
  ss << base_dir << start << "_" << dst_name << "_" << experiment
     << "_path.txt";
  bool path_written = write_path(path, ss.str());
  if(path_written) cout << ss.str() << " written" << endl;
  */

  return found;
}

bool DistanceTester::calc_entropies() {
  unordered_map<size_t, float> entropies;
  cout << city << " : " << dst_name << " : " << experiment << endl;
  for(auto node_it = graph->cbegin(); node_it != graph->cend(); node_it++) {
    size_t id = node_it->second->id;
    if(entropies.count(id)) continue;
    vector<Node::Ptr> const &nodes = graph->get_nodes_at_loc(id);

    // get preds
    vector<float> preds;
    float pred_sum = 0.f, pred_sq_sum = 0.f;
    for(auto const &node : nodes) {
      if(node->im_idx < 0) continue;
      Mat im = get_im(node);
      vector<float> pred = caffe->get_pred(im);
      preds.push_back(pred[net_idx]);
      pred_sum    += preds.back();
      pred_sq_sum += preds.back() * preds.back();
    }
    // calc stdev
    if(preds.size() == 0) continue;
    float mean = pred_sum / preds.size();
    entropies[id] = sqrt(pred_sq_sum/preds.size() - mean*mean);
  }
  string entropy_filename = base_dir + dst_name +
      string("_") + experiment + string("_entropy.txt");
  ofstream entropy_file(entropy_filename);
  if(!entropy_file.is_open()) {
    cout << "Could not open " << entropy_filename << " for writing" << endl;
    return false;
  }
  entropy_file << "#lat,lng,entropy" << endl;
  for(auto const &e : entropies) {
    Node::Ptr n = graph->get_valid_node(e.first);
    if(!n) continue;
    entropy_file << setprecision(7) << n->lat << ","
                 << n->lng << "," << entropies.at(n->id) << endl;
  }
  entropy_file.close();
  cout << entropy_filename << " written" << endl;
  return true;
}

DistanceGISTTester::DistanceGISTTester(string city, string dst_name) :
  mean(-1.f), stdev(-1.f), sklearn_wrapper(city, dst_name),
  BaseTester(city, dst_name, "distance_gist") {
  string stats_filename = base_dir + string("/distance/label_stats.txt");
  ifstream stats_file(stats_filename);
  if(!stats_file.is_open()) {
    cout << "Could not open " << stats_filename << endl;
    exit(-1);
  }
  string dest;
  float m, s;
  for(; stats_file >> dest >> m >> s;) {
    if(dst_name.compare(dest) != 0) continue;
    mean = m;
    stdev = s;
    break;
  }
  if(mean < 0.f || stdev < 0.f) {
    cout << "Label stats mean = " << mean << " std = " << stdev
         << " are wrong" << endl;
    exit(-1);
  }
}

bool DistanceGISTTester::find(size_t start) {
  Node::Ptr start_node = graph->get_valid_node(start);
  if(!start_node) {
    cout << "Start node " << start << " is not valid" << endl;
    return false;
  }

  // current robot location
  Node::Ptr r = start_node;
  float r_dist_est = +FLT_MAX;

  // recoded path
  path.clear();
  open_options.clear();
  path.push_back(r);
  unordered_map<size_t, unordered_set<Node::Ptr, NodeHash,
                                      NodeEqual> > prev_actions;

  bool found = false;
  while(path.size() < max_path_length) {
    if(near_dst(r)) {
      found = true;
      break;
    }
    cout << city << " : " << dst_name << " : gist " << " robot location = " << r << endl;

    // determine open directions
    vector<Node::Ptr> dirs = graph->get_nodes_at_loc(r->id);

    // get distance estimates for directions
    vector<pair<float, int> > dist_est;
    for(int i = 0; i < dirs.size(); i++) {
      if(dirs[i]->im_idx < 0) continue;
      string im_filename = get_im_name(dirs[i]);
      float pred = sklearn_wrapper.get_pred(im_filename);
      dist_est.push_back(make_pair(pred*stdev + mean, i));
      cout << "Dir " << dirs[i] << " distance est = " << dist_est.back().first << endl;
    }
    cout << endl;

    // sort the directions
    sort(dist_est.begin(), dist_est.end(), compare_pairs);

    // pick the next robot location
    bool moved = false;
    Node::Ptr next_r = Node::Ptr();
    float next_r_dist_est = 0;
    for(int i = 0; i < dist_est.size(); i++) {
      Node::Ptr nbr = graph->get_nbrd(r, dirs[dist_est[i].second]->d).first;
      if(!nbr || prev_actions[r->id].count(nbr)) continue;
      if(!moved) {
        prev_actions[r->id].insert(nbr);
        next_r = nbr;
        next_r_dist_est = dist_est[i].first;
        moved = true;
      } else {
        open_options.insert(nbr);
      }
    }
    if(moved) {
      r = next_r;
      r_dist_est = next_r_dist_est;
    } else {
      open_options.erase(r);
      cout << "No option to move, backtracking..." << endl;
      r = backtrack(r, r_dist_est);
      if(!r) {
        cout << "No valid backtracking options" << endl;
        return false;
      }
    }
    path.push_back(r);
  }
  /*
  stringstream ss;
  ss << base_dir << start << "_" << dst_name << "_" << experiment
     << "_path.txt";
  bool path_written = write_path(path, ss.str());
  if(path_written) cout << ss.str() << " written" << endl;
  */

  return found;
}

bool DistanceGISTTester::calc_entropies() {
  unordered_map<size_t, float> entropies;
  cout << city << " : " << dst_name << " : " << experiment << endl;
  for(auto node_it = graph->cbegin(); node_it != graph->cend(); node_it++) {
    size_t id = node_it->second->id;
    if(entropies.count(id)) continue;
    vector<Node::Ptr> const &nodes = graph->get_nodes_at_loc(id);

    // get preds
    vector<float> preds;
    float pred_sum = 0.f, pred_sq_sum = 0.f;
    for(auto const &node : nodes) {
      if(node->im_idx < 0) continue;
      string im_filename = get_im_name(node);
      float pred = sklearn_wrapper.get_pred(im_filename);
      preds.push_back(pred);
      pred_sum    += preds.back();
      pred_sq_sum += preds.back() * preds.back();
    }
    // calc stdev
    if(preds.size() == 0) continue;
    float mean = pred_sum / preds.size();
    entropies[id] = sqrt(pred_sq_sum/preds.size() - mean*mean);
  }
  string entropy_filename = base_dir + dst_name +
      string("_") + experiment + string("_entropy.txt");
  ofstream entropy_file(entropy_filename);
  if(!entropy_file.is_open()) {
    cout << "Could not open " << entropy_filename << " for writing" << endl;
    return false;
  }
  entropy_file << "#lat,lng,entropy" << endl;
  for(auto const &e : entropies) {
    Node::Ptr n = graph->get_valid_node(e.first);
    if(!n) continue;
    entropy_file << setprecision(7) << n->lat << ","
                 << n->lng << "," << entropies.at(n->id) << endl;
  }
  entropy_file.close();
  cout << entropy_filename << " written." << endl;
  return true;
}

DirectionTester::DirectionTester(string city, string dst_name) :
  BaseTester(city, dst_name, "direction") {
    proto_filename = "../ml/direction/deploy.prototxt";
    stringstream ss;
    ss << string("../output/streetview/") << "combine"
       << "_vgg_lw_direction_iter_384000.caffemodel";
    model_filename = ss.str();

    caffe = new CaffeWrapper(proto_filename, model_filename, mean_filename);
    cout << "Finished caffe initialization" << endl;
}

vector<pair<float, int> > DirectionTester::get_net_out(Node::Ptr const &n) {
  Mat im = get_im(n);
  vector<float> net_out = caffe->get_pred(im);
  size_t n_dst_kinds = net_out.size() / 4;
  vector<pair<float, int> > pred(Direction::N_DIRS);
  cout << "Scores (fwd, right, left, bwd) = ";
  for (int i = 0; i < pred.size(); i++) {
    pred[i] = make_pair(net_out[i * n_dst_kinds + net_idx], i);
    cout << pred[i].first << " ";
  }
  cout << endl;
  return pred;
}

bool DirectionTester::find(size_t start) {
  Node::Ptr start_node = graph->get_valid_node(start);
  if(!start_node) {
    cout << "Start node " << start << " is not valid" << endl;
    return false;
  }

  // current robot location
  Node::Ptr r = start_node;
  float r_pred = 0.f;

  // recoded path
  path.clear();
  open_options.clear();
  path.push_back(r);
  unordered_map<size_t, unordered_set<Node::Ptr, NodeHash,
                                      NodeEqual> > prev_actions;

  bool found = false;
  while(path.size() < max_path_length) {
    if(near_dst(r)) {
      found = true;
      break;
    }
    cout << city << " : " << dst_name << " : direction " << " robot location = " << r << endl;

    if(r->im_idx < 0) {
      cout << r << " -> im_idx = " << r->im_idx << endl;
      vector<Node::Ptr> ns = graph->get_nodes_at_loc(r->id);
      vector<pair<float, int> > nps;
      for(int i = 0; i < ns.size(); i++) {
        if(ns[i]->im_idx < 0) continue;
        vector<pair<float, int> > pred = get_net_out(ns[i]);
        nps.push_back(make_pair(pred[0].first, i));
      }
      sort(nps.rbegin(), nps.rend(), compare_pairs);
      Node::Ptr r_turn = Node::Ptr();
      for(auto const &np : nps) {
        if(!prev_actions[r->id].count(ns[np.second])) {
          prev_actions[r->id].insert(ns[np.second]);
          r_turn = ns[np.second];
          break;
        }
      }
      r = r_turn;
      if(r) cout << "Turned to " << r << endl;
      else {
        cout << "No option to turn to" << endl;
        break;
      }
    }
    vector<pair<float, int> > pred = get_net_out(r);
    sort(pred.rbegin(), pred.rend(), compare_pairs);

    // pick the next robot location
    bool moved = false;
    Node::Ptr next_r = Node::Ptr();
    float next_r_pred = 0.f;
    for(int i = 0; i < pred.size(); i++) {
      int dir = pred[i].second;
      Node::Ptr nbr;
      switch(dir) {
        case 0:  nbr = graph->get_nbrd(r,
                     Direction::Enum(r->d)).first;
                 break;
        case 1:  nbr = graph->get_nbrd(r,
                     Direction::Enum((r->d + 1) % 4)).first;
                 break;
        case 2:  nbr = graph->get_nbrd(r,
                     Direction::Enum((r->d + 3) % 4)).first;
                 break;
        case 3:  nbr = graph->get_nbrd(r,
                     Direction::Enum((r->d + 2) % 4)).first;
                 break;
        default: cout << "Wrong prediction " << dir << endl;
                 return false;
      }
      if(!nbr || prev_actions[r->id].count(nbr)) continue;
      if(!moved) {
        prev_actions[r->id].insert(nbr);
        next_r = nbr;
        next_r_pred = pred[i].first;
        moved = true;
      } else {
        open_options.insert(nbr);
      }
    }
    if(moved) {
      r = next_r;
      r_pred = next_r_pred;
    } else {
      open_options.erase(r);
      cout << "No option to move, backtracking..." << endl;
      r = backtrack(r, r_pred);
      if(!r) {
        cout << "No valid backtracking options" << endl;
        break;
      }
    }
    path.push_back(r);
  }
  /*
  stringstream ss;
  ss << base_dir << start << "_" << dst_name << "_" << experiment
     << "_path.txt";
  bool path_written = write_path(path, ss.str());
  if(path_written) cout << ss.str() << " written" << endl;
  */

  return found;
}

bool DirectionTester::calc_entropies() {
  unordered_map<size_t, float> entropies;
  cout << city << " : " << dst_name << " : " << experiment << endl;
  for(auto node_it = graph->cbegin(); node_it != graph->cend(); node_it++) {
    size_t id = node_it->second->id;
    if(entropies.count(id)) continue;
    vector<Node::Ptr> const &nodes = graph->get_nodes_at_loc(id);

    // get preds
    vector<float> preds;
    float pred_sum = 0.f, pred_sq_sum = 0.f;
    for(auto const &node : nodes) {
      if(node->im_idx < 0) continue;
      vector<pair<float, int> > pred = get_net_out(node);
      preds.push_back(pred[0].first);
      pred_sum    += preds.back();
      pred_sq_sum += preds.back() * preds.back();
    }
    // calc stdev
    if(preds.size() == 0) continue;
    float mean = pred_sum / preds.size();
    entropies[id] = sqrt(pred_sq_sum/preds.size() - mean*mean);
  }
  string entropy_filename = base_dir + dst_name +
      string("_") + experiment + string("_entropy.txt");
  ofstream entropy_file(entropy_filename);
  if(!entropy_file.is_open()) {
    cout << "Could not open " << entropy_filename << " for writing" << endl;
    return false;
  }
  entropy_file << "#lat,lng,entropy" << endl;
  for(auto const &e : entropies) {
    Node::Ptr n = graph->get_valid_node(e.first);
    if(!n) continue;
    entropy_file << setprecision(7) << n->lat << ","
                 << n->lng << "," << entropies.at(n->id) << endl;
  }
  entropy_file.close();
  cout << entropy_filename << " written" << endl;
  return true;
}

PairTester::PairTester(string city, string dst_name) :
  BaseTester(city, dst_name, "pair") {
    proto_filename = "../ml/pair/deploy.prototxt";
    stringstream ss;
    ss << string("../output/streetview/") << "combine"
      << "_vgg_pair_iter_155000.caffemodel";
    model_filename = ss.str();

    caffe = new CaffeWrapper(proto_filename, model_filename, mean_filename);
    cout << "Finished caffe initialization" << endl;
}

bool PairTester::find(size_t start) {
  Node::Ptr start_node = graph->get_valid_node(start);
  if(!start_node) {
    cout << "Start node " << start << " is not valid" << endl;
    return false;
  }

  // current robot location
  Node::Ptr r = start_node;
  float r_dist_est = +FLT_MAX;

  // recoded path
  path.clear();
  open_options.clear();
  path.push_back(r);
  unordered_map<size_t, unordered_set<Node::Ptr, NodeHash,
                                      NodeEqual> > prev_actions;

  bool found = false;
  while(path.size() < max_path_length) {
    if(near_dst(r)) {
      found = true;
      break;
    }
    cout << city << " : " << dst_name << " : pair " << " robot location = " << r << endl;

    // determine open directions
    vector<Node::Ptr> dirs = graph->get_nodes_at_loc(r->id);

    // get scores for directions
    vector<pair<float, int> > dist_est;
    for(int i = 0; i < dirs.size(); i++) {
      if(dirs[i]->im_idx < 0) continue;
      Mat im = get_im(dirs[i]);
      vector<float> pred = caffe->get_pred(im);
      dist_est.push_back(make_pair(pred[net_idx], i));
      cout << "Dir " << dirs[i] << " score = " << dist_est.back().first << endl;
    }
    cout << endl;

    // sort the directions
    sort(dist_est.rbegin(), dist_est.rend(), compare_pairs);

    // pick the next robot location
    bool moved = false;
    Node::Ptr next_r = Node::Ptr();
    float next_r_dist_est = 0.f;
    for(int i = 0; i < dist_est.size(); i++) {
      Node::Ptr nbr = graph->get_nbrd(r, dirs[dist_est[i].second]->d).first;
      if(!nbr || prev_actions[r->id].count(nbr)) continue;
      if(!moved) {
        prev_actions[r->id].insert(nbr);
        next_r = nbr;
        next_r_dist_est = dist_est[i].first;
        moved = true;
      } else {
        open_options.insert(nbr);
      }
    }
    if(moved) {
      r = next_r;
      r_dist_est = next_r_dist_est;
    } else {
      open_options.erase(r);
      cout << "No option to move, backtracking..." << endl;
      r = backtrack(r, r_dist_est);
      if(!r) {
        cout << "No valid backtracking options" << endl;
        break;
      }
    }
    path.push_back(r);
  }
  stringstream ss;
  ss << base_dir << start << "_" << dst_name << "_" << experiment
     << "_path.txt";
  bool path_written = write_path(path, ss.str());
  if(path_written) cout << ss.str() << " written" << endl;

  return found;
}

bool PairTester::calc_entropies() {
  unordered_map<size_t, float> entropies;
  cout << city << " : " << dst_name << " : " << experiment << endl;
  for(auto node_it = graph->cbegin(); node_it != graph->cend(); node_it++) {
    size_t id = node_it->second->id;
    if(entropies.count(id)) continue;
    vector<Node::Ptr> const &nodes = graph->get_nodes_at_loc(id);

    // get preds
    vector<float> preds;
    float pred_sum = 0.f, pred_sq_sum = 0.f;
    for(auto const &node : nodes) {
      if(node->im_idx < 0) continue;
      Mat im = get_im(node);
      vector<float> pred = caffe->get_pred(im);
      preds.push_back(pred[net_idx]);
      pred_sum    += preds.back();
      pred_sq_sum += preds.back() * preds.back();
    }
    // calc stdev
    if(preds.size() == 0) continue;
    float mean = pred_sum / preds.size();
    entropies[id] = sqrt(pred_sq_sum/preds.size() - mean*mean);
  }
  string entropy_filename = base_dir + dst_name +
      string("_") + experiment + string("_entropy.txt");
  ofstream entropy_file(entropy_filename);
  if(!entropy_file.is_open()) {
    cout << "Could not open " << entropy_filename << " for writing" << endl;
    return false;
  }
  entropy_file << "#lat,lng,entropy" << endl;
  for(auto const &e : entropies) {
    Node::Ptr n = graph->get_valid_node(e.first);
    if(!n) continue;
    entropy_file << setprecision(7) << n->lat << ","
                 << n->lng << "," << entropies.at(n->id) << endl;
  }
  entropy_file.close();
  cout << entropy_filename << " written" << endl;
  return true;
}

bool PairTester::sort_nodes() {
  std::priority_queue<std::pair<Node::Ptr, float>,
                      std::vector<std::pair<Node::Ptr, float> >, PairCompare> q;
  cout << city << " : " << dst_name << " : " << experiment << endl;
  int count = 0;
  for(auto node_it = graph->cbegin(); node_it != graph->cend(); node_it++) {
    if(count % 100 == 0) cout << "Node " << count << " / " << graph->get_n_nodes() << endl;
    count++;
    if (node_it->second->im_idx < 0) continue;

    Mat im = get_im(node_it->second);
    vector<float> pred = caffe->get_pred(im);
    q.push(make_pair(node_it->second, -(pred[net_idx])));  // PairCompare is >
  }
  string nodes_filename = base_dir + dst_name +
      string("_") + experiment + string("_node_preds.txt");
  ofstream nodes_file(nodes_filename);
  if(!nodes_file.is_open()) {
    cout << "Could not open " << nodes_filename << " for writing" << endl;
    return false;
  }
  while(!q.empty()) {
    pair<Node::Ptr, float> p = q.top();
    q.pop();
    nodes_file << p.first << " " << p.second << endl;
  }
  nodes_file.close();
  cout << nodes_filename << " written" << endl;
  return true;
}

bool RandomWalkTester::find(size_t start) {
  Node::Ptr start_node = graph->get_valid_node(start);
  if(!start_node) {
    cout << "Start node " << start << " is not valid" << endl;
    return false;
  }
  
  // current robot location
  Node::Ptr r = start_node;
  float r_dist_est = +FLT_MAX;

  // recoded path
  path.clear();
  open_options.clear();
  path.push_back(r);
  unordered_map<size_t, unordered_set<Node::Ptr, NodeHash,
                                      NodeEqual> > prev_actions;

  bool found = false;
  unsigned seed = chrono::system_clock::now().time_since_epoch().count();
  default_random_engine generator(seed);
  uniform_int_distribution<int> distribution(0, 99);
  while(path.size() < max_path_length) {
    if(near_dst(r)) {
      found = true;
      break;
    }
    cout << city << " : " << dst_name << " : random " << " robot location = " << r << endl;

    // determine open directions
    vector<Node::Ptr> dirs = graph->get_nodes_at_loc(r->id);

    // get distance estimates for directions (random numbers)
    vector<pair<float, int> > dist_est;
    for(int i = 0; i < dirs.size(); i++) {
      float rand_num = static_cast<float>(distribution(generator));
      dist_est.push_back(make_pair(rand_num, i));
      cout << "Dir " << dirs[i] << " score = " << dist_est.back().first << endl;
    }
    cout << endl;

    // sort the directions
    sort(dist_est.rbegin(), dist_est.rend(), compare_pairs);
     
    // pick the next robot location
    bool moved = false;
    Node::Ptr next_r = Node::Ptr();
    float next_r_dist_est = 0.f;
    for(int i = 0; i < dist_est.size(); i++) {
      Node::Ptr nbr = graph->get_nbrd(r, dirs[dist_est[i].second]->d).first;
      // if(!nbr || prev_actions[r->id].count(nbr)) continue;
      if(!nbr) continue;
      if(!moved) {
        // prev_actions[r->id].insert(nbr);
        next_r = nbr;
        next_r_dist_est = dist_est[i].first;
        moved = true;
      } else {
        open_options.insert(nbr);
      }
    }
    if(moved) {
      r = next_r;
      r_dist_est = next_r_dist_est;
    } else {
      open_options.erase(r);
      cout << "No option to move, backtracking..." << endl;
      r = backtrack(r, r_dist_est);
      if(!r) {
        cout << "No valid backtracking options" << endl;
        return false;
      }
    }
    path.push_back(r);
  }
  /*
  stringstream ss;
  ss << base_dir << start << "_" << dst_name << "_" << experiment
     << "_path.txt";
  bool path_written = write_path(path, ss.str());
  if(path_written) cout << ss.str() << " written" << endl;
  */

  return found;
}

bool RandomWalkTester::calc_entropies() {
  unsigned seed = chrono::system_clock::now().time_since_epoch().count();
  default_random_engine generator(seed);
  uniform_int_distribution<int> distribution(0, 99);
  unordered_map<size_t, float> entropies;
  cout << city << " : " << dst_name << " : " << experiment << endl;
  for(auto node_it = graph->cbegin(); node_it != graph->cend(); node_it++) {
    size_t id = node_it->second->id;
    if(entropies.count(id)) continue;
    cout << "Node location : " << id << endl;
    vector<Node::Ptr> const &nodes = graph->get_nodes_at_loc(id);

    // get preds
    vector<float> preds;
    float pred_sum = 0.f, pred_sq_sum = 0.f;
    for(auto const &node : nodes) {
      float pred = static_cast<float>(distribution(generator));
      preds.push_back(pred);
      pred_sum    += preds.back();
      pred_sq_sum += preds.back() * preds.back();
    }
    // calc stdev
    if(preds.size() == 0) continue;
    float mean = pred_sum / preds.size();
    entropies[id] = sqrt(pred_sq_sum/preds.size() - mean*mean);
  }
  string entropy_filename = base_dir + dst_name + string("_") + experiment +
      string("_entropy.txt");
  ofstream entropy_file(entropy_filename);
  if(!entropy_file.is_open()) {
    cout << "Could not open " << entropy_filename << " for writing" << endl;
    return false;
  }
  entropy_file << "lat,lng,entropy" << endl;
  for(auto const &e : entropies) {
    Node::Ptr n = graph->get_valid_node(e.first);
    if(!n) continue;
    entropy_file << setprecision(7) << n->lat << ","
                 << n->lng << "," << entropies.at(n->id) << endl;
  }
  entropy_file.close();
  cout << entropy_filename << " written" << endl;
  return true;
}
