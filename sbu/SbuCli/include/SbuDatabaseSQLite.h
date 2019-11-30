#pragma once

#include "SbuDatabase.h"
#include <boost/filesystem.hpp>

std::shared_ptr<ISbuDBDatabase> CreateSQLiteDB(boost::filesystem::path dbPath, const char* initScript);
