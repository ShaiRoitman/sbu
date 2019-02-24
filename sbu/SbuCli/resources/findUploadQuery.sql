SELECT 
	Path,
	DigestType, 
	DigestValue 
FROM 
	NextState 
WHERE 
	UploadState IS NULL AND
	(Status='Added' OR 	Status='Updatd' ) AND 
	Type='File'
