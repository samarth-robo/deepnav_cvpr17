#include <common.h>

#include <iostream>

using namespace std;

int main(int argc, char **argv) {
  if(argc != 2) {
    cout << "Udage: ./test_cleaner city" << endl;
    return -1;
  }

  bool done = clean_im_list("/data/dataset/san_francisco/train_im_list.txt");
  done     &= clean_im_list("/data/dataset/san_francisco/test_im_list.txt");

  return (done ? 0 : -1);
}
