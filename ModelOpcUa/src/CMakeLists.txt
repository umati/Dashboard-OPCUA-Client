cmake_minimum_required (VERSION 3.11)

add_library (ModelOpcUaLib 
	"ModelOpcUa/ModelDefinition.cpp"
	"ModelOpcUa/ModelInstance.cpp"
	)

target_include_directories(ModelOpcUaLib INTERFACE 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<INSTALL_INTERFACE:include>)

