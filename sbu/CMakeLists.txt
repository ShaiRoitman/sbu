cmake_minimum_required (VERSION 2.8.11)
project (SmartBackupUtility)

file(GLOB_RECURSE cmakes "${PROJECT_SOURCE_DIR}/*.cmake")

#source_group("BuildSystem" ${cmakes})

set (BOOST_BASE_DIR "C:/git/boost/boost_1_65_0")
set (BOOST_INCLUDE ${BOOST_BASE_DIR})
set (BOOST_LIBS "${BOOST_BASE_DIR}/stage/lib")

add_subdirectory (SbuCli)
