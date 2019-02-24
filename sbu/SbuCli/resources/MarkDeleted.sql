INSERT INTO NextState
	(Path, Type, Status)
SELECT 
	CurrentState.Path, 
	CurrentState.Type,
	'Deleted'
FROM 
	CurrentState
LEFT JOIN 
	Entries
ON	
	CurrentState.Path = Entries.Path
WHERE
	Entries.Path is NULL
