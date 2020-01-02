
set (POCO_DEP PocoZip PocoFoundation PocoUtil PocoCrypto PocoNet)

set (AWS_DEP aws-cpp-sdk-s3 aws-cpp-sdk-core aws-cpp-sdk-transfer)

set (SBU_PLATFORM_LIBS boost_filesystem boost_program_options dl pthread)

include_directories("/git/SQLiteCpp/include")

link_directories("/git/boost_1_70_0/stage/lib")
link_directories("/git/poco/releaseBuild/lib")
link_directories("/git/aws-sdk-cpp/release/aws-cpp-sdk-s3")
link_directories("/git/aws-sdk-cpp/release/aws-cpp-sdk-transfer")
link_directories("/git/aws-sdk-cpp/release/aws-cpp-sdk-core")

