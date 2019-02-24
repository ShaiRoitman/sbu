SELECT 
	Entries.Path
FROM 
	Entries
LEFT JOIN 
	CurrentState
ON 
	Entries.Path = CurrentState.Path
WHERE 
	Entries.DigestValue IS NULL OR
	( CurrentState.Path IS NULL AND
	  Entries.Size != CurrentState.Size OR
	  Entries.Modified != CurrentState.Modified OR
	  Entries.Created != CurrentState.Created OR
	  Entries.Type != CurrentState.Type
	)
