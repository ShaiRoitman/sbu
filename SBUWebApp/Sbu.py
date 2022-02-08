import SBUGlobals
import os
import subprocess

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
        timeStamp = "20220208_100000"

        args.append( [" --action", "Backup"] )
        args.append( [" --name", backupDefName] )
        args.append( [" --Logging.FileOutput", os.path.join(self.LogsPath, backupDefName, timeStamp)] )

        return self.execute( args )

    def execute(self, cmdLine: list[str]) -> {int, str}:
        process = subprocess.Popen(cmdLine)
        retValue = [process.wait(), process.stdout.readlines()]
        return retValue
