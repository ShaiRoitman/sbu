#pragma once
#include "Poco/Zip/ZipArchive.h"
#include "Loggers.h"
#include "Poco/Zip/ZipManipulator.h"

#include <string>
#include <iostream>
#include <ostream>
#include <fstream>
#include <sstream>
#include <memory>

class ZipWrapper
{
public:
	ZipWrapper(const std::string& fileName);
	virtual ~ZipWrapper();
	bool ExtractFile(const std::string& handle, const std::string& path);
	bool Close();

	std::shared_ptr<Poco::Zip::ZipArchive> zipArchive;
	std::shared_ptr <std::ifstream> zipArchiveStream;
	std::string zipArchiveStreamName;
};
