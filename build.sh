#!/bin/bash

docker run --rm -it -v $PWD:/sbu compile_sbu:$COMPILE_SBU_VERSION ./linuxBuild/compile.sh
