file(GLOB_RECURSE cmakes "${PROJECT_SOURCE_DIR}/CMakeLists.cmake")

find_package(Boost REQUIRED COMPONENTS filesystem program_options log)
find_package(SQLiteCpp CONFIG REQUIRED)
find_package(sqlite3 CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Poco REQUIRED Foundation Util Crypto)

#set(Boost_USE_STATIC_LIBS TRUE)
#set(OPENSSL_USE_STATIC_LIBS TRUE)

file(GLOB         resources		"resources/*.sql")
file(GLOB		  resources_db	"resources/databases/*.sql")
file(GLOB_RECURSE operations	"sources/Operations/*.cpp")
file(GLOB_RECURSE databases		"sources/Databases/*.cpp")
file(GLOB sources				"sources/*.cpp")
file(GLOB_RECURSE include		"include/*.h")
file(GLOB_RECURSE external		"headers/*.h")

include_directories("include")
include_directories("headers")

source_group("Headers" FILES ${external})
source_group("Includes" FILES ${include})
source_group("Resources" FILES ${resources})
source_group("Resources\\Databases" FILES ${resources_db})
source_group("BuildSystem" ${cmakes})
source_group("Sources" FILES ${sources})
source_group("Sources\\Operations" FILES ${operations})
source_group("Sources\\Databases" FILES ${databases})

add_executable (sbu ${sources} ${include} ${external} ${cmakes} ${resources} ${operations} ${databases} ${resources_db})

target_link_libraries (sbu LINK_PUBLIC SQLiteCpp sqlite3 OpenSSL::SSL OpenSSL::Crypto ${Boost_LIBRARIES} Poco::Foundation Poco::Util Poco::Crypto)

