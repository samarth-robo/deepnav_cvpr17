#include <caffe_wrapper.h>

using namespace std;
using namespace cv;
using namespace boost;
using namespace caffe;

CaffeWrapper::CaffeWrapper(string proto_file, string model_file,
    string mean_file) {
  Caffe::set_mode(Caffe::GPU);
  Caffe::SetDevice(0);
  
  // load net
  net.reset(new Net<float>(proto_file, TEST));
  net->CopyTrainedLayersFrom(model_file);
  
  // get net input size
  Blob<float>* input_layer = net->input_blobs()[0];
  input_size = Size(input_layer->width(), input_layer->height());
  n_channels = input_layer->channels();

  // load mean file
  set_mean(mean_file);
}

// Load the mean file in binaryproto format
void CaffeWrapper::set_mean(const string& mean_file) {
  BlobProto blob_proto;
  ReadProtoFromBinaryFileOrDie(mean_file.c_str(), &blob_proto);

  // Convert from BlobProto to Blob<float>
  Blob<float> mean_blob;
  mean_blob.FromProto(blob_proto);
  if(mean_blob.channels() != n_channels) {
    cout << "Number of channels of mean file doesn't match input layer." << endl;
    return;
  }

  // The format of the mean file is planar 32-bit float BGR or grayscale
  vector<Mat> channels;
  float* data = mean_blob.mutable_cpu_data();
  for(int i = 0; i < n_channels; i++) {
    // Extract an individual channel
    Mat channel(mean_blob.height(), mean_blob.width(), CV_32FC1, data);
    channels.push_back(channel);
    data += mean_blob.height() * mean_blob.width();
  }

  // Merge the separate channels into a single image
  cv::merge(channels, this->mean);
}

vector<float> CaffeWrapper::get_pred(const Mat &im) {
  Blob<float>* input_layer = net->input_blobs()[0];
  input_layer->Reshape(1, n_channels,
                       input_size.height, input_size.width);
  // Forward dimension change to all layers
  net->Reshape();

  vector<Mat> input_channels;
  wrap_input_layer(&input_channels);
  preprocess(im, &input_channels);

  net->ForwardFrom(0);

  // Copy the output layer to a std::vector
  Blob<float>* output_layer = net->output_blobs()[0];
  const float* begin = output_layer->cpu_data();
  const float* end = begin + output_layer->count();
  return std::vector<float>(begin, end);
}

void CaffeWrapper::wrap_input_layer(vector<Mat>* input_channels) {
  Blob<float>* input_layer = net->input_blobs()[0];

  int width = input_layer->width();
  int height = input_layer->height();
  float* input_data = input_layer->mutable_cpu_data();
  for(int i = 0; i < input_layer->channels(); i++) {
    Mat channel(height, width, CV_32FC1, input_data);
    input_channels->push_back(channel);
    input_data += width * height;
  }
}

void CaffeWrapper::preprocess(Mat const &im, vector<Mat>* input_channels) {
  // Convert the input image to the input image format of the network
  Mat im_resized;
  if(im.size() != this->mean.size()) resize(im, im_resized, this->mean.size());
  else im_resized = im;

  Mat im_float;
  im_resized.convertTo(im_float, CV_32FC3);

  Mat im_normalized;
  subtract(im_float, this->mean, im_normalized);

  // offsets for cropping (center crop)
  size_t h_off = (im_normalized.rows - input_size.height) / 2;
  size_t w_off = (im_normalized.cols - input_size.width) / 2;
  Rect roi(w_off, h_off, input_size.width, input_size.height);
  Mat im_cropped = im_normalized(roi);

  // This operation will write the separate BGR planes directly to the
  // input layer of the network because it is wrapped by the cv::Mat
  // objects in input_channels
  cv::split(im_cropped, *input_channels);

  if(reinterpret_cast<float*>(input_channels->at(0).data) !=
      net->input_blobs()[0]->cpu_data()) {
    cout << "Input channels are not wrapping the input layer of the network."
         << endl;
  }
}
