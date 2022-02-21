#!/usr/bin/env bash

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
