#include "Loggers.h"
#include "SbuDatabasePoco.h"

#include "Poco/Data/Session.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Data/Transaction.h"
#include "Poco/Data/MetaColumn.h"

using namespace Poco::Data::Keywords;
using Poco::Data::MetaColumn;
using Poco::Data::Transaction;
using Poco::Data::Session;
using Poco::Data::Statement;
using Poco::Data::RecordSet;

static auto logger = LoggerFactory::getLogger("application.SbuDatabasePoco");

class PocoSbuDBColumn : public ISbuDBColumn
{
public:
	PocoSbuDBColumn(RecordSet& r, const char* c) :
		recordSet(r), columnName(c)
	{
	}
	virtual ~PocoSbuDBColumn() {}

	virtual std::string getString() const override
	{
		return recordSet.value(columnName).convert<std::string>();
	}

	virtual long long getInt64() const override
	{
		return recordSet.value(columnName).convert<long long>();
	}

	virtual int getInt() const override
	{
		return recordSet.value(columnName).convert<int>();
	}

	virtual bool isNull() const override
	{
		return recordSet.isNull(columnName);
	}

	std::string columnName;
	RecordSet& recordSet;
};

class PocoSbuDBStatement : public ISbuDBStatement
{
public:
	PocoSbuDBStatement(Session session, const std::string& s) : session(session), statement(session), rs(statement)
	{
		statement << s;
	}

	virtual std::shared_ptr<ISbuDBColumn> getColumn(const char* columnName) override
	{
		logger->DebugFormat("PocoSbuDBStatement::getColumn() columnName:[%s]", columnName);
		auto retValue = std::make_shared<PocoSbuDBColumn>(rs, columnName);
		return retValue;
	}

	virtual void bind(const char* apName, const std::string&    aValue) override
	{
		logger->DebugFormat("PocoSbuDBStatement::bind() columnName:[%s] value:[%s]", apName, aValue.c_str());
		statement, bind(apName, aValue);
	}

	virtual void bind(const char* apName, long long aValue) override
	{
		logger->DebugFormat("PocoSbuDBStatement::bind() columnName:[%s] long value:[%lld]", apName, aValue);
		statement, bind(apName, aValue);
	}

	virtual int exec() override
	{
		statement, now;
		return 0;
	}

	virtual bool executeStep() override
	{
		statement, now;
		return true;
	}

	virtual void reset() override
	{
		statement.reset(session);
	}


	Statement statement;
	RecordSet rs;
	Session& session;
};

class PocoSbuDBTransaction : public ISbuDBTransaction
{
public:
	PocoSbuDBTransaction(Session& t) : transaction(t)
	{
		logger->DebugFormat("PocoSbuDBTransaction::SqliteSbuDBTransaction()");
	}
	virtual ~PocoSbuDBTransaction()
	{
		logger->DebugFormat("PocoSbuDBTransaction::~SqliteSbuDBTransaction()");
	}
	virtual void commit() override
	{
		logger->DebugFormat("PocoSbuDBTransaction::commit()");
		transaction.commit();
	}

	Transaction transaction;
};

class PocoSbuDBDatabase : public ISbuDBDatabase
{
public:
	PocoSbuDBDatabase(std::string connectionString, const char* initScript) : session("Postgres",connectionString)
	{
		session << initScript, now;
	}

	virtual std::shared_ptr <ISbuDBStatement> CreateStatement(const std::string& statement) override
	{
		auto retValue = std::make_shared<PocoSbuDBStatement>(session, statement);
		return retValue;
	}

	virtual std::shared_ptr <ISbuDBTransaction> CreateTransaction() override
	{
		auto retValue = std::make_shared<PocoSbuDBTransaction>(session);
		return retValue;
	}
	
	Session session;
};

std::shared_ptr<ISbuDBDatabase> CreatePocoDatabase(const std::string& connectionString, const char* initScript)
{
	return std::make_shared<PocoSbuDBDatabase>(connectionString, initScript);
}

