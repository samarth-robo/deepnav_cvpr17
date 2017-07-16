#include <common.h>

#include <fstream>
#include <iomanip>
#include <iostream>

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

bool write_path(vector<Node::Ptr> path, string filename) {
  ofstream f(filename.c_str());
  if(!f.is_open()) {
    cout << "Could not open " << filename << " for writing" << endl;
    return false;
  }
  f << "bin_id,latitude,longitude,im_idx" << endl;
  
  for(int i = 0; i < path.size(); i++) {
    f << path[i]->id << "," << setprecision(7) << path[i]->lat
      << "," << path[i]->lng << "," << path[i]->im_idx << endl;
  }
  f.close();
  return true;
}

bool clean_im_list(string filename) {
  string base_dir = "../";
  filename = base_dir + filename;
  cout << "F_in = " << filename << endl;
  ifstream fin(filename.c_str());
  if(!fin.is_open()) {
    cout << "Could not open " << filename << " for reading" << endl;
    return false;
  }
  string ofilename = filename.substr(0, filename.length()-4) + string("_cleaned.txt");
  cout << "F_out = " << ofilename << endl;
  ofstream fout(ofilename.c_str());
  if(!fout.is_open()) {
    cout << "Could not open " << ofilename << " for writing" << endl;
    return false;
  }
  string im_filename;
  int label;
  for(size_t count = 0; fin >> im_filename >> label; count++) {
    if(im_exists(base_dir + im_filename)) {
      fout << im_filename << " " << label << endl;
    } else {
      cout << "Image " << base_dir + im_filename << " does not exist" << endl;
    }
    if(count % 1000 == 0) cout << "Cleaning " << count << endl;
  }
  return true;
}

bool im_exists(string filename) {
  Mat im = cv::imread(filename);
  return !im.empty();
}
