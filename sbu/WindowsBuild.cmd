@echo off

call ResourceBuild.cmd
cmake  --debug-output -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -G "Visual Studio 15 2017 Win64" .
cmake --build .
