cmake_minimum_required (VERSION 3.11)

message("### opcua_dashboardclient/ModelOpcUa/src: loading OpcUA Model")

set(OPCUAMODEL_SRC
		"ModelOpcUa/ModelDefinition.cpp"
		"ModelOpcUa/ModelInstance.cpp"
)

message("### opcua_dashboardclient/ModelOpcUa/src: collecting source file list for library: ${OPCUAMODEL_SRC}")

add_library (ModelOpcUaLib ${OPCUAMODEL_SRC})

target_include_directories(ModelOpcUaLib INTERFACE 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<INSTALL_INTERFACE:include>)

