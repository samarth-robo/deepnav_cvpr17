#include <caffe_wrapper.h>

using namespace std;
using namespace cv;

int main(int argc, char **argv) {
  if(argc != 2) {
    cout << "Usage: ./test_caffe_wrapper image_path" << endl;
    return -1;
  }
  Mat im = imread(argv[1], CV_LOAD_IMAGE_COLOR);
  if(im.empty()) {
    cout << "Could not read " << argv[1] << endl;
    return -1;
  }
  CaffeWrapper *caffe = new CaffeWrapper("../ml/distance/deploy.prototxt",
      "../output/streetview/san_francisco_vgg_distance_iter_11250.caffemodel",
      "../data/mean_image.binaryproto");
  vector<float> pred(caffe->get_pred(im));
  for(int i = 0; i < pred.size(); i++) cout << pred[i] << " ";
  cout << endl;
  return 0;
}

/* proto files to test:
name: "DeepNavAlexNet"
layer {
  name: "data"
  type: "ImageData"
  top: "data"
  top: "label"
  transform_param {
    crop_size: 227 
    mean_file: "../../data/mean_image.binaryproto"
    mirror: false
  }
  image_data_param {
    source: "../../data/dataset/san_francisco/direction/train_im_list.txt"
    root_folder: "/home/samarth/research/deepnav/data/dataset/"
    shuffle: false
    new_height: 256 
    new_width: 256 
    batch_size: 1
  }
}

layer {
  name: "ho"
  type: "HDF5Output"
  bottom: "data"
  bottom: "label"
  hdf5_output_param {
    file_name: "test_images.h5"
  }
}

name: "DeepNavAlexNet_test"

input: "data"
input_dim: 1
input_dim: 3
input_dim: 227 
input_dim: 227 

layer {
  name: "d1"
  type: "DummyData"
  top: "label"
  dummy_data_param {
    shape: {dim: 1 dim: 1 dim: 1 dim: 1}
  }
}

layer {
  name: "ho"
  type: "HDF5Output"
  bottom: "data"
  bottom: "label"
  hdf5_output_param {
    file_name: "test_images_d.h5"
  }
}

layer {
  name: "d1"
  type: "DummyData"
  top: "pred"
  dummy_data_param {
    shape: {dim: 1 dim: 1 dim: 1 dim: 1}
  }
}

net: "test_images.prototxt"
solver_mode: GPU 

base_lr: 1e-3
lr_policy: "fixed"
max_iter: 1

momentum: 0.9 
weight_decay: 0.0005
*/
