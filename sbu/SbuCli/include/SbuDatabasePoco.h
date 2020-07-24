#pragma once

#include "SbuDatabase.h"
#include <boost/filesystem.hpp>

std::shared_ptr<ISbuDBDatabase> CreatePocoDatabase(const std::string& connectionString, const char* initScript);
