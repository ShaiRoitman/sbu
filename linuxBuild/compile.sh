#!/bin/bash

cp /sbu/linuxBuild/SQLiteCppConfig.cmake /usr/local/lib/cmake/SQLiteCpp/SQLiteCppConfig.cmake
cd /sbu/sbu
./LinuxBuild.sh
mkdir sbu_app
mkdir sbu_app/so
ldd SbuCli/sbu | grep "=> /" | awk '{print $3}' | xargs -I '{}' cp -v '{}' sbu_app/so
pushd sbu_app/so
strip *
popd
strip sbu
 
