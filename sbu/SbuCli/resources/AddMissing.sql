INSERT INTO CurrentState
(
	Path,
	Type,
	Size,
	Created,
	Modified,
	Accessed,
	Status	
)
SELECT 
    Entries.Path,
	Entries.Type,
    Entries.Size,
    Entries.Created,
    Entries.Modified,
    Entries.Accessed,
    'Added'
FROM
    Entries
LEFT JOIN 
    CurrentState ON CurrentState.Path = Entries.Path
WHERE 
    CurrentState.Path is NULL