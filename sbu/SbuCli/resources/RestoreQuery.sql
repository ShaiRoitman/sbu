SELECT Files.Path,
       Files.Size,
       Files.Type,
       Files.Created,
       Files.Modified,
       Files.Accessed,
       Files.Status,
       Files.FileHandle
FROM
  (SELECT MAX(Files.ID) AS FILEID
   FROM Files
   JOIN Backups ON Files.BackupID = Backups.ID
   JOIN BackupDefs ON BackupDefID = BackupDefs.ID
   WHERE BackupDefs.ID = :backupDefID
     AND Backups.Started <= :startDate
     AND Backups.ID <= :backupID
     AND Backups.Status = 'Complete'
   GROUP BY Files.Path) Latest
JOIN Files ON Latest.FILEID = Files.ID