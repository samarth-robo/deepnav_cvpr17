// program to calculate how good a random walker can be in a dataset
// metric is % of correct decisions made
#include <graph.h>
#include <graph_maker.h>
#include <search.h>

#include <iostream>

using namespace std;

int main(int argc, char **argv) {
  if(argc != 2) {
    cout << "Usage: ./" << argv[0] << " city" << endl;
    return -1;
  }

  GraphMaker gm(argv[1]);
  bool done = gm.construct_graph();
  if(!done) {
    cout << "Could not construct graph" << endl;
    return -1;
  }
  Graph::Ptr graph = gm.get_graph();
  Searcher *s = new ExhaustiveSearcher(graph);
  s->search();
  vector<Node::Ptr> const &nodes = s->get_path();

  unordered_map<size_t, float> chances;
  for(auto const &node : nodes) {
    if(!chances.count(node->id)) {
      vector<Node::Ptr> const &ns = graph->get_nodes_at_loc(node->id);
      chances[node->id] = 1.f / ns.size();
    }
  }

  double chance_acc = 0.0;
  for(auto ch : chances) chance_acc += ch.second;

  cout << "Chance is " << chance_acc / chances.size() << endl;

  return 0;
}
