#include "ZipWrapper.h"
#include "Poco/Zip/Compress.h"
#include "Poco/Zip/ZipManipulator.h"
#include "Poco/Zip/ZipStream.h"
#include "Poco/StreamCopier.h"

static auto logger = LoggerFactory::getLogger("application.ZipWrapper");

ZipWrapper::ZipWrapper(const std::string& fileName) 
{
	this->zipArchiveStream = std::make_shared<std::ifstream>(fileName, std::ios::binary);
	this->zipArchiveStreamName = fileName;
	this->zipArchive = std::make_shared<Poco::Zip::ZipArchive>(*this->zipArchiveStream);
	logger->DebugFormat("ZipWrapper::ZipWrapper() using filename:[%s]", fileName);
}

ZipWrapper::~ZipWrapper()
{
	logger->DebugFormat("ZipWrapper::~ZipWrapper() filename:[%s]", this->zipArchiveStreamName);
}

bool ZipWrapper::ExtractFile(const std::string& handle, const std::string& path)
{
	bool retValue = false;
	logger->DebugFormat("ZipWrapper::ExtractFile() filename:[%s] Extract handle:[%s] path:[%s]", this->zipArchiveStreamName, handle, path);
	try {
		Poco::Zip::ZipArchive::FileHeaders::const_iterator it = this->zipArchive->findHeader(handle);
		Poco::Zip::ZipInputStream zipin(*this->zipArchiveStream, it->second);
		std::ofstream out(path, std::ios::binary);
		Poco::StreamCopier::copyStream(zipin, out);
		out.close();
		retValue = true;
	}
	catch (std::exception ex)
	{
		logger->ErrorFormat("ZipWrapper::ExtractFile() Extract handle:[%s] path:[%s] Failed exception:[%s]",
			handle, path, ex.what());
	}
	return retValue;
}

bool ZipWrapper::Close()
{
	bool retValue = false;
	if (zipArchiveStream != nullptr)
	{
		zipArchiveStream->close();
		zipArchiveStream = nullptr;
	}

	if (zipArchive != nullptr)
	{
		zipArchive = nullptr;
	}

	logger->DebugFormat("ZipWrapper::Close() filename:[%s] closing:[%d]", zipArchiveStreamName, retValue);
	return retValue;
}
