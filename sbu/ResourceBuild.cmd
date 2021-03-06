@echo on

set FILENAME=SbuCli\headers\Text_Resources.h

echo namespace Text_Resource { > %FILENAME%
CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\databases\BackupDB.sql BackupDB >> %FILENAME%
CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\databases\Repository.sql Repository >> %FILENAME%
CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\databases\FileRepository.sql FileRepository >> %FILENAME%

CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\AddMissing.sql AddMissing >> %FILENAME%
CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\AddUpdated.sql AddUpdated >> %FILENAME%
CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\CopyBackupState.sql CopyBackupState >> %FILENAME%
CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\CurrentState.sql CurrentState >> %FILENAME%
CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\InsertDirectory.sql InsertDirectory >> %FILENAME%
CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\InsertFile.sql InsertFile >> %FILENAME%
CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\MarkDeleted.sql MarkDeleted >> %FILENAME%
CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\RestoreQuery.sql RestoreQuery >> %FILENAME%
CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\findHashCalcQuery.sql findHashCalcQuery >> %FILENAME%
CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\findUploadQuery.sql findUploadQuery >> %FILENAME%
CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\findBackupInfo.sql findBackupInfo >> %FILENAME%

echo } >> %FILENAME%