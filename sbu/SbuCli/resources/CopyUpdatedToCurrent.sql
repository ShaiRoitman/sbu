INSERT INTO 
	CurrentState
	(Path, Type, Size, Created, Modified, Accessed, DigestType, DigestValue, Status)
SELECT 
	Path, Type, Size, Created, Modified, Accessed, DigestType, DigestValue, Status
FROM
	Entries
WHERE 
	Status='Updated' OR Status='Added'