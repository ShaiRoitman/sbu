SELECT 
	ID,
	Path,
	'Added' AS Status
FROM 
	CurrentState 
WHERE 
	Status='Added' AND 
	DigestValue IS NULL

UNION

SELECT
	CurrentState.ID,
	CurrentState.Path,
	'Updated' AS Status
FROM 
	CurrentState
LEFT JOIN 
	Entries
ON
	Entries.Path = CurrentState.Path
WHERE
	Entries.Path IS NOT NULL AND
	Entries.DigestValue IS NULL AND
	CurrentState.Status !='Deleted' AND
	Entries.Size != CurrentState.Size
