#ifndef CAFFE_WRAPPER_H
#define CAFFE_WRAPPER_H

#include <caffe/caffe.hpp>
#include <opencv2/opencv.hpp>

class CaffeWrapper {
  public:
    CaffeWrapper(std::string proto_file, std::string model_file,
        std::string mean_file);
    std::vector<float> get_pred(cv::Mat const &im);
  private:
    void set_mean(std::string const &mean_file);
    void wrap_input_layer(std::vector<cv::Mat> *input_channels);
    void preprocess(cv::Mat const &im, std::vector<cv::Mat> *input_channels);
    boost::shared_ptr<caffe::Net<float> > net;
    cv::Size input_size;
    cv::Mat mean;
    int n_channels;
};

#endif
