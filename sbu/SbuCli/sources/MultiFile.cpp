#include "MultiFile.h"
#include "Poco/TemporaryFile.h"
#include "Poco/Zip/Compress.h"
#include <iostream>
#include <fstream>

static auto logger = LoggerFactory::getLogger("application.MultiFile");

MultiFile::MultiFile()
{
	this->totalSize = 0;
	this->fileSize = 0;
	this->zipFile = Poco::TemporaryFile::tempName();
	logger->DebugFormat("MultiFile::MultiFile() using filename [%s]", this->zipFile.c_str());
	auto removeResult = boost::filesystem::remove(this->zipFile);
	logger->DebugFormat("MultiFile::MultiFile() using filename [%s] Removed:[%d]", this->zipFile.c_str(), removeResult);

	zip = nullptr;
}
MultiFile::~MultiFile()
{
	this->Close();
	logger->DebugFormat("MultiFile::~MultiFile() filename:[%s]", this->zipFile.c_str());
}
Poco::UInt64 MultiFile::GetSize()
{
	auto retValue = this->totalSize;
	logger->DebugFormat("MultiFile::GetSize() filename [%s]:currentSize:[%lld]", this->zipFile.c_str(), this->totalSize);
	return retValue;
}
bool MultiFile::Close()
{
	logger->DebugFormat("MultiFile::Close()");
	bool retValue = false;
	if (this->zip != nullptr)
	{
		try {
			zip->commit();
		}
		catch (std::exception ex)
		{
			logger->DebugFormat("MultiFile::Close() Failed to commit ex:[%s]", ex.what());
			throw;
		}
		zip = nullptr;
		retValue = true;
	}

	logger->DebugFormat("MultiFile::Close() filename:[%s] closing:[%d]", this->zipFile.c_str(), retValue);
	return retValue;
}
bool MultiFile::AddFile(boost::filesystem::path file, const std::string& digest)
{
	if (this->HasFile(digest))
	{
		logger->DebugFormat("MultiFile::AddFile() filename:[%s], path:[%s] digest:[%s] Already Exists", this->zipFile.c_str(), file.string().c_str(), digest.c_str());
		return false;
	}
	fileEntry newEntry;
	newEntry.digest = digest;
	newEntry.file = file;
	newEntry.size = (long long)boost::filesystem::file_size(file);
	entries[digest] = newEntry;
	if (boost::filesystem::exists(this->zipFile) == false && this->zip == nullptr)
	{
		logger->DebugFormat("MultiFile::AddFile() Creating new file path:[%s]", this->zipFile.c_str());
		std::ofstream out(zipFile, std::ios::binary);
		Poco::Zip::Compress c(out, true);
		c.close();
		if (this->zip != nullptr)
		{
			this->zip = nullptr;
		}
		this->zip = std::make_shared<Poco::Zip::ZipManipulator>(this->zipFile, false);
	}
	this->zip->addFile(digest, file.string());
	this->totalSize += newEntry.size;

	logger->DebugFormat("MultiFile::AddFile() filename:[%s], path:[%s] digest:[%s] Size:[%ld]", this->zipFile.c_str(), file.string().c_str(), digest.c_str(), newEntry.size);
	return true;
}
bool MultiFile::HasFile(const std::string& digest)
{
	bool retValue = false;
	if (entries.find(digest) != entries.end())
	{
		retValue = true;
	}

	logger->DebugFormat("MultiFile::HasFile() filename:[%s], digest:[%s] retValue:[%d]", this->zipFile.c_str(), digest.c_str(), retValue);
	return retValue;
}

