--
-- File generated with SQLiteStudio v3.2.1 on Sun Jan 20 06:04:26 2019
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

CREATE TABLE GeneralInfo (
	RootPath		TEXT,
	Created			DATETIME
);


-- Table: Entries
CREATE TABLE Entries (
    ID              INTEGER  PRIMARY KEY,
    Path            TEXT,
    Type            TEXT,
    Added           DATETIME,
    Size            BIGINT,
    Created         DATETIME,
    Modified        DATETIME,
    Accessed        DATETIME,
    DigestType      TEXT,
    DigestValue     TEXT,
    StartDigestCalc DATETIME,
    EndDigestCalc   DATETIME,
	FileHandle      TEXT,
	StartUpload     DATETIME,
	EndUpload       DATETIME,
	Status          TEXT
);

CREATE TABLE CurrentState (
    ID              INTEGER  PRIMARY KEY,
    Path            TEXT,
	Type            TEXT,
    Size            BIGINT,
    Created         DATETIME,
    Modified        DATETIME,
    Accessed        DATETIME,
    DigestType      TEXT,
    DigestValue     TEXT,
	Status          TEXT,
	UploadState     TEXT,
	FileHandle      TEXT
);

CREATE TABLE NextState (
    Path            TEXT PRIMARY KEY,
	Type            TEXT,
    Size            BIGINT,
    Created         DATETIME,
    Modified        DATETIME,
    Accessed        DATETIME,
    DigestType      TEXT,
    DigestValue     TEXT,
	Status          TEXT,
	UploadState     TEXT,
	FileHandle      TEXT
);

-- Table: Scan
CREATE TABLE Scan (
    ID        INTEGER  PRIMARY KEY AUTOINCREMENT,
    Path      TEXT,
    Added     DATETIME,
    Started   DATETIME,
    Completed DATETIME
);

-- View: NextScan
CREATE VIEW NextScan AS
    SELECT ID, Path
      FROM Scan
     WHERE Completed IS NULL
     ORDER BY ID ASC
     LIMIT 1;

COMMIT TRANSACTION;
PRAGMA foreign_keys = on;
