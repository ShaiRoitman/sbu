#pragma once

#include <string>
#include <chrono>
#include <boost/filesystem.hpp>
#include "Loggers.h"
#include "Text_Resources.h"
#include "SbuDatabase.h"

typedef long long Integer;

Integer getIntegerFromString(const std::string& string);

std::string getHostName();
std::string to_utf8(boost::filesystem::path path);
boost::filesystem::path from_utf8(const std::string& str);
std::string return_current_time_and_date();
std::string return_time_and_date(time_t tp);
std::string calcHash(boost::filesystem::path path);
std::string calcHash(const std::string& str);

void register_stacktrace_handler();


std::string get_string_from_time_point(std::chrono::system_clock::time_point tp);
std::chrono::system_clock::time_point get_time_point(const std::string& timePoint);
bool copy_file_logged(boost::filesystem::path srcPath, boost::filesystem::path outFilePath);

std::shared_ptr<ISbuDBDatabase> getOrCreateDb(boost::filesystem::path dbPath, const char* initScript);

class Resource
{
public:
	static std::string ReadResource(boost::filesystem::path dir);
};

#include <sys/stat.h>
#include "boost/filesystem/path.hpp"
namespace sbu_stats
{
#ifdef _WIN32
	typedef struct _stat64 Stat;
	static __inline int __CRTDECL stat(boost::filesystem::path path, Stat* const _Stat)
	{
		return ::_wstati64(path.generic_wstring().c_str(), _Stat);
	}
#else
	typedef struct stat Stat;
	static __inline int __CRTDECL stat(char const* const _FileName, Stat* const _Stat)
	{
		return ::stat(_FileName, _Stat);
	}
#endif
}

