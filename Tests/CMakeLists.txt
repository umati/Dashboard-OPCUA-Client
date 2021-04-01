cmake_minimum_required(VERSION 3.9)
enable_testing()

message("### opcua_dashboardclient/Tests: loading tests.")

# NO_CMAKE_PATH prevents the findGTest-Skirpt from the cmake installation
find_package(GTest REQUIRED NO_CMAKE_PATH)

message("### opcua_dashboardclient/Tests: adding executables")

add_executable(TestOpcUaClient TestOpcUaClient.cpp)
target_link_libraries(TestOpcUaClient OpcUaClientLib GTest::gtest_main)
target_link_libraries(TestOpcUaClient OpcUaClientLib GTest::gmock_main)
add_test(
        NAME TestOpcUaClient
        COMMAND TestOpcUaClient
        WORKING_DIRECTORY $<TARGET_FILE_DIR:TestOpcUaClient>
)


add_executable(TestSubscription TestSubscription.cpp)
target_link_libraries(TestSubscription OpcUaClientLib GTest::gtest_main)
target_link_libraries(TestSubscription OpcUaClientLib GTest::gmock_main)
add_test(
        NAME TestSubscription
        COMMAND TestSubscription
        WORKING_DIRECTORY $<TARGET_FILE_DIR:TestSubscription>
)


add_executable(TestMachineObserver TestMachineObserver.cpp)
target_link_libraries(TestMachineObserver OpcUaClientLib GTest::gtest_main)
target_link_libraries(TestMachineObserver OpcUaClientLib GTest::gmock_main)
add_test(
        NAME TestMachineObserver
        COMMAND TestMachineObserver
        WORKING_DIRECTORY $<TARGET_FILE_DIR:TestMachineObserver>
)

add_executable(TestConverter TestConverter.cpp)
target_link_libraries(TestConverter OpcUaClientLib GTest::gtest_main)
add_test(
        NAME TestConverter
        COMMAND TestConverter
        WORKING_DIRECTORY $<TARGET_FILE_DIR:TestConverter>
)

add_executable(TestConfigurationJsonFile testconfigurationjsonfile.cpp)
target_link_libraries(TestConfigurationJsonFile Util GTest::gtest_main)
add_test(
        NAME TestConfigurationJsonFile
        COMMAND TestConfigurationJsonFile
        WORKING_DIRECTORY $<TARGET_FILE_DIR:TestConfigurationJsonFile>
)

add_executable(TestIdEncode TestIdEncode.cpp)
target_link_libraries(TestIdEncode OpcUaClientLib Util GTest::gtest_main)
add_test(
        NAME TestIdEncode
        COMMAND TestIdEncode
        WORKING_DIRECTORY $<TARGET_FILE_DIR:TestIdEncode>
)

set(CONFIG_TESTFILES
        data/Configuration.json
        data/Configuration2.json
        )
foreach (file_iterator ${CONFIG_TESTFILES})
    add_custom_command(TARGET TestConfigurationJsonFile POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${file_iterator}
            $<TARGET_FILE_DIR:TestConfigurationJsonFile>
            COMMENT "Copy Test Configuration"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            )
endforeach (file_iterator)
