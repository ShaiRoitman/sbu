include(${PROJECT_SOURCE_DIR}/CMakeInclude.cmake)

include_directories(${BOOST_INCLUDE})
link_directories(${BOOST_LIBS})

add_executable (sbu ${sources} ${headers} ${external} ${cmakefile})

target_link_libraries (sbu LINK_PUBLIC)