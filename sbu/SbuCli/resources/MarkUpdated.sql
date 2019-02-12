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
		CurrentState.DigestValue IS NOT NULL AND
		Entries.DigestValue != CurrentState.DigestValue AND
		Entries.Type = 'File' AND
		(CurrentState.Status != 'Added' OR CurrentState.Status !='Deleted')
)