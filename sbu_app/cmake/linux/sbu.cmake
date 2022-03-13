

set (GIT_ROOT "/home/master/git")
set (POCO_DEP PocoZip PocoFoundation PocoUtil PocoCrypto PocoNet)
set (SQLITELIB sqlite3)

set (AWS_DEP aws-cpp-sdk-s3 aws-cpp-sdk-core aws-cpp-sdk-transfer)

set (SBU_PLATFORM_LIBS boost_filesystem boost_program_options dl pthread)

include_directories("/git/SQLiteCpp/include")
message ("${GIT_ROOT}/boost_1_72_0/boost")
include_directories("${GIT_ROOT}/boost_1_72_0")

link_directories("${GIT_ROOT}/sqlite3-cmake")
link_directories("${GIT_ROOT}/boost_1_72_0/stage/lib")
link_directories("${GIT_ROOT}/poco/releaseBuild/lib")
link_directories("${GIT_ROOT}/aws-sdk-cpp/release/aws-cpp-sdk-s3")
link_directories("${GIT_ROOT}/aws-sdk-cpp/release/aws-cpp-sdk-transfer")
link_directories("${GIT_ROOT}/aws-sdk-cpp/release/aws-cpp-sdk-core")

