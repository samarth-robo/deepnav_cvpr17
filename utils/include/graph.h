#ifndef GRAPH_H
#define GRAPH_H

#include <boost/shared_ptr.hpp>
#include <string.h>
#include <unordered_map>
#include <vector>

namespace Direction {
  enum Enum {N, E, S, W, N_DIRS};
  char get_char(Enum d);
}

struct Node {
  typedef boost::shared_ptr<Node> Ptr;
  Node(size_t id, Direction::Enum d = Direction::N, float lat = 0.f,
      float lng = 0.f, int im_idx = -1);
  size_t id;
  Direction::Enum d;
  std::vector<Node::Ptr> nbrs;
  std::vector<float> distances;
  float lat, lng;
  int im_idx;
  bool equal(Node::Ptr const &n) {return (id == n->id);}
  bool equald(Node::Ptr const &n) {return (id == n->id && d==n->d);}
  void set_im_idx(int im_idx_) {im_idx = im_idx_;}
};

std::ostream& operator<<(std::ostream& os, Node::Ptr const &n);

struct PairHash {
  size_t operator()(std::pair<size_t, int> const &p) const;
};

struct PairEqual {
  bool operator()(std::pair<size_t, int> const &a,
      std::pair<size_t, int> const &b) const;
};

class Graph {
  public:
    typedef boost::shared_ptr<Graph> Ptr;
    typedef boost::shared_ptr<const Graph> ConstPtr;
    typedef std::unordered_map<std::pair<size_t, int>, Node::Ptr, PairHash,
            PairEqual> umap;

    bool empty() const {return nodes.empty();}
    size_t get_n_nodes() const {return nodes.size();}
    Node::Ptr get_node_ptr(size_t id, Direction::Enum d = Direction::N)
      const;
    std::vector<Node::Ptr> get_nodes_at_loc(size_t id) const;
    std::pair<Node::Ptr, float> get_nbrd(Node::Ptr const &n, Direction::Enum d)
      const;
    Node::Ptr get_valid_node(size_t id) const;
    umap::const_iterator cbegin() const {return nodes.cbegin();}
    umap::const_iterator cend()   const {return nodes.cend();}

    bool add_node(size_t id, Direction::Enum d, float lat, float lng,
        int im_idx = -1);
    bool add_edge_directed(size_t id1, Direction::Enum d1,
        size_t id2, Direction::Enum d2, float dist = 0.f);
    bool add_edge_undirected(size_t id1, Direction::Enum d1,
        size_t id2, Direction::Enum d2, float dist = 0.f);

  private:
    umap nodes;
    unsigned int grid_dim_x, grid_dim_y;
};

#endif
