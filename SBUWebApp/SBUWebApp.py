from fastapi import FastAPI
from typing import List, Optional
from datetime import datetime
from SQLHelper import WhereClause
import sqlite3
import uvicorn
from Sbu import SbuApp
import json

from SbuDataClasses import RepositoryBackupDef, RepositoryFile, RepositoryBackup, FileRepositoryFile, BackupInfoModel

app = FastAPI()


@app.get("/repository/backupdefs",
         tags=["Repository"],
         response_model=List[RepositoryBackupDef])
async def get_backupdefs():
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


@app.post("/repository/backupdefs",
          tags=["Repository"],
          response_model=RepositoryBackupDef)
async def create_repository_backupdef(backupDef: RepositoryBackupDef):
    retValue: RepositoryBackupDef = RepositoryBackupDef()

    app: SbuApp = SbuApp()

    cmdLine = f"--action CreateBackupDef --name {backupDef.name} --path {backupDef.rootPath}"
    result = app.execute(cmdLine)
    if (result[0] == 0):
        result_splitted = str(result[1]).split(",")

        retValue.id = result_splitted[0]
        retValue.name = result_splitted[1]
        retValue.hostname = result_splitted[2]
        retValue.rootPath = result_splitted[3]
        retValue.addedDate = result_splitted[4]

    return retValue


@app.get("/repository/backups",
         response_model=List[RepositoryBackup],
         tags=["Repository"],
         description="Get Backups Already done")
async def get_repository_backups(backupdefID: Optional[int] = None,
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


@app.get("/repository/files",
         tags=["Repository"],
         response_model=List[RepositoryFile])
async def get_repository_files(backupdefName: str,
                               timestamp: Optional[datetime] = None):
    retValue:  List[RepositoryFile] = []

    app: SbuApp = SbuApp()

    cmdLine = f"--action Restore --name {backupdefName} --path {timestamp}"
    result = app.execute(cmdLine)
    if (result[0] == 0):
        result_splitted = str(result[1]).split(",")

        retValue.id = result_splitted[0]
        retValue.name = result_splitted[1]
        retValue.hostname = result_splitted[2]
        retValue.rootPath = result_splitted[3]
        retValue.addedDate = result_splitted[4]

    return retValue


@app.get("/filerepository/files",
         tags=["FileRepository"],
         response_model=List[FileRepositoryFile])
async def get_files(backupdefID: Optional[int] = None,
                    addedBefore: Optional[datetime] = None,
                    addedAfter: Optional[datetime] = None,
                    sizeBigger: Optional[int] = None,
                    sizeSmaller: Optional[int] = None):
    retValue: List[FileRepositoryFile] = []
    # SQL Query
    return retValue


@app.get("/filerepository/files/{fileId}",
         tags=["FileRepository"])
async def get_file(fileId: str):
    retValue = None

    # SQL Query and file download

    return retValue


@app.post("/operations/backup",
          tags=["Operations"],
          response_model=BackupInfoModel)
async def operate_backup(backupDefId: int):
    retValue: BackupInfoModel = BackupInfoModel()

    # SQL Query

    return retValue


@app.get("/operations/backup/{backupId}",
         tags=["Operations"],
         response_model=BackupInfoModel)
async def get_backup_info(backupId: int):
    retValue: BackupInfoModel = BackupInfoModel()

    # sbu shell

    return retValue


@app.post("/operations/restore",
          tags=["Operations"],
          response_model=List[RepositoryFile])
async def operate_restore(backupDefId: int, destinationPath: str):
    retValue: List[RepositoryFile] = []

    # sbu shell

    return retValue


if __name__ == "__main__":
    uvicorn.run("SBUWebApp:app", host="127.0.0.1", port=5000, log_level="info")
