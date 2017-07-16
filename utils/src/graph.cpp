#include <graph.h>
#include <boost/functional/hash.hpp>

#include <iostream>

using namespace std;

char Direction::get_char(Direction::Enum d) {
  switch(d) {
    case Direction::N: return 'N';
    case Direction::E: return 'E';
    case Direction::S: return 'S';
    case Direction::W: return 'W';
  }
}

Node::Node(size_t id, Direction::Enum d, float lat, float lng, int im_idx) :
  id(id), d(d), lat(lat), lng(lng) , im_idx(im_idx) {}

// for printing node
std::ostream& operator<<(std::ostream& os, Node::Ptr const &n) {
  if(n) {
    char d = Direction::get_char(n->d);
    return os << n->id << " : " << d;
  } else {
    return os << "Tried to print empty Node::Ptr" << endl;
  }
}

bool Graph::add_node(size_t id, Direction::Enum d, float lat, float lng,
    int im_idx) {
  Node::Ptr n(new Node(id, d, lat, lng, im_idx));
  auto p = nodes.emplace(piecewise_construct, forward_as_tuple(id, static_cast<int>(d)),
      forward_as_tuple(n));
  bool emplaced = p.second;
  Node::Ptr nn = (p.first)->second;
  if(!emplaced) {  // node already exists
    if(nn->im_idx < 0) nn->set_im_idx(im_idx);
  }
  return p.second;
}

bool Graph::add_edge_directed(size_t id1, Direction::Enum d1,
    size_t id2, Direction::Enum d2, float dist) {
  Node::Ptr n1 = get_node_ptr(id1, d1), n2 = get_node_ptr(id2, d2);
  if(n1 && n2) { 
    n1->nbrs.push_back(n2);
    n1->distances.push_back(dist);
    return true;
  } else {
    cout << "One of the nodes " << id1 << " : " << Direction::get_char(d1)
         << " and " << id2 << " : " << Direction::get_char(d2)
         << " do not exist" << endl;
    return false;
  }
}

bool Graph::add_edge_undirected(size_t id1, Direction::Enum d1,
    size_t id2, Direction::Enum d2, float dist) {
  bool done = add_edge_directed(id1, d1, id2, d2, dist);
  done     &= add_edge_directed(id2, d2, id1, d1, dist);
  return done;
}

Node::Ptr Graph::get_node_ptr(size_t id, Direction::Enum d) const {
  umap::const_iterator got = nodes.find(make_pair(id, static_cast<int>(d)));
  if(got != nodes.end()) return got->second;
  else {
    /*
    cout << "Accessing illegal node " << id << " : " << Direction::get_char(d)
         << endl;
     */
    return Node::Ptr();
  }
}

pair<Node::Ptr, float> Graph::get_nbrd(Node::Ptr const &n,
    Direction::Enum d) const {
  pair<Node::Ptr, float> out(Node::Ptr(), 0.f);
  if(!n) return out;
  Node::Ptr nd = get_node_ptr(n->id, d);
  if(!nd) return out;
  if(nd->nbrs.empty()) return out;
  if(nd->nbrs[0]->d == d) {
    out.first = nd->nbrs[0];
    out.second = nd->distances[0];
  }
  return out;
}

vector<Node::Ptr> Graph::get_nodes_at_loc(size_t id) const {
  vector<Node::Ptr> nbrs;
  for(int d = 0; d < Direction::N_DIRS; d++) {
    Node::Ptr n = get_node_ptr(id, static_cast<Direction::Enum>(d));
    if(n) nbrs.push_back(n);
  }
  return nbrs;
}

Node::Ptr Graph::get_valid_node(size_t id) const {
  vector<Node::Ptr> const &ns = get_nodes_at_loc(id);
  for(auto const &n : ns) {
    if(n->im_idx >= 0) {return n;}
  }
  return Node::Ptr();
} 

size_t PairHash::operator()(pair<size_t, int> const &p) const {
  size_t seed = 7;
  boost::hash_combine(seed, p.first);
  boost::hash_combine(seed, p.second);
  return seed;
}

bool PairEqual::operator()(pair<size_t, int> const &a,
    pair<size_t, int> const &b) const {
  return (a.first == b.first) && (a.second == b.second);
}
