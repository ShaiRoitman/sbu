#include "MultiFile.h"
#include "Poco/TemporaryFile.h"
#include "Poco/Zip/Compress.h"

static auto logger = LoggerFactory::getLogger("application.MultiFile");

MultiFile::MultiFile()
{
	this->totalSize = 0;
	this->fileSize = 0;
	this->zipFile = Poco::TemporaryFile::tempName();
	logger->DebugFormat("MultiFile::MultiFile() using filename [%s]", this->zipFile.c_str());
	boost::filesystem::remove(this->zipFile);
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
	bool retValue = false;
	if (this->zip != nullptr)
	{
		zip->commit();
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
		logger->DebugFormat("MultiFile::AddFile() filename:[%s], path:[%s] digest:[%s] Already Exists", this->zipFile.c_str(), file.string(), digest);
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

	logger->DebugFormat("MultiFile::AddFile() filename:[%s], path:[%s] digest:[%s] Size:[%ld]", this->zipFile.c_str(), file.string(), digest, newEntry.size);
	return true;
}
bool MultiFile::HasFile(const std::string& digest)
{
	bool retValue = false;
	if (entries.find(digest) != entries.end())
	{
		retValue = true;
	}

	logger->DebugFormat("MultiFile::HasFile() filename [%s], digest [%s] retValue ", this->zipFile.c_str(), digest, retValue);
	return retValue;
}

