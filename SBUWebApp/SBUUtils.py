import SBUGlobals
import os
import json
import sys
import logging

def LoadConfiguration():
    config_file = os.environ['SBUWEBAPP_CONFIG']
    if (config_file is None):
        print ("No SBUWEBAPP_CONFIG environment variable defined")
        sys.exit(1)

    with open(config_file) as json_file:
        SBUGlobals.configuration = json.load(json_file)

def InitLogger():
    logConfig = SBUGlobals.configuration["Logging"]

    FORMAT = '%(asctime)s %(levelname)s %(name)s : %(message)s'
    logging.root.setLevel(logConfig["Verbosity"])
    logging.basicConfig(filename= logConfig["LogFile"],
                        format=FORMAT,
                        )

    logging.getLogger("").info("Starting SBUWebApp")

def InitApp():
    LoadConfiguration()
    InitLogger()
