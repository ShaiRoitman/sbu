#!/bin/bash

./ResourceBuild.sh
cmake -DCMAKE_PREFIX_PATH=/sbu/sbu/cmake/linux -DSBU_PLATFROM=linux . 
make -j 6
