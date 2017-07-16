#ifndef DATASET_MAKER_H
#define DATASET_MAKER_H

#include <map>

#include <graph.h>
#include <graph_maker.h>
#include <search.h>

class BaseDatasetMaker {
  public:
    typedef boost::shared_ptr<BaseDatasetMaker> Ptr;
    typedef std::unordered_map<Node::Ptr, float, NodeHash, NodeEqual> umap;
    BaseDatasetMaker(std::string city, std::string experiment,
                     float discount_factor=0.9f);
    void set_graph_maker(GraphMaker gm) {graph_maker = gm;}
    // function to generate paths and labels; implemented by children
    virtual bool generate_paths() = 0;
    // function to create dataset
    bool make_dataset();
    bool rename_dataset();
  protected:
    // graph
    Graph::Ptr graph;
    // object for constructing graph
    GraphMaker graph_maker;
    // Searcher objects
    Searcher::Ptr astar_searcher, ex_searcher;
    // dataset
    std::string city, experiment;
    // base directories for dataset
    std::string base_dir, exp_dir;
    // array storing generated paths
    std::vector<Node::Ptr> train_path, test_path;
    // array storing labels, separately for each destination
    std::map<std::string, std::vector<float> > train_labels, test_labels;
    // array storing weights to be applied to CNN loss
    std::map<std::string, std::vector<float> > train_loss_weights,
        test_loss_weights;
    // array storing the destinations
    std::map<std::string, std::vector<Node::Ptr> > all_dsts;
    // points for defining train/test boundary
    double line_lat1, line_lng1, line_lat2, line_lng2, line_slope, line_intercept;
    // discount factor for location aware loss
    float discount_factor;
    // function for classifying point right/left of train/test boundary
    bool right_of_line(float lat, float lng);
};

// captures images at every node in the graph
class ExhaustiveDatasetMaker : public BaseDatasetMaker {
  public:
    ExhaustiveDatasetMaker(std::string city);
    bool generate_paths();
    std::map<std::string, std::vector<double> > label_stats;  // mean, stdev
  private:
    const GeographicLib::Geodesic& geod;
};

// captures images along the path of A* searches
class AStarDatasetMaker : public BaseDatasetMaker {
  public:
    AStarDatasetMaker(std::string city);
    bool generate_paths();
  private:
    int get_astar_label(Node::Ptr const &s, Node::Ptr const&t);
};

// pair dataset
class PairDatasetMaker : public BaseDatasetMaker {
  public:
    PairDatasetMaker(std::string city);
    bool generate_paths();
};

/*
// pair dataset (convert from txt files on disk)
class PairTextDatasetMaker : public BaseDatasetMaker {
  public:
    PairTextDatasetMaker(std::string city);
    bool generate_paths();
};
*/

#endif
