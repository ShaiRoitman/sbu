INSERT INTO NextState
(
	Path,
	Type,
	Size,
	Created,
	Modified,
	Accessed,
	DigestType,
	DigestValue,
	Status	
)
SELECT 
    Entries.Path,
	Entries.Type,
    Entries.Size,
    Entries.Created,
    Entries.Modified,
    Entries.Accessed,
	Entries.DigestType,
	Entries.DigestValue,
    'Updated'
FROM
    Entries
LEFT JOIN
    CurrentState ON CurrentState.Path = Entries.Path
WHERE 
    CurrentState.Path IS NOT NULL AND
	( Entries.Size != CurrentState.Size OR
	  Entries.Modified != CurrentState.Modified OR
	  Entries.Created != CurrentState.Created OR
	  Entries.Type != CurrentState.Type
	)
