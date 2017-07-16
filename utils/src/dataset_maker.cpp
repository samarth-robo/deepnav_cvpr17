#include <common.h>
#include <dataset_maker.h>
#include <hdf5_wrapper.h>

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

BaseDatasetMaker::BaseDatasetMaker(string city, string experiment,
                                   float discount_factor) :
  city(city), experiment(experiment), graph_maker(GraphMaker(city)),
  astar_searcher(new AStarSearcher()), ex_searcher(new ExhaustiveSearcher()),
  discount_factor(discount_factor) {
  base_dir = string("../data/dataset/") + city + string("/");
  exp_dir  = base_dir + experiment + string("/");
  
  // make graph
  bool done = graph_maker.construct_graph();
  if(!done) {
    cout << "Could not construct graph" << endl;
    exit(-1);
  }
  graph = graph_maker.get_graph();

  // init searcher objects
  astar_searcher->set_graph(graph);
  ex_searcher->set_graph(graph);

  // read destination file
  all_dsts.clear();
  string dst_filename;
  if(experiment == string("distance"))
    dst_filename = base_dir + city + string("_dsts_plot_data.txt");
  else
    dst_filename = base_dir + string("dsts_script.txt");
  ifstream dst_file(dst_filename.c_str());
  if(!dst_file.is_open()) {
    cout << "Could not open " << dst_filename << " for reading" << endl;
    return;
  }

  if(experiment == string("distance")) {
    string dummy_s;
    getline(dst_file, dummy_s);  // sink first line in file

    string dst_name;
    size_t id = 0;
    float lat = 0.f, lng = 0.f;
    char dummy = 's';
    while (getline(dst_file, dst_name, ',')) {
      dst_file >> id >> dummy >> lat >> dummy >> lng;
      dst_file.get();
      // we don't care about validity of dest node for distance experiment
      Node::Ptr n(new Node(id, Direction::N, lat, lng));
      all_dsts[dst_name].push_back(n);
    }
  } else {
    string dst_name;
    size_t id = 0;
    for(; dst_file >> dst_name >> id;) {
      Node::Ptr n = graph->get_valid_node(id);
      if (n) {
        all_dsts[dst_name].push_back(n);
      } else {
        cout << "Destination " << id << " is not valid for " << dst_name << endl;
      }
    }
  }
  dst_file.close();
  for(auto const &dst : all_dsts) {
    cout << "Destinations for " << dst.first << ": ";
    for(int i = 0; i < all_dsts[dst.first].size(); i++)
      cout << all_dsts[dst.first][i] << " ";
    cout << endl;
  }

  // read separating line
  string line_filename = base_dir + string("/box.txt");
  ifstream line_file(line_filename.c_str());
  if(!line_file.is_open()) {
    cout << "Could not open " << line_filename << " for reading" << endl;
    exit(-1);
  }
  line_file >> line_lat1 >> line_lng1;
  line_file >> line_lat2 >> line_lng2;
  line_file.close();
  // get line slope and intercept
  line_slope = (line_lat1 - line_lat2) / (line_lng1 - line_lng2);
  line_intercept = (line_lat2*line_lng1 - line_lat1*line_lng2) /
    (line_lng1 - line_lng2);
  cout << "Slope = " << line_slope << ", intercept = " << line_intercept << endl;
}

bool BaseDatasetMaker::right_of_line(float lat, float lng) {
  double r = line_slope*lng + line_intercept - lat;
  bool right = (r > 0);
  return right;
}

