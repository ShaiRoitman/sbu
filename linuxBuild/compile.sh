#!/bin/bash

cp /sbu/linuxBuild/SQLiteCppConfig.cmake /usr/local/lib/cmake/SQLiteCpp/SQLiteCppConfig.cmake
cd /sbu/sbu
./LinuxBuild.sh
mkdir sbu_app
ldd SbuCli/sbu | grep "=> /" | awk '{print $3}' | xargs -I '{}' cp -v '{}' sbu_app/
cp SbuCli/sbu sbu_app
cd sbu_app
strip *



 
