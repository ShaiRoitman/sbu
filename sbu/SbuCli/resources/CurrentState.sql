INSERT INTO BackupDB.CurrentState 
(
	Path,
	Size,
	Created,
	Modified,
	Accessed,
	DigestType,
	DigestValue,
	Status
)
SELECT 
	Path, 
	Size, 
	Created, 
	Modified, 
	Accessed,
	DigestType,
	DigestValue,
	'Current' 
FROM (
	SELECT 
		Files.Path, 
		Files.Size, 
		Files.Created, 
		Files.Modified, 
		Files.Accessed,
		Files.DigestType,
		Files.DigestValue,
		Files.Status
	FROM 
	(SELECT 
		MAX(Files.ID) AS FILEID 
	FROM Files 
		JOIN
			Backups ON Files.BackupID = Backups.ID
		JOIN 
			BackupDefs ON Backups.BackupDefID = BackupDefs.ID
		WHERE BackupDefs.ID = :backupDefID
		GROUP BY Files.Path ) Latest
	JOIN Files ON 
		Latest.FILEID = Files.ID) repo;
