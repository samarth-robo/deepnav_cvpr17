// program to make dataset for training AVS network
#include <tester.h>

#include <boost/make_shared.hpp>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char **argv) {
  // command line arguments
  if(argc != 5) {
    cout << "Usage: " << argv[0] << " city destination_name experiment start_id" << endl;
    return -1;
  }

  BaseTester::Ptr tester;
  if(string(argv[3]) == string("pair")) 
    tester = boost::make_shared<PairTester>(argv[1], argv[2]);
  else if(string(argv[3]) == string("distance")) 
    tester = boost::make_shared<DistanceTester>(argv[1], argv[2]);
  else if(string(argv[3]) == string("hog")) 
    tester = boost::make_shared<DistanceGISTTester>(argv[1], argv[2]);
  else if(string(argv[3]) == string("direction")) 
    tester = boost::make_shared<DirectionTester>(argv[1], argv[2]);
  else if(string(argv[3]) == string("random")) 
    tester = boost::make_shared<RandomWalkTester>(argv[1], argv[2]);
  else {
    cout << "Wrong expriment " << argv[3] << endl;
    return -1;
  }

  bool done = tester->find(size_t(stoi(argv[4])));
  if(done) {
    size_t length = tester->get_path_length();
    cout << "Path found : " << length << endl;
  } else
    cout << "Path not found" << endl;

  return (done ? 0 : -1);
}
