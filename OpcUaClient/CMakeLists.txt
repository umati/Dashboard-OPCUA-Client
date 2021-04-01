cmake_minimum_required(VERSION 3.9)

message("### opcua_dashboardclient/OpcUaClient: loading OpcUaClient")

find_package(open62541 REQUIRED)
find_package(Open62541Cpp REQUIRED)

set(CLIENT_SRC
        "OpcUaClient.cpp"
        "SetupSecurity.cpp"
        "Subscription.cpp"
        "Converter/UaNodeIdToModelNodeId.cpp"
        "Converter/ModelNodeIdToUaNodeId.cpp"
        "Converter/ModelToUaConverter.cpp"
        "Converter/UaToModelConverter.cpp"
        "Converter/UaNodeClassToModelNodeClass.cpp"
        "Converter/ModelQualifiedNameToUaQualifiedName.cpp"
        "Converter/UaQualifiedNameToModelQualifiedName.cpp"
        "Converter/UaDataValueToJsonValue.cpp"
        "Exceptions/OpcUaNonGoodStatusCodeException.cpp"
        )
message("### opcua_dashboardclient/OpcUaClient: collecting source file list for library: ${CLIENT_SRC}")

add_library(OpcUaClientLib ${CLIENT_SRC})
target_link_libraries(OpcUaClientLib Util)
target_link_libraries(OpcUaClientLib open62541::open62541)
target_link_libraries(OpcUaClientLib Open62541Cpp::Open62541Cpp)
target_link_libraries(OpcUaClientLib ModelOpcUaLib)
target_link_libraries(OpcUaClientLib DashboardClient)
target_include_directories(OpcUaClientLib PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
        )
