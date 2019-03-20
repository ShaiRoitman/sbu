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
    Name     TEXT UNIQUE,
    Hostname TEXT,
    RootPath TEXT,
    Added    DATETIME
);


-- Table: Backups
CREATE TABLE Backups (
    ID          INTEGER  PRIMARY KEY AUTOINCREMENT,
    BackupDefID INTEGER  REFERENCES BackupDefs (ID),
    Started     DATETIME,
    LastStatusUpdate DATETIME,
    Status      TEXT
);


-- Table: Files
CREATE TABLE Files (
    ID			INTEGER  PRIMARY KEY AUTOINCREMENT,
    BackupID	INTEGER  REFERENCES Backups (ID),
    Path		TEXT,
	Type        TEXT,
	Size		BIGINT,
    Created		DATETIME,
    Modified	DATETIME,
    Accessed	DATETIME,
	DigestType	TEXT,
	DigestValue TEXT,
	FileHandle	TEXT,
    Status		TEXT
);

CREATE VIEW LatestFiles AS
    SELECT Files.Path,
           Files.Size,
           Files.Type,
           Files.Created,
           Files.Modified,
           Files.Accessed,
           Files.Status,
           Files.FileHandle,
           backupDefID,
           Started,
           backupID
      FROM (
               SELECT MAX(Files.ID) AS FILEID,
                      BackupDefs.ID AS backupDefID,
                      Backups.Started AS Started,
                      Backups.ID AS backupID
                 FROM Files
                      JOIN
                      Backups ON Files.BackupID = Backups.ID
                      JOIN
                      BackupDefs ON BackupDefID = BackupDefs.ID
                WHERE Backups.Status = 'Complete'
                GROUP BY Files.Path
           )
           Latest
           JOIN
           Files ON Latest.FILEID = Files.ID;


COMMIT TRANSACTION;
PRAGMA foreign_keys = on;
