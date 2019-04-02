#include <memory>
#include <stdio.h>
#include <iostream>
#include <map>

#include "CommandLineAndConfig.h"
#include "boost/filesystem.hpp"
#include "Loggers.h"
#include "Operations.h"
#include "ExitCodes.h"

#include "Poco/Zip/Compress.h"
#include "Poco/Zip/ZipManipulator.h"
#include <iostream>

class MultiFile
{
public:
	MultiFile()
	{
		this->totalSize = 0;
		this->fileSize = 0;
		this->zipFile = "c:\\workdir\\test.zip";
		boost::filesystem::remove(this->zipFile);
	}

	struct fileEntry
	{
		boost::filesystem::path file;
		std::string digest;
		long long size;
	};

	long totalSize;
	long fileSize;

	std::string zipFile;
	std::map<std::string, fileEntry> entries;

	bool AddFile(boost::filesystem::path file, const std::string& digest)
	{
		if (this->HasFile(digest))
			return false;
		fileEntry newEntry;
		newEntry.digest = digest;
		newEntry.file = file;
		newEntry.size = (long long)boost::filesystem::file_size(file);
		entries[digest] = newEntry;
		if (boost::filesystem::exists(zipFile) == false)
		{
			std::ofstream out(zipFile, std::ios::binary);
			Poco::Zip::Compress c(out, true);
			c.close();
		}
		Poco::Zip::ZipManipulator zip(zipFile, false);
		zip.addFile(digest, file.string());
		auto zipFile = zip.commit();
		auto newEntryZip = zipFile.findHeader(digest);
		this->fileSize += newEntryZip->second.getCompressedSize();
		this->totalSize += newEntry.size;
	}

	bool HasFile(const std::string& digest)
	{
		if (entries.find(digest) != entries.end())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	void Test()
	{
		this->AddFile("C:\\workdir\\backupDB.db", "Shai");
		this->AddFile("C:\\workdir\\backupDB.db", "Shai");
		this->AddFile("C:\\workdir\\Repository.db", "Shai2");
	}
};

int main(int argc, const char* argv[])
{
	MultiFile a;
	a.Test();
	std::map<std::string, Operation::Factory> operations;
	operations["CreateBackupDef"] = CreateBackupDefFactory;
	operations["ListBackupDef"] = ListBackupDefsFactory;
	operations["Backup"] = BackupFactory;
	operations["Restore"] = RestoreFactory;
	operations["ListBackup"] = ListBackupsFactory;
	operations["BackupInfo"] = BackupInfoFactory;

	CommandLineAndOptions options;
	int retValue = options.ParseOptions(argc, argv);
	LoggerFactory::InitLogger(options.vm);
	static auto logger = LoggerFactory::getLogger("application");

	logger->Info("Application Started");

	if (!options.vm["General.Workdir]"].empty())
	{
		auto workDir = options.vm["General.Workdir]"].as<std::string>();
		boost::filesystem::current_path(workDir);
	}
	logger->InfoFormat("Working Directory [%s]", boost::filesystem::current_path().string().c_str());

	if ( retValue == ExitCode_Success )
	{
		std::string action = options.vm["action"].as<std::string>();
		logger->DebugFormat("Application action:[%s]", action.c_str());

		if (operations.find(action) != operations.end())
		{
			try {
				auto factory = operations[action]();
				retValue = factory->Operate(options.vm);
			}
			catch (std::exception ex)
			{
				logger->ErrorFormat("Fail to run action:[%s] got exception:[%s]", action.c_str(), ex.what());
				retValue = ExitCode_GeneralFailure;
			}
		}
		else
		{
			logger->ErrorFormat("Fail to run missing action:[%s]", action.c_str());
			retValue = ExitCode_InvalidAction;
		}
	}

	logger->InfoFormat("Application Ended retValue:[%d]", retValue);
	return retValue;
}
