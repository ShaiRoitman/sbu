#pragma once

#include "Operations.h"
#include "RepositoryDB.h"

class BackupOperation : public Operation
{
public:
	class Strategy
	{
	public:
		std::function<void(const IRepositoryDB::BackupInfo& backup)> successFunc;
	};

	BackupOperation();
	BackupOperation(std::shared_ptr<Strategy> strategy);

	int Init(boost::program_options::variables_map& vm);
	int Scan(boost::program_options::variables_map& vm);
	int DiffCalc(boost::program_options::variables_map& vm);
	int FileUpload(boost::program_options::variables_map& vm);
	int Complete(boost::program_options::variables_map& vm);
	int All(boost::program_options::variables_map& vm);
	int Operate(boost::program_options::variables_map& vm);
private:
	std::shared_ptr<Strategy> strategy;
};
