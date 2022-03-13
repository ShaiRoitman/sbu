#!/bin/bash

echo "Full Build"

COMPILE_SBU_VERSION=`cat compile_sbu/version.txt`
docker build -t compile_sbu:$COMPILE_SBU_VERSION -f compile_sbu/Dockerfile compile_sbu

docker run --rm -it -v $PWD:/sbu sbu_compile ./linuxBuild/compile.sh

cp -r sbu/sbu_app/so sbuApp/so
cp sbu/SbuCli/sbu sbuApp

WEB_SBU_VERSION=`cat webservice_sbu/version.txt`
docker build -t web_sbu:$WEB_SBU_VERSION -f webservice_sbu/Dockerfile webservice_sbu

mkdir sbuApp/SBUWebApp
cp -r SBUWebApp/*.py sbuApp/SBUWebApp
cp SBUWebApp/requirements.txt sbuApp/SBUWebApp/requirements.txt
cp SBUWebApp/SBUWebApp.pyproj sbuApp/SBUWebApp/SBUWebApp.pyproj

SBU_VERSION=`cat sbuApp/version.txt`
docker build -t sbu_run:$SBU_VERSION -f sbuApp/Dockerfile sbuApp
