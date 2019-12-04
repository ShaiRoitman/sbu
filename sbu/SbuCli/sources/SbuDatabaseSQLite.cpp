#include "Loggers.h"
#include "SbuDatabaseSQLite.h"

#include "SQLiteCpp/SQLiteCpp.h"
#include "SQLiteCpp/Database.h"


static auto logger = LoggerFactory::getLogger("application.SbuDatabaseSQLite");

using namespace SQLite;

class SqliteSbuDBColumn : public ISbuDBColumn
{
public:
	SqliteSbuDBColumn(SQLite::Statement& statement, const char* columName) : column(statement.getColumn(columName))
	{
	}
	virtual ~SqliteSbuDBColumn() {}
	virtual std::string getString() const override
	{
		return column.getString();
	}

	virtual long long getInt64() const override
	{
		return column.getInt64();
	}

	virtual int getInt() const override
	{
		return column.getInt();
	}

	virtual bool isNull() const override
	{
		return column.isNull();
	}

	SQLite::Column column;
};

class SqliteSbuDBStatement : public ISbuDBStatement
{
public:
	SqliteSbuDBStatement(SQLite::Database& db, const std::string& s) : statement(db, s) {}

	virtual std::shared_ptr<ISbuDBColumn> getColumn(const char* columnName) override
	{
		logger->DebugFormat("SqliteSbuDBStatement::getColumn() columnName:[%s]", columnName);
		auto retValue = std::make_shared<SqliteSbuDBColumn>(statement, columnName);
		return retValue;
	}

	virtual void bind(const char* apName, const std::string&    aValue) override
	{
		logger->DebugFormat("SqliteSbuDBStatement::bind() columnName:[%s] value:[%s]", apName, aValue.c_str());
		statement.bind(apName, aValue);
	}

	virtual void bind(const char* apName, long long aValue) override
	{
		logger->DebugFormat("SqliteSbuDBStatement::bind() columnName:[%s] long value:[%lld]", apName, aValue);
		statement.bind(apName, aValue);
	}

	virtual int exec() override
	{
		return statement.exec();
	}

	virtual bool executeStep() override
	{
		return statement.executeStep();
	}

	virtual void reset() override
	{
		statement.reset();
	}


	SQLite::Statement statement;
};

class SqliteSbuDBTransaction : public ISbuDBTransaction
{
public:
	SqliteSbuDBTransaction(SQLite::Database& t) : transaction(t) 
	{
		logger->DebugFormat("SqliteSbuDBTransaction::SqliteSbuDBTransaction()");
	}
	virtual ~SqliteSbuDBTransaction()
	{
		logger->DebugFormat("SqliteSbuDBTransaction::~SqliteSbuDBTransaction()");
	}
	virtual void commit() override
	{
		logger->DebugFormat("SqliteSbuDBTransaction::commit()");
		transaction.commit();
	}

	SQLite::Transaction transaction;
};

std::shared_ptr<SQLite::Database> getOrCreateSQLiteDb(boost::filesystem::path dbPath, const char* initScript)
{
	std::shared_ptr<SQLite::Database> db = nullptr;
	bool dbexists = false;
	try
	{
		dbexists = exists(dbPath);
		db = std::make_shared<SQLite::Database>(dbPath.string(), SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);

		if (!dbexists)
		{
			db->exec(initScript);
		}
	}
	catch (std::exception ex)
	{
		logger->ErrorFormat("getOrCreateDb() dbPath:[%s] Failed to open exception:[%s]", dbPath.string().c_str(), ex.what());
	}

	logger->InfoFormat("getOrCreateDb() dbPath:[%s], dbExists:[%d] initScript [%s]", dbPath.string().c_str(), dbexists, "");
	return db;
}

class SqliteSbuDBDatabase : public ISbuDBDatabase
{
public:
	SqliteSbuDBDatabase(boost::filesystem::path dbPath, const char* initScript)
	{
		this->db = getOrCreateSQLiteDb(dbPath, initScript);
	}

	virtual std::shared_ptr <ISbuDBStatement> CreateStatement(const std::string& statement) override
	{
		auto retValue = std::make_shared<SqliteSbuDBStatement>(*db, statement);
		return retValue;
	}

	virtual std::shared_ptr <ISbuDBTransaction> CreateTransaction() override
	{
		auto retValue = std::make_shared<SqliteSbuDBTransaction>(*db);
		return retValue;
	}

	std::shared_ptr<SQLite::Database> db;
};

std::shared_ptr<ISbuDBDatabase> CreateSQLiteDB(boost::filesystem::path dbPath, const char* initScript)
{
	return std::make_shared<SqliteSbuDBDatabase>(dbPath, initScript);
}

