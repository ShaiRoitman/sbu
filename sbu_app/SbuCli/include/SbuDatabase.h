#pragma once

#include <string>
#include <memory>

class ISbuDBColumn
{
public:
	virtual ~ISbuDBColumn() {}
	virtual std::string getString() const = 0 ;
	virtual long long getInt64() const = 0 ;
	virtual int getInt() const = 0;
	virtual bool isNull() const = 0;
};

class ISbuDBStatement
{
public:
	virtual ~ISbuDBStatement() {}
	virtual std::shared_ptr<ISbuDBColumn> getColumn(const char* columnName) = 0;
	virtual void bind(const char* apName, const std::string&    aValue) = 0;
	virtual void bind(const char* apName, long long aValue) = 0;
	virtual int exec() = 0;
	virtual bool executeStep() = 0;
	virtual void reset() = 0;
}; 

class ISbuDBTransaction
{
public:
	virtual ~ISbuDBTransaction() {}
	virtual void commit() = 0;
}; 

class ISbuDBDatabase
{
public:
	virtual ~ISbuDBDatabase() {}
	virtual std::shared_ptr <ISbuDBStatement> CreateStatement(const std::string& statement) = 0;
	virtual std::shared_ptr <ISbuDBTransaction> CreateTransaction() = 0;
};
