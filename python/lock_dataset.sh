#!/bin/bash
if [[ $# -ne 1 ]]
  then
    echo "Usage: ./lock_dataset.sh path-to-city"
    exit -1
fi
IM_DIR=$1/images/*.jpg
LINK_DIR=$1/links/*.txt
set -v
chmod -R -w $1/nodes/
chmod -w $1/state.pk
for f in $IM_DIR; do echo "$f"; chmod -w "$f"; done
for f in $LINK_DIR; do echo "$f"; chmod -w "$f"; done
set +v
