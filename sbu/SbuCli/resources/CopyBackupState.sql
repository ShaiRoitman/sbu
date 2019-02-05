INSERT INTO FILES 
(
	BackupID,
	Path,
	Size,
	Created,
	Modified,
	Accessed,
	DigestType,
	DigestValue,
	FileHandle,
	Status
)
SELECT 
	:backupID,
	Path,
	Size,
	Created,
	Modified,
	Accessed,
	DigestType,
	DigestValue,
	FileHandle,
	Status
FROM BackupDB.CurrentState;