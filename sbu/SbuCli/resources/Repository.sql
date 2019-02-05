--
-- File generated with SQLiteStudio v3.2.1 on Fri Jan 18 20:11:11 2019
--
-- Text encoding used: System
--
PRAGMA foreign_keys = off;
BEGIN TRANSACTION;

CREATE TABLE ExecutionLog (
	EventTime		DATETIME,
	Comment			TEXT,
	Argument		TEXT
);

-- Table: BackupDefs
CREATE TABLE BackupDefs (
    ID       INTEGER  PRIMARY KEY AUTOINCREMENT,
    Name     TEXT,
    Hostname TEXT,
    RootPath TEXT,
    Added    DATETIME
);


-- Table: Backups
CREATE TABLE Backups (
    ID          INTEGER  PRIMARY KEY AUTOINCREMENT,
    BackupDefID INTEGER  REFERENCES BackupDefs (ID),
    Started     DATETIME,
    Ended       DATETIME,
    Status      TEXT
);


-- Table: Files
CREATE TABLE Files (
    ID			INTEGER  PRIMARY KEY AUTOINCREMENT,
    BackupID	INTEGER  REFERENCES Backups (ID),
    Path		TEXT,
	Size		BIGINT,
    Created		DATETIME,
    Modified	DATETIME,
    Accessed	DATETIME,
	DigestType	TEXT,
	DigestValue TEXT,
	FileHandle	TEXT,
    Status		TEXT
);


COMMIT TRANSACTION;
PRAGMA foreign_keys = on;
