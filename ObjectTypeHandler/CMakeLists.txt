#
cmake_minimum_required(VERSION 3.11)
message("### opcua_dashboardclient/ObjectTypeHandler: loading ObjectTypeHandler")

enable_testing()

set(CLIENT_SRC
        "ObjectTypeHandler.cpp.cpp"
        "Type.cpp"
        )
message("### opcua_dashboardclient/ObjectTypeHandler: collecting source file list for library: ${CLIENT_SRC}")

add_library(OpcUaClientLib ${CLIENT_SRC})