#include <common.h>
#include <graph.h>
#include <graph_maker.h>
#include <search.h>

#include <iostream>

using namespace std;

int main(int argc, char **argv) {
  if(argc != 2) {
    cout << "Usage: ./test_search city" << endl;
    return -1;
  }

  GraphMaker gm(argv[1]);
  gm.construct_graph();
  Graph::Ptr graph = gm.get_graph();

  // search
  Searcher::Ptr s(new AStarSearcher(graph));
  // Searcher::Ptr s(new ExhaustiveSearcher(graph));
  Node::Ptr start(new Node(80354, Direction::W)),
            end  (new Node(79994, Direction::E));
  bool path_found = s->search(start, end);
  if(path_found) {
    vector<Node::Ptr> path = s->get_path();
    cout << "Path is" << endl;
    for(int i = 0; i < path.size(); i++) {
      cout << path[i] << ", cost = " << s->get_cost(path[i], end) << endl;
    }
    bool done = write_path(path, "../data/path_a*.txt");
  } else {
    cout << "Path not found" << endl;
  }

  return 0;
}
