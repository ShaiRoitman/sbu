#pragma once

#include "../httpModels/HelpInformation.h"
#include "../httpModels/ProgramInformation.h"
#include "../httpModels/BackupDefs.h"
#include "../httpModels/BackupInfo.h"
#include "../httpModels/FullBackupDefInfo.h"
#include "../httpModels/CreateBackupDef.h"
#include "../httpModels/RestoreOptions.h"
#include "../httpModels/ConfigurationBody.h"
#include "RepositoryDB.h"

void Convert(const IRepositoryDB::BackupDef& backupdef, std::shared_ptr<io::swagger::server::model::BackupDef> retValue);
void Convert(const IRepositoryDB::BackupInfo& backup, std::shared_ptr<io::swagger::server::model::Backup> retValue);
std::shared_ptr<io::swagger::server::model::BackupDef> CreateBackupDef(const IRepositoryDB::BackupDef& backupdef);
std::shared_ptr<io::swagger::server::model::Backup> CreateBackup(const IRepositoryDB::BackupInfo& backup);
