// program to make dataset for training AVS network
#include <dataset_maker.h>
#include <boost/make_shared.hpp>
#include <iostream>

using namespace std;

int main(int argc, char **argv) {
  // command line arguments
  if(argc != 3) {
    cout << "Usage: ./test_dataset_maker city method" << endl;
    return -1;
  }

  BaseDatasetMaker::Ptr db_maker;

  if(string(argv[2]) == string("pair"))
    db_maker = boost::make_shared<PairDatasetMaker>(argv[1]);
  else if(string(argv[2]) == string("direction"))
    db_maker = boost::make_shared<AStarDatasetMaker>(argv[1]);
  else if(string(argv[2]) == string("distance"))
    db_maker = boost::make_shared<ExhaustiveDatasetMaker>(argv[1]);
  else {
    cout << "Wrong method " << argv[2] << endl;
    return -1;
  }

  db_maker->generate_paths();
  bool done = db_maker->make_dataset();
  return (done ? 0 : -1);
  // return 0;
}
