#include "AwsS3StorageHandler.h"
#include "Loggers.h"
#include "utils.h"

#include <aws/core/Aws.h>
#include "aws/s3/S3Client.h"
#include "aws/s3/model/PutObjectRequest.h"
#include "aws/s3/model/GetObjectRequest.h"
#include "aws/core/auth/AWSCredentialsProvider.h"
#include "aws/transfer/TransferManager.h"

static std::shared_ptr<ILogger> logger = LoggerFactory::getLogger("application.FileSystemStorageHandler");

AwsS3StorageHandler::AwsS3StorageHandler(
	std::string region,
	std::string bucket,
	std::string basePath,
	std::string key,
	std::string secret)
{
	this->basePath = basePath;
	this->bucket = bucket;
	this->region = region;
	this->key = key;
	this->secret = secret;

	Aws::SDKOptions awsoptions;
	Aws::InitAPI(awsoptions);

	this->executor = new Aws::Utils::Threading::PooledThreadExecutor(2);
	Aws::Auth::AWSCredentials credentials(key.c_str(), secret.c_str());
	Aws::Client::ClientConfiguration clientConfig;
	clientConfig.region = region.c_str(); 
	this->client = std::make_shared<Aws::S3::S3Client>(credentials, clientConfig);
	this->transferConfiguration = std::make_shared<Aws::Transfer::TransferManagerConfiguration>(this->executor);
	this->transferConfiguration->s3Client = this->client;
	this->transferManager = Aws::Transfer::TransferManager::Create(*this->transferConfiguration);
}

bool AwsS3StorageHandler::CopyFileToRepository(const IFileRepositoryDB::RepoHandle& handle, boost::filesystem::path srcFilePath)
{
	auto transferHandle = transferManager->UploadFile(
		srcFilePath.string().c_str(), 
		this->bucket.c_str(), 
		handle.c_str(),
		"text/plain", 
		Aws::Map<Aws::String, Aws::String>());

	transferHandle->WaitUntilFinished();

	if (transferHandle->GetStatus() == Aws::Transfer::TransferStatus::COMPLETED)
		return true;
	return false;
}

bool AwsS3StorageHandler::CopyFileFromRepository(const IFileRepositoryDB::RepoHandle& handle, boost::filesystem::path dstFilePath)
{
	auto transferHandle = transferManager->DownloadFile(
		this->bucket.c_str(),
		handle.c_str(),
		dstFilePath.string().c_str());

	transferHandle->WaitUntilFinished();

	if (transferHandle->GetStatus() == Aws::Transfer::TransferStatus::COMPLETED)
		return true;
	return false;
}
