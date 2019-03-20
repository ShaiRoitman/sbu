#pragma once

#include <string>
#include <chrono>
#include <boost/filesystem.hpp>
#include "Loggers.h"
#include "Text_Resources.h"
#include "SQLiteCpp/Database.h"

typedef long long Integer;

std::string getHostName();
std::string to_utf8(boost::filesystem::path path);
boost::filesystem::path from_utf8(const std::string& str);
std::string return_current_time_and_date();
std::string return_time_and_date(__time64_t);
std::string calcHash(boost::filesystem::path path);
std::string calcHash(const std::string& str);

std::string get_string_from_time_point(std::chrono::system_clock::time_point tp);
std::chrono::system_clock::time_point get_time_point(const std::string& timePoint);
bool copy_file_logged(boost::filesystem::path srcPath, boost::filesystem::path outFilePath);

std::shared_ptr<SQLite::Database> getOrCreateDb(boost::filesystem::path dbPath, const char* initScript);
class Resource
{
public:
	static std::string ReadResource(boost::filesystem::path dir);
};