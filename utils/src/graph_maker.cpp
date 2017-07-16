// program to create a graph from Street View locations
// input: txt files describing nodes and links
// output: graph

#include <graph_maker.h>

#include <algorithm>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <string>
#include <sstream>

using namespace std;
namespace fs = boost::filesystem;

GraphMaker::GraphMaker(string city) : city(city), graph(new Graph) {
  base_dir = string("../data/dataset/") + city + string("/");
  nodes_dir  = base_dir + string("/nodes/");
  links_dir  = base_dir + string("/links/");
  images_dir = base_dir + string("/images/");
}

bool GraphMaker::construct_graph() {
  vector<string> node_files;
  // crawl all files in nodes_dir
  if(fs::is_directory(nodes_dir)) {
    for(fs::directory_iterator i(nodes_dir); i != fs::directory_iterator(); i++) {
      string filename(i->path().filename().string());
      if(filename.find("txt") != string::npos) {
        node_files.push_back(nodes_dir + filename);
        cout << "Using " << filename << endl;
      }
    }
  }

  // first read node files and add them to a dummy graph without connections
  Graph::Ptr dgraph(new Graph);
  for(int i = 0; i < node_files.size(); i++) {
    ifstream f(node_files[i].c_str());
    if(!f.is_open()) {
      cout << "Could not open " << node_files[i] << " for reading" << endl;
      return false;
    }
    string line;
    getline(f, line);
    getline(f, line);
    getline(f, line);
    for(; getline(f, line);) {
      size_t id;
      float lat, lng;
      char ch;
      istringstream iss(line);
      iss >> id >> ch >> lat >> ch >> lng;
      bool done = dgraph->add_node(id, Direction::N, lat, lng);
      if(!done) {
        cout << "Could not add node " << id << " to dummy graph" << endl;
      }
    }
    f.close();
  }

  // add nodes with directions and edges to main graph
  size_t edge_count = 0;
  unordered_map<size_t, set<int> > dirs;
  for(auto it = dgraph->cbegin(); it != dgraph->cend(); it++) {
    size_t id = it->first.first;
    stringstream ss;
    ss << "/" << setw(10) << setfill('0') << id << ".txt";
    string filename = links_dir + ss.str();
    ifstream f(filename.c_str());
    if(!f.is_open()) {
      cout << "Could not open " << filename << " for reading" << endl;
      return false;
    }
    string line;
    for(int im_idx = 0; getline(f, line); im_idx++) {
      size_t nbr_id;
      float heading, dist;  // absolute heading, 0 = North, 90 = East
      char ch;
      istringstream iss(line);
      iss >> nbr_id >> ch >> heading >> ch >> dist;
      if(heading < 0.f || heading > 360.f) {
        cout << "WARN: Heading " << heading << " for id " << id << " is wrong!" << endl;
        return false;
      }

      // use heading to determine NSEW direction
      Direction::Enum d, opp_d;
      if     (heading > 315.f || heading <= 045.f) {
        d = Direction::N;
        opp_d = Direction::S;
      }
      else if(heading > 045.f && heading <= 135.f) {
        d = Direction::E;
        opp_d = Direction::W;
      }
      else if(heading > 135.f && heading <= 225.f) {
        d = Direction::S;
        opp_d = Direction::N;
      }
      else if(heading > 225.f && heading <= 315.f) {
        d = Direction::W;
        opp_d = Direction::E;
      }

      // add nodes and inter-node links
      Node::Ptr n = dgraph->get_node_ptr(id, Direction::N),
        nbr = dgraph->get_node_ptr(nbr_id, Direction::N);
      if(n) {
        graph->add_node(n->id, d, n->lat, n->lng, im_idx);
        dirs[id].insert(static_cast<int>(d));
        graph->add_node(n->id, opp_d, n->lat, n->lng);
        dirs[id].insert(static_cast<int>(opp_d));
      }
      if(nbr) {
        graph->add_node(nbr->id, d, nbr->lat, nbr->lng);
        dirs[nbr->id].insert(static_cast<int>(d));
        graph->add_node(nbr->id, opp_d, nbr->lat, nbr->lng);
        dirs[nbr->id].insert(static_cast<int>(opp_d));
      }
      if(n && nbr) {
        bool done = graph->add_edge_directed(n->id, d, nbr->id, d, dist);
        if(done) edge_count ++;
        done      = graph->add_edge_directed(nbr->id, opp_d, n->id, opp_d, dist);
        if(done) edge_count ++;
      } else {
        cout << "One of the nodes " << id << " : " << Direction::get_char(d)
             << " and " << nbr_id << " : " << Direction::get_char(d)
             << " do not exist in nodes file, but exist in links file" << endl;
      }
    }
    f.close();
  }

  // add intra-node turn links
  cout << "Adding intra node turn links" << endl;
  size_t count = 0;
  for(auto node_it = dirs.cbegin(); node_it != dirs.cend(); node_it++, count++) {
    if(count % 100 == 0) cout << "Node " << count << " / " << dirs.size() << endl;
    size_t id = node_it->first;
    vector<int> ndirs;
    for(auto dir_it = (node_it->second).cbegin(); dir_it != (node_it->second).cend(); dir_it++) {
      ndirs.push_back(*dir_it);
    }
    for(int i = 0; i < ndirs.size(); i++) {
      // only neighboring directions are connected
      // if((ndirs[i] + 1) % Direction::N_DIRS == ndirs[(i+1) % ndirs.size()]) {
      if(true) {  // full directional connectivity
        bool done = graph->add_edge_undirected(id, static_cast<Direction::Enum>(ndirs[i]),
            id, static_cast<Direction::Enum>(ndirs[(i+1) % ndirs.size()]));
        if(done) {
          edge_count += 2;
        }
      }
    }
  }

  cout << "Graph construction complete, " << graph->get_n_nodes() << " nodes, "
       <<  edge_count << " edges." << endl;
  return true;
}
