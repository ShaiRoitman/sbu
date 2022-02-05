#!/usr/bin/env bash

export SBU_CONFIG_PATH=/sbu/Configuration

if test -f "$SBU_CONFIG_PATH/SBUWebAppConfig.json"; then
  echo "$SBU_CONFIG_PATH/SBUWebAppConfig.json exists."
else
  echo cp /sbu/App/SBUWebApp/DefaultSBUWebAppConfig.json $SBU_CONFIG_PATH/SBUWebAppConfig.json
  cp /sbu/App/SBUWebApp/DefaultSBUWebAppConfig.json $SBU_CONFIG_PATH/SBUWebAppConfig.json
fi

if test -f "$SBU_CONFIG_PATH/SBUApp.config"; then
  echo "$SBU_CONFIG_PATH/SBUApp.config exists."
else
  echo /sbu/App/sbu/DefaultSBUApp.config.json $SBU_CONFIG_PATH/SBUApp.config
  cp /sbu/App/sbu/DefaultSBUApp.config.json $SBU_CONFIG_PATH/SBUApp.config
fi
