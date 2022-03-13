INSERT INTO NextState ( PATH, TYPE, SIZE, Created, Modified, Accessed, DigestType, DigestValue, Status)
SELECT Entries.Path,
       Entries.Type,
       Entries.Size,
       Entries.Created,
       Entries.Modified,
       Entries.Accessed,
       Entries.DigestType,
       Entries.DigestValue,
       'Added'
FROM Entries
LEFT JOIN CurrentState ON CurrentState.Path = Entries.Path
WHERE CurrentState.Path IS NULL