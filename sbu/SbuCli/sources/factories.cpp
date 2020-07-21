#include "factories.h"
#include "sbu_exceptions.h"
#include "FileRepositoryDB.h"
#include "StorageHandlers/AwsS3StorageHandler.h"
#include "StorageHandlers/FileRepositoryStorageHandler.h"

static std::shared_ptr<ILogger> logger = LoggerFactory::getLogger("factories");

std::shared_ptr<IFileRepositoryDB> getFileRepository(boost::program_options::variables_map& vm)
{
	std::shared_ptr<IFileRepositoryDB> fileRepDB;
	std::shared_ptr<IStorageHandler> storageHander;

	boost::filesystem::path dbPath = vm["FileRepository.name"].as<std::string>();
	long maxSizeToBulk = vm["FileRepository.maxSizeToBulk"].as<long>();
	long bulkSize = vm["FileRepository.bulkSize"].as<long>();

	if (vm["General.StorageType"].empty())
	{
		logger->ErrorFormat("getFileRepository() Missing StorageType");
		throw sbu_missingParameter();
	}

	auto repoType = vm["General.StorageType"].as<std::string>();

	if (repoType == "FileRepository" || repoType == "SecureFileRepository")
	{
		boost::filesystem::path repoPath = vm["FileRepository.path"].as<std::string>();
		storageHander = std::make_shared<FileSystemStorageHandler>(repoPath);
		logger->InfoFormat("getFileRepository() return FileSystemHandler path:[%s]", repoPath.string().c_str());
	}
	else if (repoType == "AwsS3" || repoType == "SecureAwsS3")
	{
		std::string region = vm["AwsS3Storage.region"].as<std::string>();
		std::string bucket = vm["AwsS3Storage.bucket"].as<std::string>();
		std::string basePath = vm["AwsS3Storage.path"].as<std::string>();
		std::string key = vm["AwsS3Storage.key"].as<std::string>();
		std::string secret = vm["AwsS3Storage.secret"].as<std::string>();
		storageHander = std::make_shared<AwsS3StorageHandler>(region, bucket, basePath, key, secret);
		logger->InfoFormat("getFileRepository() return AWS S3 bucket:[%s]", bucket.c_str());
	}
	else
	{
		logger->ErrorFormat("getFileRepository() invalid repoType:[%s]", repoType.c_str());
		return nullptr;
	}

	auto isSecure = (repoType == "SecureFileRepository") || (repoType == "SecureAwsS3");

	if (!vm["Storage.password"].empty() && isSecure)
	{
		fileRepDB = CreateSecureFileRepositoryDB(storageHander, dbPath, vm["Storage.password"].as<std::string>(), maxSizeToBulk, bulkSize);
		logger->InfoFormat("getFileRepository() returning Encrypted and Obfuscated FileRepository");
	}
	else
	{
		fileRepDB = CreateFileRepositoryDB(storageHander, dbPath, maxSizeToBulk, bulkSize);
		logger->InfoFormat("getFileRepository() returning Clear FileRepository");
	}
	
	return fileRepDB;
}

std::shared_ptr<IRepositoryDB> getRepository(boost::program_options::variables_map& vm)
{
	std::string repoDB = vm["Repository.path"].as<std::string>();
	auto RepoDB = CreateRepositoryDB(repoDB);;

	return RepoDB;
}
