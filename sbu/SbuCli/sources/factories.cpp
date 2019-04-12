#include "factories.h"

std::shared_ptr<IFileRepositoryDB> getFileRepository(boost::program_options::variables_map& vm)
{
	std::shared_ptr<IFileRepositoryDB> fileRepDB;
	boost::filesystem::path repoPath = vm["FileRepository.path"].as<std::string>();
	boost::filesystem::path dbPath = vm["FileRepository.name"].as<std::string>();
	long minSizeToBulk = vm["FileRepository.minSizeToBulk"].as<long>();
	long bulkSize = vm["FileRepository.bulkSize"].as<long>();

	if (!vm["FileRepository.password"].empty())
	{
		fileRepDB = CreateSecureFileRepositorySQLiteDB(dbPath, repoPath, vm["FileRepository.password"].as<std::string>(), minSizeToBulk, bulkSize);
	}
	else
	{
		fileRepDB = CreateFileRepositorySQLiteDB(dbPath, repoPath, minSizeToBulk, bulkSize);
	}
	
	return fileRepDB;
}

std::shared_ptr<IRepositoryDB> getRepository(boost::program_options::variables_map& vm)
{
	std::string repoDB = vm["Repository.path"].as<std::string>();
	auto RepoDB = CreateRepositorySQLiteDB(repoDB);;

	return RepoDB;
}
