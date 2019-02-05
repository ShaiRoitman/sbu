include(${PROJECT_SOURCE_DIR}/CMakeInclude.cmake_include)

file(GLOB_RECURSE cmakes "${PROJECT_SOURCE_DIR}/CMakeLists.cmake")


find_package(Boost REQUIRED COMPONENTS filesystem program_options)
find_package(SQLiteCpp CONFIG REQUIRED)
find_package(sqlite3 CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

file(GLOB_RECURSE resources "resources/*.sql")

source_group("Resources" FILES ${resources})
source_group("BuildSystem" ${cmakes})

add_executable (sbu ${sources} ${include} ${external} ${cmakes} ${resources})

target_link_libraries (sbu LINK_PUBLIC SQLiteCpp sqlite3 OpenSSL::SSL OpenSSL::Crypto ${Boost_LIBRARIES})

