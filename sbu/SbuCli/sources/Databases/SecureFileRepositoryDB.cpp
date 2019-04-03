#include "FileRepositoryDBImpl.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include "utils.h"

#include "Poco/Crypto/Cipher.h"
#include "Poco/Crypto/CipherFactory.h"
#include "Poco/Crypto/CipherKey.h"
#include "Poco/FileStream.h"
#include "Poco/Crypto/CryptoStream.h"
#include "Poco/StreamCopier.h"
using namespace Poco::Crypto;


using namespace boost::filesystem;
using namespace SQLite;

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

class SecureFileRepositoryDB : public FileRepositoryDB
{
public:
	SecureFileRepositoryDB(boost::filesystem::path dbPath, boost::filesystem::path dataRootPath, const std::string& password)
		: FileRepositoryDB(dbPath, dataRootPath, LLONG_MAX, LLONG_MAX)
	{
		CipherFactory& factory = CipherFactory::defaultFactory();
		pCipher = factory.createCipher(CipherKey("aes-256-cbc", password, "SecureFileRepositoryDB"));
	}

	virtual std::string AddFile(boost::filesystem::path file, const std::string& digestType, const std::string& digest) override
	{
		std::string secureFile = boost::filesystem::unique_path().string();
		PocoEncryptFile(file, secureFile, pCipher);
		std::string secureDigest = calcHash(digest);
		std::string secureDigestType = "Secure_" + digestType;

		auto retValue = FileRepositoryDB::AddFile(secureFile, secureDigestType, secureDigest);

		boost::filesystem::remove(secureFile);
		return retValue;
	}

	virtual bool GetFile(const std::string& handle, boost::filesystem::path outFilePath) override
	{
		boost::filesystem::path srcPath;
		bool retValue = false;
		if (this->HasFile(handle, &srcPath))
		{
			boost::filesystem::create_directories(outFilePath.branch_path());
			PocoDecryptFile(srcPath, outFilePath, pCipher);
			retValue = true;
		}

		return retValue;
	}

private:
	Cipher* pCipher;
};


std::shared_ptr<IFileRepositoryDB> CreateSecureFileRepositorySQLiteDB(boost::filesystem::path dbPath, boost::filesystem::path dataRootPath, const std::string& password)
{
	static auto logger = LoggerFactory::getLogger("application.SecureFileRepositoryDB");
	logger->DebugFormat("Creating SecureFileRepositoryDB dbPath:[%s] dataRootPath:[%s]", dbPath.string().c_str(), dataRootPath.string().c_str());
	return std::make_shared<SecureFileRepositoryDB>(dbPath, dataRootPath, password);
}