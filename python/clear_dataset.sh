#!/bin/bash
if [[ $# -ne 1 ]]
  then
    echo "Usage: ./clear_dataset.sh path-to-city"
    exit -1
fi
IM_DIR=$1/images/*.jpg
LINK_DIR=$1/links/*.txt
set -v
rm -r $1/nodes/*
rm $1/state.pk
for f in $IM_DIR; do echo "$f"; rm "$f"; done
for f in $LINK_DIR; do echo "$f"; rm "$f"; done
set +v
