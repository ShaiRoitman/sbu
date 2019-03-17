#include "FileRepositoryDB.h"
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

static auto logger = LoggerFactory::getLogger("application.FileRepositoryDB");

class FileRepositoryDB : public IFileRepositoryDB
{
public:
	FileRepositoryDB(boost::filesystem::path dbPath, boost::filesystem::path dataRootPath)
	{
		this->dataRootPath = dataRootPath;
		this->db = getOrCreateDb(dbPath, Text_Resource::FileRepository);
		logger->DebugFormat("FileRepositoryDB::FileRepositoryDB() path:[%s] root:[%s]", to_utf8(dbPath).c_str(), to_utf8(dataRootPath).c_str());
	}

	virtual std::string AddFile(boost::filesystem::path file, const std::string& digestType, const std::string& digest) override
	{
		logger->DebugFormat("FileRepositoryDB::AddFile() path:[%s] digestType:[%s] digest:[%s]", file.string().c_str(), digestType.c_str(), digest.c_str());

		std::string key = digest;
		SQLite::Statement query(*db, "SELECT Path FROM Files WHERE DigestValue=:key");
		query.bind(":key", key);
		if (!query.executeStep())
		{
			logger->DebugFormat("FileRepositoryDB::AddFile() key [%s] is missing -> adding", key.c_str());
			SQLite::Statement insertQuery(*db, "INSERT INTO Files (Path, Size, Added, DigestType, DigestValue) VALUES (:path,:size,:added,:digestType,:digestValue)");
			insertQuery.bind(":path", to_utf8(key));
			insertQuery.bind(":size", (long long)boost::filesystem::file_size(file));
			insertQuery.bind(":added", return_current_time_and_date());
			insertQuery.bind(":digestType", digestType);
			insertQuery.bind(":digestValue", key);
			insertQuery.exec();

			path destPath = this->dataRootPath / boost::filesystem::path(digest);
			boost::filesystem::create_directories(destPath.branch_path());
			if (copy_file_logged(file, destPath) == false)
			{
				logger->ErrorFormat("FileRepositoryDB::AddFile() key [%s] Failed", key.c_str());
			}
		}
		else
		{
			logger->DebugFormat("FileRepositoryDB::AddFile() key [%s] exists", key.c_str());
		}

		return key;
	}

	virtual bool GetFile(const std::string& handle, boost::filesystem::path outFilePath) override
	{
		logger->DebugFormat("FileRepositoryDB::GetFile() handle:[%s] destination:[%s]", 
			handle.c_str(),
			outFilePath.string().c_str());

		SQLite::Statement query(*db, "SELECT Path FROM Files WHERE DigestValue=:key");
		query.bind(":key", handle);
		bool retValue = false;
		if (query.executeStep())
		{
			path srcPath = this->dataRootPath / from_utf8(query.getColumn("Path").getString());
			boost::filesystem::create_directories(outFilePath.branch_path());
			if (copy_file_logged(srcPath, outFilePath) == false)
			{
				logger->WarningFormat("FileRepositoryDB::GetFile() handle:[%s] destination:[%s] Failed",
					handle.c_str(),
					outFilePath.string().c_str());
			}
		}
		else
		{
			logger->WarningFormat("FileRepositoryDB::GetFile() handle:[%s] destination:[%s] Failed Missing in repository",
				handle.c_str(),
				outFilePath.string().c_str());
		}
		
		return retValue;
	}

	virtual void Complete() override
	{
	}

private:
	std::shared_ptr<SQLite::Database> db;
	path dataRootPath;
};

void PocoEncryptFile(boost::filesystem::path source, boost::filesystem::path dest, Cipher* pCipher)
{
	Poco::FileOutputStream sink(dest.string());
	CryptoOutputStream encryptor(sink, pCipher->createEncryptor());

	Poco::FileInputStream fileSource(source.string());
	Poco::StreamCopier::copyStream(fileSource, encryptor);

	// Always close output streams to flush all internal buffers
	encryptor.close();
	sink.close();
}

void PocoDecryptFile(boost::filesystem::path source, boost::filesystem::path dest, Cipher* pCipher)
{
	Poco::FileOutputStream sink(dest.string());
	CryptoOutputStream decryptor(sink, pCipher->createDecryptor());

	Poco::FileInputStream fileSource(source.string());
	Poco::StreamCopier::copyStream(fileSource, decryptor);

	// Always close output streams to flush all internal buffers
	decryptor.close();
	sink.close();
}


class SecureFileRepositoryDB : public FileRepositoryDB
{
public:
	SecureFileRepositoryDB(boost::filesystem::path dbPath, boost::filesystem::path dataRootPath) : FileRepositoryDB(dbPath, dataRootPath)
	{
	}
	
	virtual std::string AddFile(boost::filesystem::path file, const std::string& digestType, const std::string& digest) override
	{
		CipherFactory& factory = CipherFactory::defaultFactory();
		// Creates a 256-bit AES cipher
		Cipher* pCipher = factory.createCipher(CipherKey("aes-256","Shai"));

		std::string secureFile;
		PocoEncryptFile(file, secureFile, pCipher);
		std::string secureDigest = pCipher->encryptString(digest);
		std::string secureDigestType = "Secure_" + digestType;
		return FileRepositoryDB::AddFile(secureFile, secureDigestType, secureDigest);
	}

	virtual bool GetFile(const std::string& handle, boost::filesystem::path outFilePath) override
	{
		return FileRepositoryDB::GetFile(handle, outFilePath);
	}
};

std::shared_ptr<IFileRepositoryDB> CreateFileRepositorySQLiteDB(boost::filesystem::path dbPath, boost::filesystem::path dataRootPath)
{
	logger->DebugFormat("Creating FileRepositoryDB dbPath:[%s] dataRootPath:[%s]", dbPath.string().c_str(), dataRootPath.string().c_str());
	return std::make_shared<FileRepositoryDB>(dbPath, dataRootPath);
}