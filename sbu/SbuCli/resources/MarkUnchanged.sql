UPDATE 
	CurrentState 
SET 
	Status='Unchanged'
WHERE
	CurrentState.Type='File' AND
	Path IN
(
	SELECT 
		CurrentState.Path
	FROM 
		CurrentState
	LEFT JOIN 
		Entries
	ON
		CurrentState.Path = Entries.Path AND
		CurrentState.DigestValue = Entries.DigestValue AND
		CurrentState.Status = 'Current'
)