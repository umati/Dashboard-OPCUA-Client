cmake_minimum_required(VERSION 3.9)

message("### opcua_dashboardclient/DashboardClient: loading Dasbhoard client")

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
MESSAGE(STATUS "cmake module Path ${CMAKE_MODULE_PATH}")

###########
# Version #
###########

# The current version information. On the master branch, we take the version
# number from the latest release plus the "-undefined" label. Will be
# overwritten with more detailed information if git is available.
set(UMATI_CLIENT_VER_MAJOR 1)
set(UMATI_CLIENT_VER_MINOR 3)
set(UMATI_CLIENT_VER_PATCH 2)
set(UMATI_CLIENT_VER_LABEL "-undefined") # like "-rc1" or "-g4538abcd" or "-g4538abcd-dirty"
set(UMATI_CLIENT_VER_COMMIT "unknown-commit")

# Overwrite the version information based on git if available
include(SetGitBasedVersion)
set_umati_dashboard_client_version()

# Examples for the version string are:
# v1.2
# v1.2.3
# v1.2.3-rc1
# v1.2.3-rc1-dirty
# v1.2.3-5-g4538abcd
# v1.2.3-5-g4538abcd-dirty
set(UMATI_CLIENT_VERSION "v${UMATI_CLIENT_VER_MAJOR}.${UMATI_CLIENT_VER_MINOR}.${UMATI_CLIENT_VER_PATCH}${UMATI_CLIENT_VER_LABEL}")
MESSAGE(STATUS "umati Dashboard Client Version: ${UMATI_CLIENT_VERSION}")



find_package(nlohmann_json 3.6.1 REQUIRED)
find_package(open62541 REQUIRED)

set(DASHBOARDCLIENT_SRC "DashboardClient.cpp" "IDashboardDataClient.cpp" "OpcUaTypeReader.cpp"
                        "Converter/ModelToJson.cpp"
)

message("### opcua_dashboardclient/DashboardClient: collecting source file list for library: ${DASHBOARDCLIENT_SRC}")

add_library(DashboardClient ${DASHBOARDCLIENT_SRC})

target_link_libraries(DashboardClient PUBLIC ModelOpcUaLib)
target_link_libraries(DashboardClient PUBLIC Util)
target_link_libraries(DashboardClient PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(DashboardClient PUBLIC open62541::open62541)

target_include_directories(
    DashboardClient PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<INSTALL_INTERFACE:include>
)
