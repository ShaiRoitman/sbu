#include "FileRepositoryDBImpl.h"
#include "utils.h"

#include "Poco/Crypto/Cipher.h"
#include "Poco/Crypto/CipherFactory.h"
#include "Poco/Crypto/CipherKey.h"
#include "Poco/FileStream.h"
#include "Poco/Crypto/CryptoStream.h"
#include "Poco/StreamCopier.h"

using namespace Poco::Crypto;
using namespace boost::filesystem;

static void PocoTransform(boost::filesystem::path source, boost::filesystem::path dest, Poco::Crypto::CryptoTransform* transform)
{
	Poco::FileOutputStream sink(dest.string());
	CryptoOutputStream encryptor(sink, transform);

	Poco::FileInputStream fileSource(source.string());
	Poco::StreamCopier::copyStream(fileSource, encryptor);

	// Always close output streams to flush all internal buffers
	encryptor.close();
	sink.close();
}
static void PocoEncryptFile(boost::filesystem::path source, boost::filesystem::path dest, Cipher* pCipher)
{
	PocoTransform(source, dest, pCipher->createEncryptor());
}
static void PocoDecryptFile(boost::filesystem::path source, boost::filesystem::path dest, Cipher* pCipher)
{
	PocoTransform(source, dest, pCipher->createDecryptor());
}

static std::shared_ptr<ILogger> logger = LoggerFactory::getLogger("application.SecureFileRepositoryDB");

class SecureFileRepositoryDB : public FileRepositoryDB
{
public:
	SecureFileRepositoryDB(std::shared_ptr<IStorageHandler> fileHandler, boost::filesystem::path dbPath, const std::string& password)
		: FileRepositoryDB(fileHandler, dbPath, LLONG_MAX, LLONG_MAX)
	{
		CipherFactory& factory = CipherFactory::defaultFactory();
		pCipher = factory.createCipher(CipherKey("aes-256-cbc", password, "SecureFileRepositoryDB"));
		this->password = password;
	}

	virtual bool CopyFileToRepository(const RepoHandle& handle, boost::filesystem::path filePath)
	{
		std::string secureFile = boost::filesystem::unique_path().string();
		PocoEncryptFile(filePath, secureFile, pCipher);
		std::string secureDigest = calcHash(handle + this->password);
		std::string secureDigestType = "Secure_SHA1";

		auto retValue = this->fileHandler->CopyFileToRepository(secureDigest, secureFile);

		boost::filesystem::remove(secureFile);
		logger->InfoFormat("SecureFileRepositoryDB::CopyFileToRepository() Handle:[%s] FilePath:[%s]", handle.c_str(), filePath.string().c_str());
		return retValue;
	}

	virtual bool CopyFileFromRepository(const RepoHandle& handle, boost::filesystem::path filePath)
	{
		std::string secureFile = boost::filesystem::unique_path().string();
		std::string secureDigest = calcHash(handle + this->password);
		auto retValue = this->fileHandler->CopyFileFromRepository(secureDigest, secureFile);
		PocoDecryptFile(secureFile, filePath, pCipher);

		boost::filesystem::remove(secureFile);
		logger->InfoFormat("SecureFileRepositoryDB::CopyFileToRepository() Handle:[%s] FilePath:[%s]", handle.c_str(), filePath.string().c_str());
		return retValue;
	}

private:
	Cipher* pCipher;
	std::string password;
};

std::shared_ptr<IFileRepositoryDB> CreateSecureFileRepositoryDB(
	std::shared_ptr<IStorageHandler> storageHander,
	boost::filesystem::path dbPath,
	const std::string& password,
	long minSizeToBulk,
	long bulkSize)
{
	logger->DebugFormat("Creating SecureFileRepositoryDB dbPath:[%s] ", dbPath.string().c_str());
	return std::make_shared<SecureFileRepositoryDB>(
		storageHander, 
		dbPath,
		password);
}