bool BaseDatasetMaker::make_dataset() {
  // generate some filenames
  string train_label_txt_filename  = exp_dir + string("/train_labels.txt");
  string train_label_hdf5_filename = exp_dir + string("/train_labels.h5");
  string train_im_list_filename    = exp_dir + string("/train_im_list.txt");
  string test_label_txt_filename   = exp_dir + string("/test_labels.txt");
  string test_label_hdf5_filename  = exp_dir + string("/test_labels.h5");
  string test_im_list_filename     = exp_dir + string("/test_im_list.txt");
  string train_label_list_filename = exp_dir + string("/train_label_list.txt");
  string test_label_list_filename  = exp_dir + string("/test_label_list.txt");
  string train_lw_hdf5_filename    = exp_dir + string("/train_loss_weights.h5");
  string test_lw_hdf5_filename     = exp_dir + string("/test_loss_weights.h5");

  // open output files
  ofstream train_im_list_file(train_im_list_filename.c_str()),
           train_label_txt_file(train_label_txt_filename.c_str()),
           train_label_list_file(train_label_list_filename.c_str()),
           test_im_list_file(test_im_list_filename.c_str()),
           test_label_txt_file(test_label_txt_filename.c_str()),
           test_label_list_file(test_label_list_filename.c_str());
  if(!train_im_list_file.is_open()) {
    cout << "Could not open " << train_im_list_filename << endl;
    return false;
  }
  if(!train_label_txt_file.is_open()) {
    cout << "Could not open " << train_label_txt_filename << endl;
    return false;
  }
  if(!train_label_list_file.is_open()) {
    cout << "Could not open " << train_label_list_filename << endl;
    return false;
  }
  if(!test_im_list_file.is_open()) {
    cout << "Could not open " << test_im_list_filename << endl;
    return false;
  }
  if(!test_label_txt_file.is_open()) {
    cout << "Could not open " << test_label_txt_filename << endl;
    return false;
  }
  if(!test_label_list_file.is_open()) {
    cout << "Could not open " << test_label_list_filename << endl;
    return false;
  }
  HDF5Wrapper *train_label_hdf5_file = new HDF5Wrapper(train_label_hdf5_filename,
                                                       string("label"), 2),
              *test_label_hdf5_file  = new HDF5Wrapper(test_label_hdf5_filename,
                                                       string("label"), 2),
              *train_lw_hdf5_file    = new HDF5Wrapper(train_lw_hdf5_filename,
                                                       string("loss_weight"), 2),
              *test_lw_hdf5_file     = new HDF5Wrapper(test_lw_hdf5_filename,
                                                       string("loss_weight"), 2);

  // write data
  if(experiment == string("distance") || experiment == string("direction")) {
    // train
    for(int i = 0; i < train_path.size(); i++) {
    // for(int i = 0; i < 100; i++) {
      if(i % 1000 == 0) {
        cout << city << " train Location " << i << " of " << train_path.size()
             << endl;
      }
      stringstream ss;
      ss << city << "/images/"
         << setw(10) << setfill('0') << train_path[i]->id << "_"
         << train_path[i]->im_idx << ".jpg";
      string im_filename = ss.str();
      if(im_exists(string("../data/dataset/") + im_filename)) {
        train_im_list_file << im_filename << " " << 0 << endl;
        vector<float> label_vector, loss_weight_vector;
        for(auto const &dst : all_dsts) {
          if(experiment == string("direction")) {
            label_vector.push_back(train_labels.at(dst.first)[i]);
          } else {
            map<string, vector<double> > &label_stats =
                static_cast<ExhaustiveDatasetMaker *>(this)->label_stats;
            double l = (train_labels.at(dst.first)[i] -
                label_stats.at(dst.first)[0]) / label_stats.at(dst.first)[1];
            label_vector.push_back(static_cast<float>(l));
          }
          loss_weight_vector.push_back(train_loss_weights.at(dst.first)[i]);
          train_label_txt_file << label_vector.back() << " ";
        }
        train_label_txt_file << endl;
        vector<size_t> dim_vector = {1, label_vector.size()};
        bool done = train_label_hdf5_file->write(label_vector, dim_vector);
        done &= train_lw_hdf5_file->write(loss_weight_vector, dim_vector);
        if(!done) {
          cout << "Could not write to HDF5 files" << endl;
          return false;
        }
      } else {
        cout << im_filename << " does not exist on disk" << endl;
      }
    }
    if(experiment == string("distance")) {
      string label_stats_filename = exp_dir + string("/label_stats.txt");
      ofstream label_stats_file(label_stats_filename.c_str());
      if(!label_stats_file.is_open()) {
        cout << "Could not open " << label_stats_filename << endl;
        return false;
      }
      for(auto const &ls :
          static_cast<ExhaustiveDatasetMaker *>(this)->label_stats) {
        label_stats_file << ls.first << " " << float(ls.second[0])
                         << " " << float(ls.second[1]) << endl;
      }
    }
    // test
    for(int i = 0; i < test_path.size(); i++) {
    // for(int i = 0; i < 100; i++) {
      if(i % 1000 == 0) {
        cout << city << " test Location " << i << " of " << test_path.size()
             << endl;
      }
      stringstream ss;
      ss << city << "/images/"
         << setw(10) << setfill('0') << test_path[i]->id << "_"
         << test_path[i]->im_idx << ".jpg";
      string im_filename = ss.str();
      if(im_exists(string("../data/dataset/") + im_filename)) {
        test_im_list_file << im_filename << " " << 0 << endl;
        vector<float> label_vector, loss_weight_vector;
        for(auto const& dst : all_dsts) {
          if(experiment == string("direction")) {
            label_vector.push_back(test_labels.at(dst.first)[i]);
          } else {
            map<string, vector<double> > &label_stats =
                static_cast<ExhaustiveDatasetMaker *>(this)->label_stats;
            double l = (test_labels.at(dst.first)[i] -
                label_stats.at(dst.first)[0]) / label_stats.at(dst.first)[1];
            label_vector.push_back(static_cast<float>(l));
          }
          loss_weight_vector.push_back(test_loss_weights.at(dst.first)[i]);
          test_label_txt_file << label_vector.back() << " ";
        }
        test_label_txt_file << endl;
        vector<size_t> dim_vector = {1, label_vector.size()};
        bool done = test_label_hdf5_file->write(label_vector, dim_vector);
        done &= test_lw_hdf5_file->write(loss_weight_vector, dim_vector);
        if(!done) {
          cout << "Could not write to HDF5 files" << endl;
          return false;
        } 
      } else {
        cout << im_filename << " does not exist on disk" << endl;
      }
    }
  } else if(experiment == "pair") {
    string train_im_list_p_filename = exp_dir + string("/train_im_list_p.txt");
    string test_im_list_p_filename  = exp_dir + string("/test_im_list_p.txt");
    ofstream train_im_list_p_file(train_im_list_p_filename.c_str()),
             test_im_list_p_file (test_im_list_p_filename.c_str());
    if(!train_im_list_p_file.is_open()) {
      cout << "Could not open " << train_im_list_p_filename << endl;
      return false;
    }
    if(!test_im_list_p_file.is_open()) {
      cout << "Could not open " << test_im_list_p_filename << endl;
      return false;
    }

    // train
    for(int i = 0; i < train_path.size(); i+=2) {
    // for(int i = 0; i < 100; i+=2) {
      if(i % 1000 == 0) {
        cout << city << " train Location " << i/2 << " of " << train_path.size()/2
             << endl;
      }
      stringstream ss1, ss2;
      ss1 << city << "/images/"
          << setw(10) << setfill('0') << train_path[i]->id << "_"
          << train_path[i]->im_idx << ".jpg";
      ss2 << city << "/images/"
          << setw(10) << setfill('0') << train_path[i+1]->id << "_"
          << train_path[i+1]->im_idx << ".jpg";
      string im_filename1 = ss1.str(), im_filename2 = ss2.str();
      if(im_exists(string("../data/dataset/") + im_filename1) &&
          im_exists(string("../data/dataset/") + im_filename2)) {
        train_im_list_file   << im_filename1 << " " << 0 << endl;
        train_im_list_p_file << im_filename2 << " " << 0 << endl;
        vector<float> label_vector, loss_weight_vector;
        for(auto const &dst : all_dsts) {
          label_vector.push_back(train_labels.at(dst.first)[i/2]);
          loss_weight_vector.push_back(train_loss_weights.at(dst.first)[i/2]);
          train_label_txt_file << label_vector.back() << " ";
        }
        train_label_txt_file << endl;
        vector<size_t> dim_vector = {1, label_vector.size()};
        bool done = train_label_hdf5_file->write(label_vector, dim_vector);
        done &= train_lw_hdf5_file->write(loss_weight_vector, dim_vector);
        if(!done) {
          cout << "Could not write to HDF5 files" <<  endl;
          return false;
        } 
      } else {
        cout << im_filename1 << " or " << im_filename2
             << " do not exist on disk" << endl;
      }
    }
    train_im_list_p_file.close();

    // test
    for(int i = 0; i < test_path.size(); i+=2) {
    // for(int i = 0; i < 100; i+=2) {
      if(i % 1000 == 0) {
        cout << city << " test Location " << i/2 << " of " << test_path.size()/2
             << endl;
      }
      stringstream ss1, ss2;
      ss1 << city << "/images/"
          << setw(10) << setfill('0') << test_path[i]->id << "_"
          << test_path[i]->im_idx << ".jpg";
      ss2 << city << "/images/"
          << setw(10) << setfill('0') << test_path[i+1]->id << "_"
          << test_path[i+1]->im_idx << ".jpg";
      string im_filename1 = ss1.str(), im_filename2 = ss2.str();
      if(im_exists(string("../data/dataset/") + im_filename1) &&
          im_exists(string("../data/dataset/") + im_filename2)) {
        test_im_list_file   << im_filename1 << " " << 0 << endl;
        test_im_list_p_file << im_filename2 << " " << 0 << endl;
        vector<float> label_vector, loss_weight_vector;
        for(auto const &dst : all_dsts) {
          label_vector.push_back(test_labels.at(dst.first)[i/2]);
          loss_weight_vector.push_back(test_loss_weights.at(dst.first)[i/2]);
          test_label_txt_file << label_vector.back() << " ";
        }
        test_label_txt_file << endl;
        vector<size_t> dim_vector = {1, label_vector.size()};
        bool done = test_label_hdf5_file->write(label_vector, dim_vector);
        done &= test_lw_hdf5_file->write(loss_weight_vector, dim_vector);
        if(!done) {
          cout << "Could not write to HDF5 files" << endl;
          return false;
        }
      } else {
        cout << im_filename1 << " or " << im_filename2
             << " do not exist on disk" << endl;
      }
    }
    test_im_list_p_file.close();
  }
  
  train_label_list_file << "../" << train_label_hdf5_filename << endl;
  train_label_list_file.close();
  test_label_list_file << "../" << test_label_hdf5_filename << endl;
  test_label_list_file.close();

  train_label_txt_file.close(); 
  train_im_list_file.close();
  delete train_label_hdf5_file;
  test_label_txt_file.close(); 
  test_im_list_file.close();
  delete test_label_hdf5_file;

  return true;
}

