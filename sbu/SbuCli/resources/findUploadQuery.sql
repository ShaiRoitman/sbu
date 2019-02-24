SELECT 
	Path,
	DigestType, 
	DigestValue 
FROM 
	NextState 
WHERE 
	UploadState IS NULL AND
	(Status='Added' OR 	Status='Updated' ) AND 
	Type='File'
