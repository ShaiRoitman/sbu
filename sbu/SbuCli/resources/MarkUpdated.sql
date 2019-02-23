UPDATE 
	CurrentState
SET 
	Status='Updated' 
WHERE 
	Path IN
(
	SELECT 
		CurrentState.Path
	FROM 
		CurrentState
	LEFT JOIN
		Entries
	ON 
		CurrentState.Path = Entries.Path
	WHERE
		CurrentState.Status != 'Added' AND 
		Entries.Path IS NOT NULL AND
		CurrentState.Type = 'File' AND
		(CurrentState.Size != Entries.Size OR
		 CurrentState.Modified != Entries.Modified)
)		
