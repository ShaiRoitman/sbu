SELECT Entries.Path
FROM Entries
LEFT JOIN CurrentState ON Entries.Path = CurrentState.Path
WHERE Entries.DigestValue IS NULL
  AND Entries.Size IS NOT NULL
  AND CurrentState.Path IS NULL
  OR (Entries.Size != CurrentState.Size
      OR Entries.Modified != CurrentState.Modified
      OR Entries.Created != CurrentState.Created
      OR Entries.Type != CurrentState.Type )