#ifndef GRAPH_MAKER_H
#define GRAPH_MAKER_H

#include <graph.h>

class GraphMaker {
  private:
    // graph
    Graph::Ptr graph;
    // name of city
    std::string city;
    // directories
    std::string base_dir, nodes_dir, links_dir, images_dir;
    // convenience function to add nodes in all 4 directions (with turn connections)
    bool add_nodes(size_t id, float lat, float lng);
  public:
    GraphMaker(std::string city);
    bool construct_graph();
    Graph::Ptr get_graph() const {return graph;};
};

#endif
