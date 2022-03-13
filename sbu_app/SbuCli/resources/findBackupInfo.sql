SELECT Backups.ID AS ID,
       Backups.BackupDefID AS BackupDefID,
       BackupDefs.RootPath AS RootPath
FROM Backups
JOIN BackupDefs ON Backups.BackupDefID = BackupDefs.ID
WHERE Backups.BackupDefID=:backupdefID
ORDER BY ID DESC
LIMIT 1