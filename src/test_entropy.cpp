// program to get entropy plot from trained AVS network
#include <tester.h>

#include <iostream>
#include <string>
#include <boost/make_shared.hpp>

using namespace std;

int main(int argc, char **argv) {
  // command line arguments
  if(argc != 4) {
    cout << "Usage: " << argv[0] << " city destination_name experiment" << endl;
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
  bool done = tester->calc_entropies();

  return (done ? 0 : -1);
}
