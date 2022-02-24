import SBUGlobals
import os
import subprocess
from typing import List
import datetime

class SbuApp:
    def __init__(self):
        paths = SBUGlobals.configuration["paths"]
        self.executablePath = paths["SBUApp"]
        self.configPath = paths["SBUAppConfig"]
        self.LogsPath = paths["Logs"]
        self.restoreBase = paths["RestorePath"]

        self.cmdLine = [
            self.executablePath,
            "--config", self.configPath,
        ]

    def executeBackup(self,
                      backupDefName: str,
                      ) -> {int, str}:
        args = self.cmdLine
        timeStamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")

        args.append( "--action")
        args.append( "Backup" )
        args.append( "--name")
        args.append( backupDefName )
        args.append( "--Logging.FileOutput")
        args.append( os.path.join(self.LogsPath, f"{backupDefName}_{timeStamp}.log") )
        args.append( "--BackupDB.path" )
        args.append( os.path.join("c:\\sbu\\work\\Backup.db") )

        return self.execute( args )

    def execute(self, cmdLine: List[str]) -> {int, str}:
        print (cmdLine)
        process = subprocess.Popen(cmdLine)
        retValue = [process.wait(), ""] #process.stdout.readlines()
        return retValue
