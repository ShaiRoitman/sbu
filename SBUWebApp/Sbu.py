import SBUGlobals
import os
import subprocess
import datetime
from SbuDataClasses import ExecutionResult
from typing import Dict, Optional


class SbuApp:
    def __init__(self):
        paths = SBUGlobals.configuration["paths"]
        self.executablePath = paths["SBUApp"]
        self.configPath = paths["SBUAppConfig"]
        self.LogsPath = paths["Logs"]
        self.restoreBase = paths["RestorePath"]
        self.args: Dict[str, str] = {}
        self.args["--config"] = self.configPath

    def createBasePath(self, backupDefName: str) -> {str}:
        now = datetime.datetime.now()
        timeStamp = now.strftime("%Y%m%d_%H%M%S")
        month = now.strftime("%Y%m")
        os.makedirs(os.path.join(self.LogsPath, f"{backupDefName}", f"{month}"), exist_ok=True)
        baseFilePath = os.path.join(self.LogsPath, f"{backupDefName}", f"{month}", f"{backupDefName}_{timeStamp}")

        return baseFilePath

    def executeBackup(self,
                      backupDefName: str,
                      ) -> ExecutionResult:
        args = self.args.copy()
        baseFilePath = self.createBasePath(backupDefName)

        args["--action"] = "Backup"
        args["--name"] = backupDefName
        args["--Logging.FileOutput"] = f"{baseFilePath}_Backup.log"
        args["--path"] = f"{baseFilePath}_BackupDB.db"

        return self.execute(args, baseFilePath)

    def executeRestore(self,
                       backupDefName: str,
                       destination: str,
                       restoreTimeStamp: Optional[datetime.datetime]
                       ) -> ExecutionResult:
        args = self.args.copy()
        baseFilePath = self.createBasePath(backupDefName)
        args["--action"] = "Restore"
        args["--name"] = backupDefName
        args["--Logging.FileOutput"] = f"{baseFilePath}_Restore.log"
        args["--path"] = destination
        if restoreTimeStamp is not None:
            args["--date"] = str(restoreTimeStamp)

        return self.execute(args, baseFilePath)

    def execute(self, args: Dict[str, str], output: str) -> ExecutionResult:

        cmdLine = []
        cmdLine.append(self.executablePath)
        for key in args:
            cmdLine.append(key)
            cmdLine.append(args[key])

        startTime = datetime.datetime.now()
        outputFileName = f"{output}.output"
        with open(outputFileName, "w") as log:
            process = subprocess.Popen(cmdLine, stdout=log)

        errorCode = process.wait()
        with open(outputFileName, 'r') as file:
            data = file.read()
        endTime = datetime.datetime.now()
        with open(f"{output}.summary", "w") as log:
            sbuConfig = ""
            if "SBU_CONFIG" in os.environ:
                sbuConfig = os.environ["SBU_CONFIG"]
            log.write(f"SBU_CONFIG : {sbuConfig}\n"
                      f"arguments : {cmdLine}\n"
                      f"StartTime : {startTime}\n"
                      f"endTime : {endTime}\n"
                      f"runTime : {endTime - startTime}\n"
                      f"errorCode : {errorCode}\n")

        retValue: ExecutionResult = ExecutionResult(
            startTime= startTime,
            endTime = endTime,
            errorCode = errorCode,
            arguments = str(cmdLine),
            output = data

        )

        return retValue
