cmake_minimum_required(VERSION 3.9)

message("### opcua_dashboardclient/MachineObserver: loading Machine Observer")

find_package(UaSdkCpp REQUIRED)

set(CLIENT_SRC
        "MachineObserver.cpp"
        "DashboardMachineObserver.cpp"
        "Topics.cpp"
        "PublishMachinesList.cpp"
        )
message("### opcua_dashboardclient/MachineObserver: collecting source file list for library: ${CLIENT_SRC}")

add_library(MachineObserver ${CLIENT_SRC})
target_link_libraries(MachineObserver OpcUaClientLib)
target_include_directories(OpcUaClientLib PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
        )
