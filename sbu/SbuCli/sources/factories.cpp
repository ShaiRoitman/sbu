#include "factories.h"

std::shared_ptr<IFileRepositoryDB> getFileRepository(boost::program_options::variables_map& vm)
{
	std::shared_ptr<IFileRepositoryDB> fileRepDB;
	boost::filesystem::path repoPath = vm["FileRepository.path"].as<std::string>();
	boost::filesystem::path dbPath = vm["FileRepository.name"].as<std::string>();

	if (!vm["FileRepository.password"].empty())
	{
		fileRepDB = CreateSecureFileRepositorySQLiteDB(dbPath, repoPath, vm["FileRepository.password"].as<std::string>());
	}
	else
	{
		fileRepDB = CreateFileRepositorySQLiteDB(dbPath, repoPath);
	}
	
	return fileRepDB;
}

std::shared_ptr<IRepositoryDB> getRepository(boost::program_options::variables_map& vm)
{
	std::string repoDB = vm["Repository.path"].as<std::string>();
	auto RepoDB = CreateRepositorySQLiteDB(repoDB);;

	return RepoDB;
}
