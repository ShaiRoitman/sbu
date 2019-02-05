@echo on

set FILENAME=SbuCli\headers\Text_Resources.h

echo namespace Text_Resource { > %FILENAME%
CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\AddMissing.sql AddMissing >> %FILENAME%
CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\BackupDB.sql BackupDB >> %FILENAME%
CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\CopyBackupState.sql CopyBackupState >> %FILENAME%
CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\CurrentState.sql CurrentState >> %FILENAME%
CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\DeleteUnchanged.sql DeleteUnchanged >> %FILENAME%
CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\FileRepository.sql FileRepository >> %FILENAME%
CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\InsertDirectory.sql InsertDirectory >> %FILENAME%
CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\InsertFile.sql InsertFile >> %FILENAME%
CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\MarkDeleted.sql MarkDeleted >> %FILENAME%
CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\MarkUpdated.sql MarkUpdated >> %FILENAME%
CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\Repository.sql Repository >> %FILENAME%
CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\RestoreQuery.sql RestoreQuery >> %FILENAME%
CompileToCppVariable.exe C:\git\sbu\sbu\SbuCli\resources\findDiffQuery.sql findDiffQuery >> %FILENAME%


echo } >> %FILENAME%