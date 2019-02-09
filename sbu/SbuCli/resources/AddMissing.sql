INSERT INTO CurrentState
(
	Path,
	Size,
	Created,
	Modified,
	Accessed,
	Status
)
SELECT 
    Entries.Path,
    Entries.Size,
    Entries.Created,
    Entries.Modified,
    Entries.Accessed,
    'Added'
FROM
    Entries
LEFT JOIN 
    CurrentState ON CurrentState.Path = Entries.Path
Where 
    CurrentState.Path is NULL and Entries.Type='File'