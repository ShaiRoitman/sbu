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
	std::string retValue = boost::asio::ip::host_name();

	logger->DebugFormat("getHostName() retValue:[%s]", retValue.c_str());
	return retValue;
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

std::string return_time_and_date(time_t time)
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


bool copy_file_logged(boost::filesystem::path srcPath, boost::filesystem::path outFilePath)
{
	bool retValue = false;
	try
	{
		boost::filesystem::copy_file(srcPath, outFilePath, boost::filesystem::copy_option::overwrite_if_exists);
		logger->InfoFormat("CopyFile() Status:[Success] from:[%s] to:[%s]",
			srcPath.string().c_str(),
			outFilePath.string().c_str());
		retValue = true;
	}
	catch (std::exception ex)
	{
		logger->ErrorFormat("CopyFile() Status:[Failed] from:[%s] to:[%s] exception:[%s]",
			srcPath.string().c_str(),
			outFilePath.string().c_str(),
			ex.what());
	}

	return retValue;
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

std::string calcHash(const std::string& str)
{
	unsigned char sha1p[SHA_DIGEST_LENGTH];
	char digest[256];
	memset(digest, 0, sizeof(digest));
	SHA_CTX ctxt;
	SHA1_Init(&ctxt);
	SHA1_Update(&ctxt, str.c_str(), str.length());
	SHA1_Final(sha1p, &ctxt);

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
		logger->ErrorFormat("getOrCreateDb() dbPath:[%s] Failed to open exception:[%s]", dbPath.string().c_str(), ex.what());
	}

	logger->InfoFormat("getOrCreateDb() dbPath:[%s], dbExists:[%d] initScript [%s]", dbPath.string().c_str(), dbexists, "");
	return db;
}
