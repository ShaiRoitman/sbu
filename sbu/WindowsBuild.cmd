@echo off

call ResourceBuild.cmd
cmake  --debug-output -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DSBU_PLATFROM=windows -G "Visual Studio 15 2017 Win64" .
cmake --build .
