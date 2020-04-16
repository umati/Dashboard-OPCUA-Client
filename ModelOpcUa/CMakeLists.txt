#
cmake_minimum_required(VERSION 3.11)
message("### opcua_dashboardclient/ModelOpcUa: loading OpcUA Model")

enable_testing()

message("### opcua_dashboardclient/ModelOpcUa: adding subdirectories")
add_subdirectory(src)

add_subdirectory(example)
