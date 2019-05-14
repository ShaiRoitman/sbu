#include "factories.h"
#include "FileRepositoryDB.h"
#include "FileRepositoryStorageHandler.h"
#include "AwsS3StorageHandler.h"

std::shared_ptr<IFileRepositoryDB> getFileRepository(boost::program_options::variables_map& vm)
{
	std::shared_ptr<IFileRepositoryDB> fileRepDB;
	std::shared_ptr<IStorageHandler> storageHander;

	boost::filesystem::path dbPath = vm["FileRepository.name"].as<std::string>();
	long minSizeToBulk = vm["FileRepository.minSizeToBulk"].as<long>();
	long bulkSize = vm["FileRepository.bulkSize"].as<long>();

	auto repoType = vm["General.StorageType"].as<std::string>();

	if (repoType == "FileRepository" || repoType == "SecureFileRepository")
	{
		boost::filesystem::path repoPath = vm["FileRepository.path"].as<std::string>();
		storageHander = std::make_shared<FileSystemStorageHandler>(repoPath);
	}
	else if (repoType == "AwsS3" || repoType == "SecureAwsS3")
	{
		std::string region = vm["AwsS3Storage.region"].as<std::string>();
		std::string bucket = vm["AwsS3Storage.bucket"].as<std::string>();
		std::string basePath = vm["AwsS3Storage.path"].as<std::string>();
		std::string key = vm["AwsS3Storage.key"].as<std::string>();
		std::string secret = vm["AwsS3Storage.secret"].as<std::string>();
		storageHander = std::make_shared<AwsS3StorageHandler>(region, bucket, basePath, key, secret);
	}
	else
	{
		return nullptr;
	}

	auto isSecure = (repoType == "SecureFileRepository") || (repoType == "SecureAwsS3");

	if (!vm["Storage.password"].empty() && isSecure)
	{
		fileRepDB = CreateSecureFileRepositorySQLiteDB(storageHander, dbPath, vm["Storage.password"].as<std::string>(), minSizeToBulk, bulkSize);
	}
	else
	{
		fileRepDB = CreateFileRepositorySQLiteDB(storageHander, dbPath, minSizeToBulk, bulkSize);
	}
	
	return fileRepDB;
}

std::shared_ptr<IRepositoryDB> getRepository(boost::program_options::variables_map& vm)
{
	std::string repoDB = vm["Repository.path"].as<std::string>();
	auto RepoDB = CreateRepositorySQLiteDB(repoDB);;

	return RepoDB;
}
