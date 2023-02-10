#!/bin/bash
if [ $# -eq 0 ];
then
    emcmake cmake .
elif [ $# -eq 1 ];
then
  echo "$0: Must supply both width and height"
  exit 1
elif [ $# -gt 2 ];
then
    echo "$0: Too many arguments: $@"
    exit 1
else
    emcmake cmake . -DSCREEN_WIDTH=$1 -DSCREEN_HEIGHT=$2
fi

emmake make