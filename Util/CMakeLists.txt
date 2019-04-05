cmake_minimum_required(VERSION 3.9)

add_library(Util ConfigureLogger.cpp)
target_link_libraries(Util easyloggingpp::easyloggingpp)
target_include_directories(Util PUBLIC .)
target_compile_definitions(Util PUBLIC ELPP_DEFAULT_LOGGER="DashboardOpcUaClient")
