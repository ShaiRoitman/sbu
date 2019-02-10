INSERT INTO FILES 
(
	BackupID,
	Path,
	Type,
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
	Type,
	Size,
	Created,
	Modified,
	Accessed,
	DigestType,
	DigestValue,
	FileHandle,
	Status
FROM BackupDB.CurrentState;