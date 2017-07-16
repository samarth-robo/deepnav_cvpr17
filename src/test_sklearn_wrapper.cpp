#include <sklearn_wrapper.h>

using namespace std;

int main(int argc, char **argv) {
  if(argc != 3) {
    cout << "Usage: " << argv[0] << " city dest_name" << endl;
    return -1;
  }

  SKLearnWrapper sk(argv[1], argv[2]);
  float pred = sk.get_pred("../data/dataset/test/images/0000062473_1.jpg");
  cout << "Pred = " << pred << endl;

  return 0;
}