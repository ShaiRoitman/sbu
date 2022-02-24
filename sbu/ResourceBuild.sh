#!/bin/bash

source variables.sh

export FILENAME="SbuCli/headers/Text_Resources.h"

echo Creating $FILENAME
echo namespace Text_Resource { > $FILENAME
$CompileTOCPP $ROOT_PATH/databases/BackupDB.sql BackupDB >> $FILENAME
$CompileTOCPP $ROOT_PATH/databases/Repository.sql Repository >> $FILENAME
$CompileTOCPP $ROOT_PATH/databases/FileRepository.sql FileRepository >> $FILENAME

$CompileTOCPP $ROOT_PATH/AddMissing.sql AddMissing >> $FILENAME
$CompileTOCPP $ROOT_PATH/AddUpdated.sql AddUpdated >> $FILENAME
$CompileTOCPP $ROOT_PATH/CopyBackupState.sql CopyBackupState >> $FILENAME
$CompileTOCPP $ROOT_PATH/CurrentState.sql CurrentState >> $FILENAME
$CompileTOCPP $ROOT_PATH/InsertDirectory.sql InsertDirectory >> $FILENAME
$CompileTOCPP $ROOT_PATH/InsertFile.sql InsertFile >> $FILENAME
$CompileTOCPP $ROOT_PATH/MarkDeleted.sql MarkDeleted >> $FILENAME
$CompileTOCPP $ROOT_PATH/RestoreQuery.sql RestoreQuery >> $FILENAME
$CompileTOCPP $ROOT_PATH/findHashCalcQuery.sql findHashCalcQuery >> $FILENAME
$CompileTOCPP $ROOT_PATH/findUploadQuery.sql findUploadQuery >> $FILENAME
$CompileTOCPP $ROOT_PATH/findBackupInfo.sql findBackupInfo >> $FILENAME

echo } >> $FILENAME
