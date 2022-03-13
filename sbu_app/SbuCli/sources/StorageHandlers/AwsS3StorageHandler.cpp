#include "StorageHandlers/AwsS3StorageHandler.h"
#include "Loggers.h"
#include "utils.h"

#include <aws/core/Aws.h>
#include "aws/s3/S3Client.h"
#include "aws/s3/model/PutObjectRequest.h"
#include "aws/s3/model/GetObjectRequest.h"
#include "aws/core/auth/AWSCredentialsProvider.h"
#include "aws/transfer/TransferManager.h"

static std::shared_ptr<ILogger> logger = LoggerFactory::getLogger("application.AwsS3StorageHandler");

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

	logger->InfoFormat(
		"AwsS3StorageHandler::AwsS3StorageHandler() region:[%s] bucket:[%s] basePath:[%s] key:[%s]",
		region.c_str(),
		bucket.c_str(),
		basePath.c_str(),
		key.c_str()
	);

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
	bool retValue = false;
	auto transferHandle = transferManager->UploadFile(
		srcFilePath.string().c_str(), 
		this->bucket.c_str(), 
		handle.c_str(),
		"text/plain", 
		Aws::Map<Aws::String, Aws::String>());

	transferHandle->WaitUntilFinished();

	if (transferHandle->GetStatus() == Aws::Transfer::TransferStatus::COMPLETED)
		retValue = true;

	logger->InfoFormat(
		"AwsS3StorageHandler::CopyFileToRepository() handle:[%s] srcFilePath:[%s] success:[%d]", 
		handle.c_str(), 
		srcFilePath.string().c_str(), 
		retValue
	);
	return retValue;
}

bool AwsS3StorageHandler::CopyFileFromRepository(const IFileRepositoryDB::RepoHandle& handle, boost::filesystem::path dstFilePath)
{
	bool retValue = false;

	auto transferHandle = transferManager->DownloadFile(
		this->bucket.c_str(),
		handle.c_str(),
		dstFilePath.string().c_str());

	transferHandle->WaitUntilFinished();

	if (transferHandle->GetStatus() == Aws::Transfer::TransferStatus::COMPLETED)
		retValue = true;

	logger->InfoFormat(
		"AwsS3StorageHandler::CopyFileFromRepository() handle:[%s] dstFilePath:[%s] success:[%d]",
		handle.c_str(),
		dstFilePath.string().c_str(),
		retValue
	);

	return retValue;
}
