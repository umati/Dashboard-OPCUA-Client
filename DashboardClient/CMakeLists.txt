cmake_minimum_required(VERSION 3.9)

message("### opcua_dashboardclient/DashboardClient: loading Dasbhoard client")

find_package(nlohmann_json 3.6.1 REQUIRED)
find_package(UaSdkCpp REQUIRED)

set(DASHBOARDCLIENT_SRC

        "DashboardClient.cpp"
        "IDashboardDataClient.cpp"
        "Converter/ModelToJson.cpp"
        )


message("### opcua_dashboardclient/DashboardClient: collecting source file list for library: ${DASHBOARDCLIENT_SRC}")

add_library(DashboardClient ${DASHBOARDCLIENT_SRC})

target_link_libraries(DashboardClient PUBLIC ModelOpcUaLib)
target_link_libraries(DashboardClient PUBLIC Util)
target_link_libraries(DashboardClient PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(DashboardClient PUBLIC UaSDKCpp::UaSDKCpp)

target_include_directories(DashboardClient PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
        )

