#pragma once
#include "Poco/Zip/ZipManipulator.h"
#include "Loggers.h"

#include "Poco/Types.h"
#include <boost/filesystem.hpp>

class MultiFile
{
public:
	MultiFile();
	virtual ~MultiFile();
	Poco::UInt64 GetSize();
	bool AddFile(boost::filesystem::path file, const std::string& digest);
	bool HasFile(const std::string& digest);
	bool Close();
public:
	struct fileEntry
	{
		boost::filesystem::path file;
		std::string digest;
		long long size;
	};

	Poco::UInt64 totalSize;
	Poco::UInt64 fileSize;

	std::string zipFile;
	std::map<std::string, fileEntry> entries;
	std::shared_ptr <Poco::Zip::ZipManipulator> zip;
	std::shared_ptr<ILogger> logger;
};
