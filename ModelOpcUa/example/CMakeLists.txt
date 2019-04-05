
cmake_minimum_required (VERSION 3.12)
find_package(GTest REQUIRED NO_CMAKE_SYSTEM_PATH)

add_executable(BasicUsage "BasicUsage.cpp" "ExampleModels.cpp")
add_test(BasicUsage BasicUsage)
target_link_libraries(BasicUsage ModelOpcUaLib)
target_link_libraries(BasicUsage GTest::gmock_main)
