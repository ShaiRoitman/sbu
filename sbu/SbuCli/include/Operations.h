#pragma once
#include "boost/program_options/variables_map.hpp"

class Operation
{
public:
	typedef std::function < std::shared_ptr<Operation>()> Factory;
	virtual int Operate(boost::program_options::variables_map& vm) = 0;
};

std::shared_ptr<Operation> CreateBackupDefFactory();
std::shared_ptr<Operation> ListBackupDefsFactory();
std::shared_ptr<Operation> ListBackupsFactory();
std::shared_ptr<Operation> RestoreFactory();
std::shared_ptr<Operation> BackupFactory();
std::shared_ptr<Operation> BackupInfoFactory();
std::shared_ptr<Operation> BackupScanFactory();
std::shared_ptr<Operation> BackupDiffCalcFactory();
std::shared_ptr<Operation> BackupFileUploadFactory();
