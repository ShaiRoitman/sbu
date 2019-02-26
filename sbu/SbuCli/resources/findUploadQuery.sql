SELECT PATH,
       DigestType,
       DigestValue
FROM NextState
WHERE UploadState IS NULL
  AND (Status='Added'
       OR Status='Updated')
  AND TYPE='File'