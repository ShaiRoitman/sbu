INSERT INTO NextState (PATH, TYPE, Status)
SELECT CurrentState.Path,
       CurrentState.Type,
       'Deleted'
FROM CurrentState
LEFT JOIN Entries ON CurrentState.Path = Entries.Path
WHERE Entries.Path IS NULL