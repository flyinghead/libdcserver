cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(dcserver LANGUAGES C CXX)
include(GNUInstallDirs)

find_package(CURL REQUIRED)

add_library(dcserver-static STATIC)
add_library(dcserver SHARED)

target_compile_options(dcserver-static PRIVATE -Wall)
target_compile_options(dcserver PRIVATE -Wall)
target_compile_definitions(dcserver-static PRIVATE DATADIR="${CMAKE_INSTALL_FULL_DATAROOTDIR}/dcnet")
target_compile_definitions(dcserver PRIVATE DATADIR="${CMAKE_INSTALL_FULL_DATAROOTDIR}/dcnet")

set(DCSER_HEADERS
	include/discord.h
	include/json.hpp
	include/status.h
	include/strprintf.hpp)

set(DCSER_SOURCE
	src/discord.cpp
	src/status.cpp)

target_include_directories(dcserver-static PRIVATE include)
target_include_directories(dcserver PUBLIC PRIVATE include)
target_sources(dcserver-static PRIVATE ${DCSER_SOURCE})
target_sources(dcserver PRIVATE ${DCSER_SOURCE})

target_link_libraries(dcserver-static PRIVATE CURL::libcurl)
target_link_libraries(dcserver PRIVATE CURL::libcurl)

set_target_properties(dcserver-static PROPERTIES OUTPUT_NAME "dcserver" ) 

set_target_properties(dcserver PROPERTIES PUBLIC_HEADER "${DCSER_HEADERS}")

install(TARGETS dcserver PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/dcserver")
install(TARGETS dcserver-static)
install(
	FILES "${CMAKE_SOURCE_DIR}/share/games.json"
	DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/dcnet")
