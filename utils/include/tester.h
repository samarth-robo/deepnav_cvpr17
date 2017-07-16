#ifndef TESTER_H
#define TESTER_H

#include <caffe_wrapper.h>
#include <graph.h>
#include <graph_maker.h>
#include <search.h>
#include <sklearn_wrapper.h>
#include <unordered_set>

class BaseTester {
  public:
    typedef boost::shared_ptr<BaseTester> Ptr;
    BaseTester(std::string city, std::string dst_name, std::string experiment,
               int max_path_length=1000, float dest_radius=10.f);  // 75.f
    void set_graph_maker(GraphMaker gm) {graph_maker = gm;}
    Graph::Ptr get_graph() {return graph;}
    // function to search; implemented by children
    virtual bool find(size_t start) = 0;
    std::vector<Node::Ptr> get_path() {return path;}
    size_t get_path_length();
    bool gather_astar_stats(size_t start);
    size_t get_shortest_path_length() {return shortest_path_length;}
    float get_shortest_path_cost() { return shortest_path_cost;}
    virtual bool calc_entropies() = 0;
    virtual bool sort_nodes() {return true;}
  protected:
    // utility functions
    bool is_node_in_path(Node::Ptr const &n);
    bool near_dst(Node::Ptr const &n);
    std::string get_im_name(Node::Ptr const &n);
    cv::Mat get_im(Node::Ptr const &n);
    // backtracking function
    Node::Ptr backtrack(Node::Ptr const &r, float current_dist_est = 0.f);
    // graph over the ground plane
    Graph::Ptr graph;
    // object for constructing graph over the ground plane
    GraphMaker graph_maker;
    // city and destination
    std::string city, dst_name;
    // experiment
    std::string experiment;
    // useful directories
    std::string base_dir, im_base_dir;
    // array storing generated paths
    std::vector<Node::Ptr> path;
    // array storing destinations
    std::vector<Node::Ptr> dsts;
    // array storing the open options and their costs (for backtracking)
    std::unordered_set<Node::Ptr, NodeHash, NodeEqual> open_options;
    // interface to Caffe
    CaffeWrapper *caffe;
    // caffe proto and mean
    std::string proto_filename, model_filename, mean_filename;
    // maximum path length
    int max_path_length;
    // radius around destination for end test
    float dest_radius;
    // geogeaphic object to calculate distance
    const GeographicLib::Geodesic &geod;
    // index in CNN output for this destination
    int net_idx;
    // A* searcher for gathering shortest path stats
    Searcher::Ptr searcher;
    // A* shortest path stats
    size_t shortest_path_length;
    float shortest_path_cost;
};

// navigates based on estimates of distance to destination (CNN)
class DistanceTester : public BaseTester {
  public:
    DistanceTester(std::string city, std::string dst_name);
    bool find(size_t start);
    bool calc_entropies();
  private:
    float mean, stdev;
};

// navigates based on estimates of distance to destination (GIST)
class DistanceGISTTester : public BaseTester {
  public:
    DistanceGISTTester(std::string city, std::string dst_name);
    bool find(size_t start);
    bool calc_entropies();
  private:
    float mean, stdev;
    SKLearnWrapper sklearn_wrapper;
};

// navigates based on direction picked by CNN (seeing only forward)
class DirectionTester : public BaseTester {
  public:
    DirectionTester(std::string city, std::string dst_name);
    bool find(size_t start);
    std::vector<std::pair<float, int> > get_net_out(Node::Ptr const &n);
    bool calc_entropies();
};

// navigates based on direction picked by CNN (seeing all options)
// uses find() from DistanceTester
class PairTester : public BaseTester {
  public:
    PairTester(std::string city, std::string dst_name);
    bool find(size_t start);
    bool calc_entropies();
    bool sort_nodes();
};

// navigates based on random decisions
class RandomWalkTester : public BaseTester {
  public:
    RandomWalkTester(std::string city, std::string dst_name) :
      BaseTester(city, dst_name, "random") {}
    bool find(size_t start);
    bool calc_entropies();
};

#endif
