INSERT INTO Files ( BackupID, PATH, TYPE, SIZE, Created, Modified, Accessed, DigestType, DigestValue, FileHandle, Status)
SELECT :backupID,
       PATH,
       TYPE,
       SIZE,
       Created,
       Modified,
       Accessed,
       DigestType,
       DigestValue,
       FileHandle,
       Status
FROM BackupDB.NextState