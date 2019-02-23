SELECT 
	ID,
	Path,
	Status
FROM 
	CurrentState 
WHERE 
	(Status='Added' OR Status='Updated' ) AND 
	DigestValue IS NULL
