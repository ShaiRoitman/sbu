#!/bin/bash

pushd compile_sbu
./build.sh
popd

pushd webservice_venv
./build.sh
popd