size_t get_path_length(const vector<Node::Ptr> &path) {
  size_t len = 0;
  for(int i = 0; i < path.size()-1; i++) {
    if(!path[i]->equal(path[i+1])) len++;
  }
  return len;
}

ExhaustiveDatasetMaker::ExhaustiveDatasetMaker(string city) :
  BaseDatasetMaker(city, "distance"), geod(GeographicLib::Geodesic::WGS84()) {}

bool ExhaustiveDatasetMaker::generate_paths() {
  // enumerate all nodes
  ex_searcher->search();
  vector<Node::Ptr> path = ex_searcher->get_path();
  cout << "Finished enumerating nodes" << endl;

  // distances arrays for all destinations
  map<string, umap> all_distances;

  for(auto const &dst : all_dsts) {
    string dst_name = dst.first;
    vector<Node::Ptr> const &dsts = dst.second;
    cout << "@@@@@ Collecting labels for " << dst_name << endl;
    
    umap &distances = all_distances[dst_name];

    // pass 1: Get distances
    distances.reserve(path.size());
    for(int i = 0; i < path.size(); i++) distances[path[i]] = -1.f;
    cout << "Initialized distances map" << endl;
    auto it = distances.cbegin();
    size_t count = 0;
    while(true) {
      while(it != distances.cend()) {
        if(it->second < 0.f) break;
        it++;
        count++;
      }
      if(it == distances.cend()) break;

      cout << city << " : " << dst_name << " : " << " distance position " << count
           << " of " << distances.size() << endl;

      Node::Ptr start = it->first;
      if(start->im_idx < 0) {
        // indicates invalid nodes, will not be used for training
        distances[start] = +FLT_MAX;
        continue;
      }

      // find nearest destination in the direction pointed
      vector<double> dst_dists;
      for(auto const &ddst : dsts) {
        double distance, bearing_s_to_d, bearing_d_to_s;
        geod.Inverse(start->lat, start->lng, ddst->lat, ddst->lng, distance,
                     bearing_s_to_d, bearing_d_to_s);
        if(bearing_s_to_d < 0.0) bearing_s_to_d += 360.0;
        Direction::Enum d;
        if     (bearing_s_to_d > 315.0 || bearing_s_to_d <= 045.0)
          d = Direction::N;
        else if(bearing_s_to_d > 045.0 && bearing_s_to_d <= 135.0)
          d = Direction::E;
        else if(bearing_s_to_d > 135.0 && bearing_s_to_d <= 225.0)
          d = Direction::S;
        else if(bearing_s_to_d > 225.0 && bearing_s_to_d <= 315.0)
          d = Direction::W;
        if(d == start->d) {
          dst_dists.push_back(distance);
        } else {
          dst_dists.push_back(+FLT_MAX);
        }
      }
      distances[start] = float(*min_element(dst_dists.begin(), dst_dists.end()));
    }
  }

  // pass 2: collect labels
  train_path.clear();
  test_path.clear();
  label_stats.clear();
  map<string, double> label_sq_sums;
  map<string, size_t> ns;
  for(auto const& dst : all_dsts) {
    train_labels[dst.first].clear();
    test_labels [dst.first].clear();
    label_stats [dst.first].resize(2, 0.0);
    train_loss_weights[dst.first].clear();
    test_loss_weights [dst.first].clear();
    label_sq_sums[dst.first] = 0.0;
    ns[dst.first] = 0;
  }
  auto it = all_distances.cbegin();
  for(auto const &node : it->second) {
    Node::Ptr n = node.first;
    map<string, float> labels, loss_weights;

    float lw_sum = 0.f;
    for(auto const &dst : all_dsts) {
      float label = all_distances[dst.first].at(n);
      labels[dst.first] = label;
      // ignore all invalid nodes
      float lw = label < 1e10 ? 1.f : 0.f;
      loss_weights[dst.first] = lw;
      lw_sum += lw;
    }

    // if this node does not have labels for all dests
    // or im_idx < 0
    if(lw_sum < 0.5f) {
      continue;
    }

    switch(right_of_line(n->lat, n->lng)) {
      case true:  // train
        train_path.push_back(n);
        for(auto const &label : labels) {
          float l = sqrt(label.second);
          // float l = log(label.second + FLT_MIN);
          train_labels[label.first].push_back(l);
          if(l < 1e10) {
            label_stats.at(label.first)[0] += l;
            label_sq_sums[label.first] += label.second;
            ns[label.first] += 1;
          }
        }
        for(auto const &lw : loss_weights) {
          train_loss_weights[lw.first].push_back(lw.second);
        }
        break;
      case false:  // test
        test_path.push_back(n);
        for(auto const &label : labels) {
          float l = sqrt(label.second);
          // float l = log(label.second + FLT_MIN);
          test_labels[label.first].push_back(l);
        }
        for(auto const &lw : loss_weights) {
          test_loss_weights[lw.first].push_back(lw.second);
        }
        break;
    }
  }
  for(auto &ls : label_stats) {
    size_t n = ns[ls.first];
    // mean
    ls.second[0] /= n;
    // stdev
    ls.second[1] = sqrt((label_sq_sums[ls.first] / n) - (ls.second[0] * ls.second[0]));
  }

  return true;
}

