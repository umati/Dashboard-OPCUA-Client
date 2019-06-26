cmake_minimum_required(VERSION 3.9)

find_package(nlohmann_json 3.6.1 REQUIRED)

set(UTIL_SRC
	ConfigurationJsonFile.cpp
	ConfigureLogger.cpp
	)

add_library(Util ${UTIL_SRC})
target_link_libraries(Util PUBLIC easyloggingpp::easyloggingpp)
target_link_libraries(Util PUBLIC nlohmann_json::nlohmann_json)
target_include_directories(Util PUBLIC .)
target_compile_definitions(Util PUBLIC ELPP_DEFAULT_LOGGER="DashboardOpcUaClient")
