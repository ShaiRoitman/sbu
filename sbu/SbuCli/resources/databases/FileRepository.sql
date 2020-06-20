--
-- File generated with SQLiteStudio v3.2.1 on Fri Jan 18 20:13:57 2019
--
-- Text encoding used: System
--
PRAGMA foreign_keys = off;
BEGIN TRANSACTION;

-- Table: Files
CREATE TABLE Files (
    ID						INTEGER PRIMARY KEY AUTOINCREMENT,
    Path					TEXT,
	Added					DATETIME,
	Size					BIGINT,
    DigestType				TEXT,
    DigestValue				TEXT,
	MultiFileHostDigest		TEXT
);

CREATE INDEX digestValueIndex ON Files (
    DigestValue ASC
);

CREATE INDEX multiFileHostDigestIndex ON Files (
    MultiFileHostDigest ASC
);


COMMIT TRANSACTION;
PRAGMA foreign_keys = on;
