set (POCO_DEP PocoZip PocoFoundation PocoUtil PocoCrypto PocoNet)
set (AWS_DEP aws-cpp-sdk-s3 aws-cpp-sdk-core aws-cpp-sdk-transfer)
set (SBU_PLATFORM_LIBS boost_filesystem boost_program_options aws-checksums dl pthread)

link_directories("/home/master/3rdParty/boost_1_70_0/stage/lib")
link_directories("/home/master/3rdParty/poco/rel/lib")
link_directories("/home/master/3rdParty/aws-sdk-cpp/aws-cpp-sdk-s3")
link_directories("/home/master/3rdParty/aws-sdk-cpp/aws-cpp-sdk-transfer")
link_directories("/home/master/3rdParty/aws-sdk-cpp/aws-cpp-sdk-core")