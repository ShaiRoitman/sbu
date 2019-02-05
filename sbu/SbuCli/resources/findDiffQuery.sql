ATTACH DATABASE 'backupDB.db' AS BackupDB;

select * from BackupDB.Entries

SELECT * FRom Files;
SELECT Path,Size from backupDB.Entries Where Type=='File';

SELECT Path,Size, Status FROM Files JOIN Backups ON Files.BackupID = Backups.ID JOIN BackupDefs ON Backups.BackupDefID = BackupDefs.ID Where BackupDefs.ID = 1;

INSERT INTO Files (BackupID, Status, Path , Size) SELECT 1, 'Added' ,Path, Size FROM backupDB.Entries Where Type='File';

Update Files Set Status="Added" ;
DELETE From Files;

SELECT a.Path,a.Size,b.Size AS NewSize
FROM 
(
    SELECT Path,Size from BackupDB.Entries Where Type='File'
) AS a
JOIN 
(
    SELECT Files.Path,Files.Size, Files.Status FROM Files 
    JOIN Backups ON Files.BackupID = Backups.ID 
    JOIN BackupDefs ON Backups.BackupDefID = BackupDefs.ID 
    Where BackupDefs.ID = 1
) AS b
ON 
    a.Path=b.Path and 
    a.Size != b.Size;

SELECT DefID, Files.Path, Files.Size, Files.Created, Files.Modified, Files.Accessed  FROM 
(SELECT BackupDefs.ID AS DefID, MAX(Files.ID) AS FILEID FROM Files 
JOIN
    Backups ON Files.BackupID = Backups.ID
JOIN 
    BackupDefs ON Backups.BackupDefID = BackupDefs.ID
    GROUP BY Files.Path ORDER BY Files.ID ) Latest
JOIN Files ON 
    Latest.FILEID = Files.ID;

DROP TABLE NewBackup;
CREATE TABLE NewBackup AS
SELECT PATH,Size,Created,Modified,Accessed,'New' AS STATUS FROM BackupDB.Entries;

DROP TABLE CurrentState;
Create TABLE CurrentState
AS
SELECT * FROM (
 SELECT Files.Path, Files.Size, Files.Created, Files.Modified, Files.Accessed,Files.Status  FROM 
(SELECT MAX(Files.ID) AS FILEID FROM Files 
JOIN
    Backups ON Files.BackupID = Backups.ID
JOIN 
    BackupDefs ON Backups.BackupDefID = BackupDefs.ID
    WHERE BackupDefs.ID = :id
    GROUP BY Files.Path ) Latest
JOIN Files ON 
    Latest.FILEID = Files.ID) repo;

UPDATE NewBackup SET Status='Deleted' WHERE NewBackup.Path IN (
SELECT NewBackup.Path from NewBackup LEFT JOIN CurrentState ON NewBackup.Path = CurrentState.Path WHERE CurrentState.Path is NULL);

DELETE FROM NewBackup WHERE NewBackup.Path IN (
SELECT NewBackup.Path FROM NewBackup JOIN CurrentState ON NewBackup.Path = CurrentState.Path WHERE NewBackup.Size = CurrentState.Size);