AStarDatasetMaker::AStarDatasetMaker(string city) :
  BaseDatasetMaker(city, "direction") {}

int AStarDatasetMaker::get_astar_label(Node::Ptr const &s, Node::Ptr const&t) {
  int label = -1;
  if(t->d == s->d) {  // forward
    label = 0;
  } else if(t->d == ((s->d + 1) % Direction::N_DIRS)) {  // right
    label = 1;
  } else if(t->d == ((s->d - 1 + Direction::N_DIRS) %
        Direction::N_DIRS)) {  // left
    label = 2;
  } else if(abs(int(t->d) - int(s->d)) == 2) {  // turn around
    label = 3;
  } else {
    cout << "A* path " << s << " -> " << t << " is wrong"
      << endl;
  }
  return label;
}  

bool AStarDatasetMaker::generate_paths() {
  // enumerate all nodes
  ex_searcher->search();
  vector<Node::Ptr> path = ex_searcher->get_path();
  cout << "Finished enumerating nodes" << endl;

  // direction arrays for all destinations
  map<string, umap> all_directions;
  // loss weight arrays for all destinations
  map<string, umap> all_loss_weights;

  for(auto const &dst : all_dsts) {
    string dst_name = dst.first;
    vector<Node::Ptr> const &dsts = dst.second;
    cout << "@@@@@ Collecting labels for " << dst_name << endl;

    umap &directions = all_directions[dst_name];
    umap &loss_weights = all_loss_weights[dst_name];

    // pass 1: Get directions
    directions.reserve(path.size());
    for(int i = 0; i < path.size(); i++) directions[path[i]] = -1.f;
    cout << "Initialized directions map" << endl;
    auto it = directions.cbegin();
    size_t count = 0;
    while(true) {
      while(it != directions.cend()) {
        if(it->second < 0.f) break;
        it++;
        count++;
      }
      if(it == directions.cend()) break;

      cout << city << " : " << dst_name << " : A* position " << count
           << " of " << directions.size() << endl;

      Node::Ptr start = it->first;
      if(start->im_idx < 0) {
        // indicates invalid nodes, will not be used for training
        directions[start] = +FLT_MAX;
        continue;
      }

      // find nearest destination
      vector<float> dst_dists;
      for(int j = 0; j < dsts.size(); j++) {
        dst_dists.push_back(boost::static_pointer_cast<AStarSearcher>
            (astar_searcher)->a_star_heuristic(start, dsts[j]));
      }
      size_t nearest_dst = min_element(dst_dists.cbegin(), dst_dists.cend()) -
        dst_dists.cbegin();

      // A*
      bool path_found = astar_searcher->search(start, dsts[nearest_dst]);
      vector<Node::Ptr> astar_path;
      if(!path_found) {
        directions[start] = +FLT_MAX;
        continue;
      } else {
        astar_path = astar_searcher->get_path();
      }

      // record directions for nodes along path and their directional neighbors
      size_t path_length = get_path_length(astar_path);
      for(int i = 0, l = 0; i < astar_path.size(); i++) {
        if(i < astar_path.size()-1) {
          // find the node where location changes
          if(astar_path[i]->equal(astar_path[i+1]))
            continue;
        }

        // directional neighbors
        vector<Node::Ptr> nbrs = graph->get_nodes_at_loc(astar_path[i]->id);

        // record labels
        for(int j = 0; j < nbrs.size(); j++) {
          if(directions[nbrs[j]] < 0.f && nbrs[j]->im_idx >= 0) {
            int label = get_astar_label(nbrs[j], astar_path[i]);
            if(label >= 0) {
              directions[nbrs[j]]   = float(label);
              loss_weights[nbrs[j]] = pow(discount_factor,
                                          path_length - l);
            }
            else directions[nbrs[j]] = 77.f;
          }
        }
        l++;
      }
    }
  }

  // pass 2: collect labels
  train_path.clear(); test_path.clear();
  for(auto const& dst : all_dsts) {
    train_labels      [dst.first].clear();
    test_labels       [dst.first].clear();
    train_loss_weights[dst.first].clear();
    test_loss_weights [dst.first].clear();
  }
  auto it = all_directions.cbegin();
  for(auto const &node : it->second) {
    Node::Ptr n = node.first;
    map<string, float> labels, loss_weights;
    
    // get direction for all destinations
    for(auto const &dst : all_dsts) {
      float label = all_directions[dst.first].at(n);
      // ignore all invalid nodes
      if(label < 4.f) {
        labels[dst.first] = label;
        loss_weights[dst.first] = all_loss_weights[dst.first].at(n);
      } else {  // ignore label in CNN
        // cout << "No " << dst.first << " for " << n << endl;
      }
    }

    // if this node does not have labels for all distances
    if(labels.size() != all_dsts.size()) continue;

    switch(right_of_line(n->lat, n->lng)) {
      case true:  // train
        train_path.push_back(n);
        for(auto const &label : labels) {
          train_labels[label.first].push_back(label.second);
        }
        for(auto const &lw : loss_weights) {
          train_loss_weights[lw.first].push_back(lw.second);
        }
        break;
      case false:
        test_path.push_back(n);
        for(auto const &label : labels) {
          test_labels[label.first].push_back(label.second);
        }
        for(auto const &lw : loss_weights) {
          test_loss_weights[lw.first].push_back(lw.second);
        }
        break;
    }
  }

  return true; 
}

