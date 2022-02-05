from pydantic.dataclasses import dataclass
from enum import Enum
from typing import Optional
from datetime import datetime


@dataclass
class RepositoryBackupDef:
    id: int
    name: str
    hostname: str
    rootPath: str
    addedDate: Optional[datetime]

    def __init__(self):
        self.id = 0
        self.name = ""
        self.hostname = ""
        self.rootPath = ""
        self.addedDate = datetime.now()


@dataclass
class RepositoryBackup:
    id: int
    backupDef: int
    start: Optional[datetime]
    lastStatusUpdate: Optional[datetime]
    Status: str

    def __init__(self):
        self.id = 0
        self.backupDef = 0
        self.start = None
        self.lastStatusUpdate = None
        self.Status = ""


class RepositoryFileStatus(Enum):
    CREATED = 1
    UPDATED = 3
    DELETED = 2


@dataclass
class RepositoryFile:
    id: int
    backupID: int
    path: str
    size: int
    created: Optional[datetime]
    updated: Optional[datetime]
    accessed: Optional[datetime]
    digestType: str
    digestValue: str
    fileHandle: str
    status: Optional[RepositoryFileStatus]


@dataclass
class FileRepositoryFile:
    id: int
    path: str
    size: int
    added: Optional[datetime]
    digestType: str
    digestValue: str
    multiFileHostDigest: str


@dataclass
class BackupInfoModel:
    id: int
    backupDefId: int
    created: Optional[datetime]
    lastAction: str
    scanComplete: Optional[datetime]
    diffComplete: Optional[datetime]
    fileUploadComplete: Optional[datetime]
    finalizationComplete: Optional[datetime]
