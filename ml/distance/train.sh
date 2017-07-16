#!/bin/bash
if [[ $# -ne 1 ]]
then
  echo "Usage: ./train.sh city"
  exit -1
fi
DEEPNAV_CITY=$1

sed "$(eval echo $(cat sub.sed))" train_val.prototxt > train_val_${DEEPNAV_CITY}.prototxt
sed "$(eval echo $(cat sub.sed))" solver.prototxt > solver_${DEEPNAV_CITY}.prototxt

caffe train -gpu 0 -solver solver_${DEEPNAV_CITY}.prototxt -weights ../../data/VGG_ILSVRC_16_layers.caffemodel  2>&1 | tee log/${DEEPNAV_CITY}_vgg_log.log
# caffe train -gpu 0 -solver solver_${DEEPNAV_CITY}.prototxt -snapshot ../../output/streetview/combine_vgg_distance_iter_139320.solverstate 2>&1 | tee -a log/${DEEPNAV_CITY}_vgg_log.log