PairDatasetMaker::PairDatasetMaker(string city) :
  BaseDatasetMaker(city, "pair") {
}

bool PairDatasetMaker::generate_paths() {
  // enumerate all nodes
  ex_searcher->search();
  vector<Node::Ptr> path = ex_searcher->get_path();
  cout << "Finished enumerating nodes" << endl;

  // dominant direction arrays for all destinations
  map<string, unordered_map<size_t, int> > all_directions;
  //  dest                  id      dir
  // loss weight arrays for all destinations
  map<string, unordered_map<size_t, float> > all_loss_weights;

  for(auto const &dst : all_dsts) {
    string dst_name = dst.first;
    vector<Node::Ptr> const &dsts = dst.second;
    cout << "@@@@@ Collecting labels for " << dst_name << endl;

    unordered_map<size_t, int> &directions = all_directions[dst_name];
    unordered_map<size_t, float> &loss_weights = all_loss_weights[dst_name];

    // pass 1: Get directions
    for(int i = 0; i < path.size(); i++) directions[path[i]->id] = -1;
    cout << "Initialized dominant directions map" << endl;
    auto it = directions.cbegin();
    size_t count = 0;
    while(true) {
      // get iterator to first negative element
      while(it != directions.cend()) {
        if(it->second < 0) break;
        it++;
        count++;
      }
      if(it == directions.cend()) break;
      size_t id = it->first;

      cout << city << " : " << dst_name << " : pair position "
           << count << " of " << directions.size() << endl;

      // get valid start node
      Node::Ptr start = graph->get_valid_node(id);
      if(!start) {
        // indicates invalid nodes, will not be used for training
        directions[id] = +77;
        continue;
      }

      // find nearest destination
      vector<float> dst_dists;
      for(int j = 0; j < dsts.size(); j++) {
        dst_dists.push_back(boost::static_pointer_cast<AStarSearcher>
            (astar_searcher)->a_star_heuristic(start, dsts[j]));
      }
      size_t nearest_dst = min_element(dst_dists.cbegin(), dst_dists.cend()) -
        dst_dists.cbegin();

      // A*
      bool path_found = astar_searcher->search(start, dsts[nearest_dst]);
      vector<Node::Ptr> astar_path;
      if(!path_found) {
        directions[id] = +77;
        continue;
      } else {
        astar_path = astar_searcher->get_path();
      }

      // record directions for node ids along path
      size_t path_length = get_path_length(astar_path);
      for(int i = 0, l = 0; i < astar_path.size(); i++) {
        if(i < astar_path.size()-1) {
          // find the node where location changes
          if(astar_path[i]->equal(astar_path[i+1]))
            continue;
        }

        // record dominant direction
        if(directions[astar_path[i]->id] < 0) {
          directions[astar_path[i]->id] = static_cast<int>(astar_path[i]->d);
          loss_weights[astar_path[i]->id] = pow(discount_factor, path_length - l);
        }
        l++;
      }
    }
  }

  // pass 2: collect labels
  train_path.clear(); test_path.clear();
  for(auto const& dst : all_dsts) {
    train_labels[dst.first].clear();
    test_labels [dst.first].clear();
    train_loss_weights[dst.first].clear();
    test_loss_weights [dst.first].clear();
  }
  auto it = all_directions.cbegin();
  for(auto const &loc : it->second) {
    size_t id = loc.first;

    vector<Node::Ptr> nodes = graph->get_nodes_at_loc(id);
    bool train_or_test = right_of_line(nodes[0]->lat, nodes[0]->lng);
    // enumerate pairs
    for(int x = 0; x < nodes.size(); x++) {
      for(int y = x+1; y < nodes.size(); y++) {        
        map<string, float> labels, loss_weights;
    
        // get label for all destinations
        for(auto const &dst : all_dsts) {
          int dom_dir = all_directions[dst.first].at(id);
          if(dom_dir > 3) break;  // ignore invalid nodes
          loss_weights[dst.first] = all_loss_weights[dst.first].at(id);
          if(dom_dir == static_cast<int>(nodes[x]->d)) {
            labels[dst.first] = 0.f;
          } else if(dom_dir == static_cast<int>(nodes[y]->d)) {
            labels[dst.first] = 1.f;
          } else {
            labels[dst.first] = 2.f;  // caffe loss: ignore_label
          }
        }

        // if this node does not have labels for all distances
        if(labels.size() != all_dsts.size()) continue;

        if(nodes[x]->im_idx < 0 || nodes[y]->im_idx < 0) continue;

        switch(train_or_test) {
          case true:  // train
            train_path.push_back(nodes[x]);
            train_path.push_back(nodes[y]);
            for(auto const &label : labels) {
              train_labels[label.first].push_back(label.second);
            }
            for(auto const &lw : loss_weights) {
              train_loss_weights[lw.first].push_back(lw.second);
            }
            break;
          case false:
            test_path.push_back(nodes[x]);
            test_path.push_back(nodes[y]);
            for(auto const &label : labels) {
              test_labels[label.first].push_back(label.second);
            }
            for(auto const &lw : loss_weights) {
              test_loss_weights[lw.first].push_back(lw.second);
            }
            break;
        }
      }
    }
  }
  return true; 
}

