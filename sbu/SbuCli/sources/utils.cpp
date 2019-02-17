#include "utils.h"

#include <iomanip>
#include <locale>
#include <codecvt>
#include <chrono>
#include <sstream>
#include "boost/asio/ip/host_name.hpp"
#include "openssl/sha.h"

static auto logger = LoggerFactory::getLogger("application.Utils");

std::string getHostName()
{
	return boost::asio::ip::host_name();
}

std::string to_utf8(boost::filesystem::path path)
{
	std::wstring_convert< std::codecvt_utf8<wchar_t>, wchar_t > convert;
	return convert.to_bytes(path.wstring());
}

boost::filesystem::path from_utf8(const std::string& str)
{
	std::wstring_convert< std::codecvt_utf8<wchar_t>, wchar_t > convert;
	return boost::filesystem::path(convert.from_bytes(str));

}

std::string return_current_time_and_date()
{
	auto now = std::chrono::system_clock::now();
	return get_string_from_time_point(now);
}

std::string return_time_and_date(__time64_t time)
{
	std::stringstream ss;
	ss << std::put_time(std::localtime(&time), "%Y-%m-%d %X");
	return ss.str();
}

std::string get_string_from_time_point(std::chrono::system_clock::time_point tp)
{
	auto in_time_t = std::chrono::system_clock::to_time_t(tp);
	return return_time_and_date(in_time_t);
}

std::chrono::system_clock::time_point get_time_point(const std::string& timePoint)
{
	std::chrono::system_clock::time_point retValue;

	std::istringstream iss(timePoint);
	std::tm tm{};
	iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

	retValue += std::chrono::seconds(std::mktime(&tm));
	return retValue;
}

std::string Resource::ReadResource(boost::filesystem::path dir)
{
	boost::filesystem::path rootDir("C:\\git\\sbu\\sbu\\SbuCli\\resources");

	boost::filesystem::path fullDir = rootDir;
	fullDir /= dir;

	std::ifstream inputStream(fullDir.string());
	std::string str((std::istreambuf_iterator<char>(inputStream)),
		std::istreambuf_iterator<char>());

	return str;
}

int sha1(const char * name, unsigned char * out)
{
	const int MAX_BUF_LEN = 64 * 1024;
	FILE * pf;
	unsigned char buf[MAX_BUF_LEN];
	SHA_CTX ctxt;

	pf = fopen(name, "rb");

	if (!pf)
		return -1;

	SHA1_Init(&ctxt);

	while (1)
	{
		size_t len;

		len = fread(buf, 1, MAX_BUF_LEN, pf);

		if (len <= 0)
			break;

		SHA1_Update(&ctxt, buf, len);
	}

	fclose(pf);

	SHA1_Final(out, &ctxt);

	return 0;
}


void bin2hex(unsigned char * src, int len, char * hex)
{
	int i, j;

	for (i = 0, j = 0; i < len; i++, j += 2)
		sprintf(&hex[j], "%02x", src[i]);
}

std::string calcHash(boost::filesystem::path path)
{
	unsigned char sha1p[SHA_DIGEST_LENGTH];
	char digest[256];
	memset(digest, 0, sizeof(digest));
	sha1(path.string().c_str(), sha1p);
	bin2hex(sha1p, sizeof(sha1p), digest);
	return digest;
}


std::shared_ptr<SQLite::Database> getOrCreateDb(boost::filesystem::path dbPath, const char* initScript)
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
		logger->Error((std::string("Error in ") + std::string(ex.what())).c_str());
	}

	logger->InfoFormat("getOrCreateDb() dbPath:[%s], dbExists:[%d] initScript [%s]", dbPath.string().c_str(), dbexists, "");
	return db;
}

#ifdef TEST 
auto backupDef = RepoDB->AddBackupDef("Shai", "c:\\git\\clu");
auto values = RepoDB->GetBackupDefs();
auto firstBackupDef = *values.begin();
auto tp = get_string_from_time_point(firstBackupDef.added);

auto backupId = RepoDB->Backup(IRepositoryDB::BackupParameters().BackupDefId(firstBackupDef.id));

boost::filesystem::remove_all("c:\\git\\clu2");
auto restorea = RepoDB->Restore(IRepositoryDB::RestoreParameters().BackupDefId(firstBackupDef.id).RootDest("c:\\git\\clu2"));

//auto backupDeleted = RepoDB->DeleteBackup(backupId.id);
//bool deleted = RepoDB->DeleteBackupDef(backupDef.id);

std::string databaseName = "backupDb.db";
printf("Smart Backup Utility\n");
remove(databaseName.c_str());
std::shared_ptr<IBackupDB> backupDB = CreateSQLiteDB(databaseName);
backupDB->StartScan("C:\\git\\sbu\\sbu\\SbuCli");

std::string fileRepoDB = "fileRepo.db";
remove(fileRepoDB.c_str());
std::shared_ptr<IFileRepositoryDB> fileRepDB = CreateFileRepositorySQLiteDB(fileRepoDB, "C:\\git\\sbu\\sbu\\SbuCli\\Repo");
std::string fileHandle = fileRepDB->AddFile("C:\\git\\sbu\\sbu\\SbuCli\\sources\\sbu.cpp");
fileRepDB->GetFile(fileHandle, "C:\\git\\sbu\\sbu\\SbuCli\\sources\\sbu.cpp2");

std::string repoDB = "repoDB.db";
remove(repoDB.c_str());
std::shared_ptr<IRepositoryDB> RepoDB = CreateRepositorySQLiteDB(repoDB);
RepoDB->SetFileRepositoryDB(fileRepDB);
#endif
