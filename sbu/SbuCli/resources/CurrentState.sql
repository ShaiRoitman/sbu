INSERT INTO BackupDB.CurrentState 
(
	Path,
	Type,
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
	Type,
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
		Files.Type,
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
		WHERE 
			BackupDefs.ID = :backupDefID AND
			Backups.Status = 'Complete'
		GROUP BY Files.Path ) Latest
	JOIN Files ON 
		Latest.FILEID = Files.ID) repo
	WHERE repo.Status !='Deleted';