/*
PairTextDatasetMaker::PairTextDatasetMaker(string city) :
  BaseDatasetMaker(city, "pair") {
}

bool PairTextDatasetMaker::generate_paths() {
  string direction_base_dir = exp_dir + string("../direction/");
  string train_im_list_filename = direction_base_dir +
    string("train_im_list.txt");
  string test_im_list_filename  = direction_base_dir +
    string("test_im_list.txt");
  // open input files
  ifstream train_im_list_file(train_im_list_filename.c_str()),
           test_im_list_file(test_im_list_filename.c_str());
  if(!train_im_list_file.is_open()) {
    cout << "Could not open " << train_im_list_filename << " for reading" << endl;
    return false;
  }
  if(!test_im_list_file.is_open()) {
    cout << "Could not open " << test_im_list_filename << " for reading" << endl;
    return false;
  }

  train_path.clear(); train_labels.clear();
  test_path.clear(); test_labels.clear();
  string im_filename;
  int label;
  // train
  for(; train_im_list_file >> im_filename >> label;) {
    if(label != 0) continue;
    // get node id and im_idx
    size_t id, im_idx;
    char ch;
    istringstream ss(im_filename.substr(im_filename.size()-16, 12));
    ss >> id >> ch >> im_idx;
    
    // get node and its neighbors
    vector<Node::Ptr> nbrs;
    Node::Ptr fwd_node;
    for(int d = 0; d < Direction::N_DIRS; d++) {
      Node::Ptr n = graph->get_node_ptr(id, static_cast<Direction::Enum>(d));
      if(n) {
        if(n->im_idx != im_idx) nbrs.push_back(n);
        else fwd_node = n;
      }
    }

    // add to path
    for(int i = 0; i < nbrs.size(); i++) {
      train_path.push_back(fwd_node);
      train_path.push_back(nbrs[i]);
    }
  }
  // test
  for(; test_im_list_file >> im_filename >> label;) {
    if(label != 0) continue;
    // get node id and im_idx
    size_t id, im_idx;
    char ch;
    istringstream ss(im_filename.substr(im_filename.size()-16, 12));
    ss >> id >> ch >> im_idx;
    
    // get node and its neighbors
    vector<Node::Ptr> nbrs;
    Node::Ptr fwd_node;
    for(int d = 0; d < Direction::N_DIRS; d++) {
      Node::Ptr n = graph->get_node_ptr(id, static_cast<Direction::Enum>(d));
      if(n) {
        if(n->im_idx != im_idx) nbrs.push_back(n);
        else fwd_node = n;
      }
    }

    // add to path
    for(int i = 0; i < nbrs.size(); i++) {
      test_path.push_back(fwd_node);
      test_path.push_back(nbrs[i]);
    }
  }
  return true;
} 
*/


