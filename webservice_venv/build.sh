#!/bin/bash

WEBSERVICE_VENV_VERSION=`cat version.txt`
docker build -t webservice_venv:$WEBSERVICE_VENV_VERSION . 
docker tag webservice_venv:$WEBSERVICE_VENV_VERSION webservice_venv
