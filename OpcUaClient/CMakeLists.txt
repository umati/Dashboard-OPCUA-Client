cmake_minimum_required(VERSION 3.9)

find_package(UASdkCpp REQUIRED)

set(CLIENT_SRC
	"OpcUaClient.cpp"
	"SetupSecurity.cpp"
	"Converter/UaNodeIdToModelNodeId.cpp"
	"Converter/ModelNodeIdToUaNodeId.cpp"
	"Converter/ModelToUaConverter.cpp"
	"Converter/UaToModelConverter.cpp"
)

add_library(OpcUaClient ${CLIENT_SRC})
target_link_libraries(OpcUaClient Util)
target_link_libraries(OpcUaClient UaSDKCpp::UaSDKCpp)
target_link_libraries(OpcUaClient ModelOpcUaLib)
