#pragma once

#include <boost/filesystem.hpp>
#include "FileRepositoryDB.h"

class IBackupDB
{
public:
	virtual void StartScan(boost::filesystem::path dir) = 0;
	virtual void ContinueScan() = 0;
	virtual bool IsScanDone() = 0;

	virtual void StartDiffCalc() = 0;
	virtual void ContinueDiffCalc() = 0;
	virtual bool IsDiffCalcDone() = 0;

	virtual void StartUpload(std::shared_ptr<IFileRepositoryDB> fileDB) = 0;
	virtual void ContinueUpload(std::shared_ptr<IFileRepositoryDB> fileDB) = 0;
	virtual bool IsUploadDone() = 0;

	virtual void Complete() = 0;
};

std::shared_ptr<IBackupDB> CreateDB(boost::filesystem::path dbPath);