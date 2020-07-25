#include "utils.h"

#include <iomanip>
#include <locale>
#include <codecvt>
#include <chrono>
#include <sstream>
#include <fstream>
#include "boost/asio/ip/host_name.hpp"
#include "openssl/sha.h"
#include "SbuDatabaseSQLite.h"
#include <signal.h>
#include <boost/stacktrace.hpp>

static auto logger = LoggerFactory::getLogger("application.Utils");

Integer getIntegerFromString(const std::string& string)
{
	Integer retValue;

	std::stringstream s(string);
	s >> retValue;

	return retValue;
}

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

std::shared_ptr<ISbuDBDatabase> getOrCreateDb(boost::filesystem::path dbPath, const char* initScript)
{
	return CreateSQLiteDB(dbPath, initScript);
}

void my_signal_handler(int signum) {
	::signal(signum, SIG_DFL);
	boost::stacktrace::safe_dump_to("./backtrace.dump");
	::raise(SIGABRT);
}
void register_stacktrace_handler()
{
	::signal(SIGSEGV, &my_signal_handler);
	::signal(SIGABRT, &my_signal_handler);
}

void AddToExecutionLog(std::shared_ptr<ISbuDBDatabase> db, const std::string& comment, const std::string& argument)
{
	auto insertQuery = db->CreateStatement("INSERT INTO ExecutionLog (EventTime, Comment, Argument) VALUES (:time, :comment, :arg)");

	auto currentTime = return_current_time_and_date();

	insertQuery->bind(":time", currentTime);
	insertQuery->bind(":comment", comment);
	insertQuery->bind(":arg", argument);
	auto result = insertQuery->exec();

	logger->DebugFormat("AddToExecutionLog() time:[%s] comment:[%s] arg:[%s] result:[%d]",
		currentTime.c_str(),
		comment.c_str(),
		argument.c_str(),
		result);
}

std::string getValueAsString(boost::program_options::variables_map& vm, const char* id)
{
	try {
		auto retValue = vm[id].as<std::string>();
		return retValue;
	}
	catch (std::exception ex)
	{
		logger->ErrorFormat("Failed to retrieve value %s from arguments", id);
		throw;
	}
}

std::string getValueAsString(boost::program_options::variables_map& vm, const char* id, const char* defaultValue)
{
	std::string retValue = defaultValue;

	if (!vm[id].empty())
		retValue = vm[id].as<std::string>();

	return retValue;
}

int getValueAsInt(boost::program_options::variables_map& vm, const char* id)
{
	try {
		auto retValue = vm[id].as<int>();
		return retValue;
	}
	catch (std::exception ex)
	{
		logger->ErrorFormat("Failed to retrieve value %s from arguments", id);
		throw;
	}
}
