#pragma once

#include <string>
#include <chrono>
#include <boost/filesystem.hpp>
#include "Loggers.h"
#include "Text_Resources.h"
#include "SQLiteCpp/Database.h"
#include "FileRepositoryDB.h"

typedef long long Integer;

std::string getHostName();
std::string to_utf8(boost::filesystem::path path);
boost::filesystem::path from_utf8(const std::string& str);
std::string return_current_time_and_date();
std::string return_time_and_date(__time64_t);
std::string calcHash(boost::filesystem::path path);

std::string get_string_from_time_point(std::chrono::system_clock::time_point tp);
std::chrono::system_clock::time_point get_time_point(const std::string& timePoint);

std::shared_ptr<SQLite::Database> getOrCreateDb(boost::filesystem::path dbPath, const char* initScript);
std::shared_ptr<IFileRepositoryDB> getFileRepository(boost::program_options::variables_map& vm);

class Resource
{
public:
	static std::string ReadResource(boost::filesystem::path dir);
};