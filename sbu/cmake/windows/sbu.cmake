set (POCO_DEP Poco::Zip Poco::Foundation Poco::Crypto Poco::Util Poco::Net)
set (AWS_DEP aws-cpp-sdk-s3 aws-cpp-sdk-core aws-cpp-sdk-transfer)
set (SBU_PLATFORM_LIBS)

set (SQLITELIB sqlite3)
find_package(sqlite3 CONFIG REQUIRED)