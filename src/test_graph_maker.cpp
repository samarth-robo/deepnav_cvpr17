// program to construct graph from SV data
#include <graph_maker.h>

#include <iostream>

using namespace std;

int main(int argc, char **argv) {
  if(argc != 2) {
    cout << "Usage: ./test_graph_maker city" << endl;
    return -1;
  }

  GraphMaker gm(argv[1]);
  cout << "Constructing graph" << endl;
  gm.construct_graph();
  Graph::Ptr graph = gm.get_graph();

  return 0;
}
