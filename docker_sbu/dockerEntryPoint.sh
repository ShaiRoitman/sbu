#!/usr/bin/env bash

export SBU_CONFIG_PATH=/sbu/Configuration
export SBU_CONFIG=$SBU_CONFIG_PATH/SBUApp.config
export SBUWEBAPP_CONFIG=$SBU_CONFIG_PATH/SBUWebAppConfig.json

if test -f "$SBUWEBAPP_CONFIG"; then
  echo "$SBUWEBAPP_CONFIG exists."
else
  echo cp /sbu/App/SBUWebApp/DefaultSBUWebAppConfig.json $SBUWEBAPP_CONFIG
  cp /sbu/App/SBUWebApp/DefaultSBUWebAppConfig.json $SBUWEBAPP_CONFIG
fi

if test -f "$SBU_CONFIG"; then
  echo "$SBU_CONFIG exists."
else
  echo /sbu/App/sbu/DefaultSBUApp.config $SBU_CONFIG
  cp /sbu/App/sbu/DefaultSBUApp.config $SBU_CONFIG
fi

python3 /sbu/App/SBUWebApp/SBUWebApp.py
