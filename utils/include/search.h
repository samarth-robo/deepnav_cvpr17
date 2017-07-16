#ifndef SEARCH_H
#define SEARCH_H

#include <graph.h>

#include <cfloat>
#include <GeographicLib/Geodesic.hpp>
#include <queue>

class NodeHash {
  public:
    size_t operator()(Node::Ptr const &n) const;
};

class NodeEqual {
  public:
    bool operator()(Node::Ptr const &a, Node::Ptr const &b) const;
};

class PairCompare {
  public:
    bool operator()(std::pair<Node::Ptr, double> const &p1,
        std::pair<Node::Ptr, double> const &p2) const;
};

class Searcher {
  public:
    typedef boost::shared_ptr<Searcher> Ptr;
    Searcher() {}
    Searcher(Graph::ConstPtr graph) : graph(graph) {}
    void set_graph(Graph::ConstPtr graph_) {graph = graph_;}
    virtual bool search(Node::Ptr const &start = Node::Ptr(),
        Node::Ptr const &end = Node::Ptr()) = 0;
    std::vector<Node::Ptr> get_path() {return path;}
    size_t get_path_length();
    virtual float get_cost(Node::Ptr const &start, Node::Ptr const &end) const = 0;
  protected:
    Graph::ConstPtr graph;
    std::vector<Node::Ptr> path;
};

// class for A* search
class AStarSearcher : public Searcher {
  public:
    typedef boost::shared_ptr<AStarSearcher> Ptr;
    AStarSearcher() : Searcher(), geod(GeographicLib::Geodesic::WGS84()) {}
    AStarSearcher(Graph::ConstPtr graph) : Searcher(graph),
      geod(GeographicLib::Geodesic::WGS84()) {}
    bool search(Node::Ptr const &start, Node::Ptr const &end);
    float a_star_heuristic(Node::Ptr const &s, Node::Ptr const &t) const;
    float get_cost(Node::Ptr const &start, Node::Ptr const &end) const;
  private:
    const GeographicLib::Geodesic& geod;
    // A* data structures
    std::priority_queue<std::pair<Node::Ptr, float>,
      std::vector<std::pair<Node::Ptr, float> >, PairCompare> q;
    std::unordered_map<Node::Ptr, float, NodeHash, NodeEqual> g_score;
    std::unordered_map<Node::Ptr, Node::Ptr, NodeHash, NodeEqual> came_from;
};

// class for exhaustive enumeration
class ExhaustiveSearcher : public Searcher {
  public:
    typedef boost::shared_ptr<ExhaustiveSearcher> Ptr;
    ExhaustiveSearcher() : Searcher() {}
    ExhaustiveSearcher(Graph::Ptr graph) : Searcher(graph) {}
    // 'search' (exhaustive enumeration) does not need start or end
    bool search(Node::Ptr const &start = Node::Ptr(),
        Node::Ptr const &end = Node::Ptr());
    float get_cost(Node::Ptr const &start, Node::Ptr const &end) const{
      return -1.f;
    }
};

#endif
