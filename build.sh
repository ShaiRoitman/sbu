#!/bin/bash

docker run --rm -it -v $PWD:/sbu compile_sbu:$COMPILE_SBU_VERSION /sbu/sbu_app/build/compile.sh

mkdir sbu/sbu_app
mkdir sbu/SBUWebApp

cp -r sbu_app/sbu_app/so sbu/sbu_app
cp sbu_app/SbuCli/sbu sbu/sbu_app
cp -r SBUWebApp/* sbu/SBUWebApp

SBU_VERSION=`cat sbu/version.txt`
docker build -t sbu_run:$SBU_VERSION -f sbu/Dockerfile sbu

