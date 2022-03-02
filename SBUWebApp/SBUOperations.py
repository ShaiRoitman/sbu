from SbuDataClasses import RepositoryBackupDef, RepositoryFile, RepositoryBackup, FileRepositoryFile, BackupInfoModel , ExecutionResult, RestoreExecuteResult
from Sbu import SbuApp
from typing import List, Optional
from datetime import datetime
from SQLHelper import WhereClause
import sqlite3


def backup(backupdefName: str) -> ExecutionResult:
    sbu_app: SbuApp = SbuApp()
    retValue: ExecutionResult = sbu_app.executeBackup(backupdefName)
    return retValue

def restore(backupdefName: str,
                          destinationPath: str,
                          timestamp: Optional[datetime] = None) -> RestoreExecuteResult:
    sbu_app: SbuApp = SbuApp()
    result: ExecutionResult = sbu_app.executeRestore(backupdefName, destinationPath, timestamp)
    retValue : RestoreExecuteResult = RestoreExecuteResult()
    retValue.startTime = result.startTime
    retValue.endTime = result.endTime
    retValue.errorCode = result.errorCode
    retValue.arguments = result.arguments
    for file in result.output.split("\n"):
        retValue.files.append(file)
    retValue.output = ""

    return retValue

def repository_files(backupdefName: str,
                               timestamp: Optional[datetime] = None):
    retValue:  List[RepositoryFile] = []

    sbu_app: SbuApp = SbuApp()

    if (timestamp):
        timestamp = datetime.now()

    cmdLine = f"--action Restore --name {backupdefName} --date {timestamp} --showOnly"
    result = sbu_app.execute(cmdLine)
    if (result[0] == 0):
        result_splitted = str(result[1]).split(",")

        retValue.id = result_splitted[0]
        retValue.name = result_splitted[1]
        retValue.hostname = result_splitted[2]
        retValue.rootPath = result_splitted[3]
        retValue.addedDate = result_splitted[4]

    return retValue

def get_backupdefs():
    retValue: List[RepositoryBackupDef] = []

    query = "SELECT ID, Name, Hostname, RootPath, Added FROM BackupDefs"
    con = sqlite3.connect("c:\\work\\RepositoryDB.db")
    queryResult = con.execute(query)
    for result in queryResult.fetchall():
        value: RepositoryBackupDef = RepositoryBackupDef()
        value.id = result[0]
        value.name = result[1]
        value.hostname = result[2]
        value.rootPath = result[3]
        value.addedDate = result[4]

        retValue.append(value)

    con.close()

    return retValue

def create_repository_backupdef(backupDef: RepositoryBackupDef):
    retValue: RepositoryBackupDef = RepositoryBackupDef()

    sbu_app: SbuApp = SbuApp()

    cmdLine = f"--action CreateBackupDef --name {backupDef.name} --path {backupDef.rootPath}"
    result = sbu_app.execute(cmdLine)
    if (result[0] == 0):
        result_splitted = str(result[1]).split(",")

        retValue.id = result_splitted[0]
        retValue.name = result_splitted[1]
        retValue.hostname = result_splitted[2]
        retValue.rootPath = result_splitted[3]
        retValue.addedDate = result_splitted[4]

    return retValue

async def repository_backups(backupdefID: Optional[int] = None,
                                 before: Optional[datetime] = None,
                                 after: Optional[datetime] = None):
    retValue: List[RepositoryBackup] = []
    where: WhereClause = WhereClause()

    where.add_op_int("BackupDefID", "=", backupdefID)
    where.add_op_date("Started", ">=", after)
    where.add_op_date("Started", "<=", before)

    query = f"SELECT ID, BackupDefID, Started, LastStatusUpdate, Status FROM Backups {where.Query()}"

    con = sqlite3.connect("c:\\work\\RepositoryDB.db")
    queryResult = con.execute(query)
    for result in queryResult.fetchall():
        value: RepositoryBackup = RepositoryBackup()
        value.id = result[0]
        value.backupDef = result[1]
        value.start = result[2]
        value.lastStatusUpdate = result[3]
        value.Status = result[4]

        retValue.append(value)

    con.close()
    return retValue
