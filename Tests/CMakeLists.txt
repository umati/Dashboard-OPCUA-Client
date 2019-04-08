cmake_minimum_required(VERSION 3.9)
enable_testing()

# NO_CMAKE_PATH prevents the findGTest-Skirpt from the cmake installation
find_package(GTest REQUIRED NO_CMAKE_PATH)

add_executable(TestOpcUaClient TestOpcUaClient.cpp)
target_link_libraries(TestOpcUaClient OpcUaClientLib GTest::gtest_main)
add_test(
	NAME TestOpcUaClient
	COMMAND TestOpcUaClient
	WORKING_DIRECTORY $<TARGET_FILE_DIR:TestOpcUaClient>
)

add_executable(TestConverter TestConverter.cpp)
target_link_libraries(TestConverter OpcUaClientLib GTest::gtest_main)
add_test(
	NAME TestConverter
	COMMAND TestConverter
	WORKING_DIRECTORY $<TARGET_FILE_DIR:TestConverter>
)
