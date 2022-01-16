from fastapi import FastAPI
from typing import List, Optional
from datetime import datetime

import uvicorn

from SbuDataClasses import RepositoryBackupDef, RepositoryFile, RepositoryBackup, FileRepositoryFile, BackupInfoModel

app = FastAPI()

import sqlite3

@app.get("/repository/backupdefs",
         tags=["Repository"],
         response_model=List[RepositoryBackupDef])
async def get_backupdefs():
    retValue: List[RepositoryBackupDef] = []

    query = "SELECT ID, Name, Hostname, RootPath, Added FROM BackupDefs";
    con = sqlite3.connect("c:\\work\\RepositoryDB.db")
    queryResult = con.execute(query);
    for result in queryResult.fetchall():
        value = RepositoryBackupDef()
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
          response_model=List[RepositoryBackupDef])
async def create_repository_backupdef(backupDef: RepositoryBackupDef):
    retValue: List[RepositoryBackupDef] = []

    return retValue


@app.get("/repository/backups",
         response_model=List[RepositoryBackup],
         tags=["Repository"],
         description="Get Backups Already done")
async def get_repository_backups(backupdefID: Optional[int] = None,
                                 before: Optional[datetime] = None,
                                 after: Optional[datetime] = None):
    retValue = List[RepositoryBackup]

    afterTS: datetime = datetime.now()
    beforeTS: datetime = datetime.now()

    WHERE_CLAUSE = f"WHERE Started >= {afterTS} and Started <= {beforeTS}"
    if (backupdefID):
        WHERE_CLAUSE = WHERE_CLAUSE + f" AND backupDefID = {backupdefID}"

    query = f"SELECT ID, BackupDefID, Started, LastStatusUpdate, Status FROM Backups {WHERE_CLAUSE}"

    con = sqlite3.connect("c:\\work\\RepositoryDB.db")
    queryResult = con.execute(query)
    for result in queryResult.fetchall():
        value = RepositoryBackupDef()
        value.id = result[0]
        value.name = result[1]
        value.hostname = result[2]
        value.rootPath = result[3]
        value.addedDate = result[4]

        retValue.append(value)

    con.close()
    return retValue


@app.get("/repository/files",
         tags=["Repository"],
         response_model=List[RepositoryFile])
async def get_repository_files(backupdefID: Optional[int] = None,
                               before: Optional[datetime] = None,
                               after: Optional[datetime] = None):
    retValue = List[RepositoryFile]
    # sbu execute
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
