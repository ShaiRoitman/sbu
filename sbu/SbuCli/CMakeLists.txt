find_package(Boost REQUIRED COMPONENTS filesystem program_options log)
find_package(SQLiteCpp CONFIG REQUIRED)
find_package(sqlite3 CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Poco REQUIRED Foundation Util Crypto Zip Net)
find_package(aws-cpp-sdk-core CONFIG REQUIRED transfer)
find_package(aws-cpp-sdk-s3 CONFIG REQUIRED)
find_package(aws-cpp-sdk-transfer CONFIG REQUIRED)

file(GLOB		resources		"resources/*.sql")
file(GLOB		resources_db	"resources/databases/*.sql")
file(GLOB		cmakes			"${PROJECT_SOURCE_DIR}/CMakeLists.cmake")

set (resources_files ${resources} ${resources_db} ${cmakes})

file(GLOB_RECURSE external		"headers/*.h")
file(GLOB httpModelsHeaders		"httpModels/*.h")
set (headers_files ${external} ${httpModelsHeaders})

file(GLOB include					"include/*.h")
file(GLOB includestoragehandlers	"include/StorageHandlers/*.h")
file(GLOB includesOperations		"include/Operations/*.h")
set (include_files ${include} ${includestoragehandlers} ${includesOperations})

file(GLOB sources					"sources/*.cpp")
file(GLOB_RECURSE operations		"sources/Operations/*.cpp")
file(GLOB_RECURSE databases			"sources/Databases/*.cpp")
file(GLOB_RECURSE storagehandlers	"sources/StorageHandlers/*.cpp")
file(GLOB httpModelssources			"httpModels/*.cpp")
set (sources_files ${sources} ${operations} ${databases} ${storagehandlers} ${httpModelssources})

include_directories("include")
include_directories("headers")

source_group("Headers" FILES ${external})
source_group("Headers\\HttpModels" FILES ${httpModelsHeaders})

source_group("Includes" FILES ${include})
source_group("Includes\\StorageHandlers" FILES ${includestoragehandlers})
source_group("Includes\\Operations" FILES ${includesOperations})

source_group("Resources" FILES ${resources})
source_group("Resources\\Databases" FILES ${resources_db})
source_group("Resources\\BuildSystem" ${cmakes})

source_group("Sources" FILES ${sources})
source_group("Sources\\Operations" FILES ${operations})
source_group("Sources\\Databases" FILES ${databases})
source_group("Sources\\StorageHandlers" FILES ${storagehandlers})
source_group("Sources\\HttpModels" FILES ${httpModelssources})

include (${PROJECT_SOURCE_DIR}/cmake/${SBU_PLATFROM}/sbu.cmake)
add_executable (sbu ${headers_files} ${include_files} ${sources_files} ${resources_files})

set_property(TARGET sbu PROPERTY CXX_STANDARD 11)
message ( ${Boost_LIBRARIES} )

target_link_libraries (sbu LINK_PUBLIC SQLiteCpp sqlite3 ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} ${AWSSDK_PLATFORM_DEPS} ${POCO_DEP} ${AWS_DEP} ${SBU_PLATFORM_LIBS})
