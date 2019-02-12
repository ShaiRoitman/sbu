UPDATE 
	CurrentState 
SET 
	Status='Deleted'
WHERE 
	Path IN (
		SELECT 
			CurrentState.Path 
		FROM 
			CurrentState 
		LEFT JOIN 
			Entries
		ON	
			CurrentState.Path = Entries.Path
		WHERE
			Entries.Path is NULL
		)

