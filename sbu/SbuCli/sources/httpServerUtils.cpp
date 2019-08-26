#include "httpServerUtils.h"

void Convert(const IRepositoryDB::BackupDef& backupdef, std::shared_ptr<io::swagger::server::model::BackupDef> retValue)
{
	retValue->setId(backupdef.id);
	retValue->setName(backupdef.name);
	retValue->setPath(backupdef.rootPath.generic_string());
	retValue->setAdded(get_string_from_time_point(backupdef.added));
	retValue->setHostName(backupdef.hostName);
}
void Convert(const IRepositoryDB::BackupInfo& backup, std::shared_ptr<io::swagger::server::model::Backup> retValue)
{
	retValue->setStatus(backup.status);
	retValue->setId(backup.id);
	retValue->setStarted(get_string_from_time_point(backup.started));
	retValue->setLastStatusUpdate(get_string_from_time_point(backup.lastUpdated));
	retValue->setBackupDefId(backup.backupDefId);
}
std::shared_ptr<io::swagger::server::model::BackupDef> CreateBackupDef(const IRepositoryDB::BackupDef& backupdef)
{
	auto retValue = std::make_shared<io::swagger::server::model::BackupDef>();
	Convert(backupdef, retValue);
	return retValue;
}
std::shared_ptr<io::swagger::server::model::Backup> CreateBackup(const IRepositoryDB::BackupInfo& backup)
{
	auto retValue = std::make_shared<io::swagger::server::model::Backup>();
	Convert(backup, retValue);
	return retValue;
}
