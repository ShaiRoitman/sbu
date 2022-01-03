#pragma once

#include "Operations.h"
#include "RepositoryDB.h"

class RestoreOperation : public Operation
{
public:
	class Strategy
	{
	public:
		std::function<void(boost::filesystem::path& destination)> altToCopy;
	};

	RestoreOperation();
	RestoreOperation(std::shared_ptr<Strategy> strategy);
	int Operate(boost::program_options::variables_map& vm);
public:
	std::shared_ptr<Strategy> strategy;
};