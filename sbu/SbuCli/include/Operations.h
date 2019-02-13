#pragma once
#include "boost/program_options/variables_map.hpp"

class Operation
{
public:
	virtual int Operate(boost::program_options::variables_map& vm) = 0;
};

class CreateBackupDefOperation : public Operation
{
public:
	virtual int Operate(boost::program_options::variables_map& vm) override;
};

class ListBackupDefsOperation : public Operation
{
public:
	virtual int Operate(boost::program_options::variables_map& vm) override;
};

class ListBackupsOperation : public Operation
{
public:
	virtual int Operate(boost::program_options::variables_map& vm) override;
};

class RestoreOperation : public Operation
{
public:
	virtual int Operate(boost::program_options::variables_map& vm) override;
};

class BackupOperation : public Operation
{
public:
	virtual int Operate(boost::program_options::variables_map& vm) override;
};
