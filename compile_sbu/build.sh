#!/bin/bash

COMPILE_SBU_VERSION=`cat version.txt`
docker build -t compile_sbu:$COMPILE_SBU_VERSION . 

