from fastapi import FastAPI
from typing import List, Optional
from datetime import datetime

from SBUOperations import backup, restore, repository_files, get_backupdefs, create_repository_backupdef, repository_backups
from SbuDataClasses import RepositoryBackupDef, RepositoryFile, RepositoryBackup, FileRepositoryFile, BackupInfoModel

app = FastAPI()

@app.get("/repository/backupdefs",
         tags=["Repository"],
         response_model=List[RepositoryBackupDef])
async def get_backupdefs():
    return get_backupdefs()


@app.post("/repository/backupdefs",
          tags=["Repository"],
          response_model=RepositoryBackupDef)
async def post_create_repository_backupdef(backupDef: RepositoryBackupDef):
    return create_repository_backupdef(backupDef)


@app.get("/repository/backups",
         response_model=List[RepositoryBackup],
         tags=["Repository"],
         description="Get Backups Already done")
async def get_repository_backups(backupdefID: Optional[int] = None,
                                 before: Optional[datetime] = None,
                                 after: Optional[datetime] = None):
    return repository_backups(backupdefID, before, after)


@app.get("/repository/files",
         tags=["Repository"],
         response_model=List[RepositoryFile])
async def get_repository_files(backupdefName: str,
                               timestamp: Optional[datetime] = None):
    return repository_files(backupdefName, timestamp)


@app.post("/operations/backup",
          tags=["Operations"],
          response_model=BackupInfoModel)
async def operate_backup(backupdefName: str):
    return backup(backupdefName)


@app.post("/operations/restore",
          tags=["Operations"],
          response_model=List[RepositoryFile])
async def operate_restore(backupdefName: str,
                          destinationPath: str,
                          timestamp: Optional[datetime] = None):
    return restore(backupdefName, destinationPath, timestamp)
