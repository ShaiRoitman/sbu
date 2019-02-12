SELECT 
	ID,
	Path,
	DigestType, 
	DigestValue 
FROM 
	CurrentState 
WHERE 
	Status='Added' AND 
	Type='File' AND 
	UploadState IS NOT NULL

UNION

SELECT 
	ID,
	Path,
	DigestType,
	DigestValue
FROM
	CurrentState
WHERE
	Status='Updated' AND 
	UploadState IS NOT NULL