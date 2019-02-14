#include "factories.h"

std::shared_ptr<IFileRepositoryDB> getFileRepository(boost::program_options::variables_map& vm)
{
	boost::filesystem::path repoPath = vm["FileRepository.path"].as<std::string>();
	boost::filesystem::path dbPath = vm["FileRepository.name"].as<std::string>();
	std::shared_ptr<IFileRepositoryDB> fileRepDB = CreateFileRepositorySQLiteDB(dbPath, repoPath);
	return fileRepDB;
}

std::shared_ptr<IRepositoryDB> getRepository(boost::program_options::variables_map& vm)
{
	std::string repoDB = "repoDB.db";
	auto RepoDB = CreateRepositorySQLiteDB(repoDB);;

	return RepoDB;
}
