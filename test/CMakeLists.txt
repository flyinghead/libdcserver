include(FetchContent)

set(INSTALL_GTEST OFF)
set(BUILD_GMOCK OFF)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/5554fcaab40f8814e6e277a56175683405f0cdd5.zip
  DOWNLOAD_EXTRACT_TIMESTAMP ON 
)
FetchContent_MakeAvailable(googletest)

add_executable(tests
	config_test.cpp
	db_test.cpp
	discord_test.cpp)
target_link_libraries(tests dcserver GTest::gtest_main sqlite3)
add_test(NAME tests COMMAND tests)
