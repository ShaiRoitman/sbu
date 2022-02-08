import SBUGlobals
import os
import json
import sys

def LoadConfiguration():
    config_file = os.environ['SBUWEBAPP_CONFIG']
    if (config_file is None):
        print ("No SBUWEBAPP_CONFIG environment variable defined")
        sys.exit(1)

    with open(config_file) as json_file:
        SBUGlobals.configuration = json.load(json_file)
        print(SBUGlobals.configuration)