#!/usr/bin/env bash

export SBU_CONFIG_PATH=/sbu/Configuration
export SBU_CONFIG=$SBU_CONFIG_PATH/SBUApp.config

if test -f "$SBU_CONFIG_PATH/SBUWebAppConfig.json"; then
  echo "$SBU_CONFIG_PATH/SBUWebAppConfig.json exists."
else
  echo cp /sbu/App/SBUWebApp/DefaultSBUWebAppConfig.json $SBU_CONFIG_PATH/SBUWebAppConfig.json
  cp /sbu/App/SBUWebApp/DefaultSBUWebAppConfig.json $SBU_CONFIG_PATH/SBUWebAppConfig.json
fi

if test -f "$SBU_CONFIG_PATH/SBUApp.config"; then
  echo "$SBU_CONFIG_PATH/SBUApp.config exists."
else
  echo /sbu/App/sbu/DefaultSBUApp.config $SBU_CONFIG
  cp /sbu/App/sbu/DefaultSBUApp.config $SBU_CONFIG
fi

/bin/bash