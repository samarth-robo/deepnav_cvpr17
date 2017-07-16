// program to evaluate AVS network
#include <eval.h>

using namespace std;

int main(int argc, char **argv) {
  // command line arguments
  if(argc != 4) {
    cout << "Usage: ./test_tester city dest_name experiment" << endl;
    return -1;
  }

  BaseEvaluator *eval;
  if(string(argv[3]) == string("distance"))
    eval = new DistanceEvaluator(string(argv[1]), string(argv[2]));
  else if(string(argv[3]) == string("distance_gist"))
    eval = new DistanceGISTEvaluator(string(argv[1]), string(argv[2]));
  else if(string(argv[3]) == string("direction"))
    eval = new DirectionEvaluator(string(argv[1]), string(argv[2]));
  else if(string(argv[3]) == string("pair"))
    eval = new PairEvaluator(string(argv[1]), string(argv[2]));
  else if(string(argv[3]) == string("random"))
    eval = new RandomWalkEvaluator(string(argv[1]), string(argv[2]));
  else {
    cout << "Wrong experiment " << argv[3] << endl;
    return -1;
  }
  eval->evaluate();

  return 0;
}
