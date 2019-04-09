cmake_minimum_required(VERSION 3.9)

find_package(UASdkCpp REQUIRED)

set(CLIENT_SRC
	"OpcUaClient.cpp"
	"SetupSecurity.cpp"
	"Converter/UaNodeIdToModelNodeId.cpp"
	"Converter/ModelNodeIdToUaNodeId.cpp"
	"Converter/ModelToUaConverter.cpp"
	"Converter/UaToModelConverter.cpp"
	"Converter/UaNodeClassToModelNodeClass.cpp"
	"Converter/ModelQualifiedNameToUaQualifiedName.cpp"
	"Converter/UaQualifiedNameToModelQualifiedName.cpp"
	"Exceptions/OpcUaNonGoodStatusCodeException.cpp"
)

add_library(OpcUaClientLib ${CLIENT_SRC})
target_link_libraries(OpcUaClientLib Util)
target_link_libraries(OpcUaClientLib UaSDKCpp::UaSDKCpp)
target_link_libraries(OpcUaClientLib ModelOpcUaLib)
target_link_libraries(OpcUaClientLib DashboardClient)
target_include_directories(OpcUaClientLib PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
  )
