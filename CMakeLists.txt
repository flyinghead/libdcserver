cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(dcserver LANGUAGES C CXX)
option(BUILD_TEST "Builds unit tests" OFF)

include(GNUInstallDirs)
if(BUILD_TEST)
	include(CTest)
endif()
if(NOT DEFINED LOCALSTATEDIR)
	# CMAKE_INSTALL_FULL_LOCALSTATEDIR is /usr/local/var by default
	set(LOCALSTATEDIR "/var/local")
endif()

find_package(CURL REQUIRED)

add_library(dcserver SHARED)

target_compile_options(dcserver PRIVATE -Wall)
target_compile_definitions(dcserver PRIVATE
	DATADIR="${CMAKE_INSTALL_FULL_DATAROOTDIR}/dcnet"
	CONFDIR="${CMAKE_INSTALL_FULL_SYSCONFDIR}/dcnet"
	STATUSDIR="${LOCALSTATEDIR}/lib/dcnet/status")

set(DCSER_HEADERS
	include/asio.hpp
	include/database.hpp
	include/discord.h
	include/discord.hpp
	include/json.hpp
	include/shared_this.hpp
	include/status.h
	include/status.hpp
	include/strprintf.hpp)

set(DCSER_SOURCE
	src/config.cpp
	src/discord.cpp
	src/http.cpp
	src/status.cpp)

target_include_directories(dcserver PUBLIC PRIVATE include)
target_sources(dcserver PRIVATE ${DCSER_SOURCE})

target_link_libraries(dcserver PRIVATE CURL::libcurl pthread)

set_target_properties(dcserver PROPERTIES PUBLIC_HEADER "${DCSER_HEADERS}")

install(TARGETS dcserver PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/dcserver")
install(
	FILES "${CMAKE_SOURCE_DIR}/share/games.json"
	DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/dcnet")

if(BUILD_TEST)
	add_subdirectory(test)
endif()

