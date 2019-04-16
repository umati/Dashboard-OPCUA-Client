cmake_minimum_required(VERSION 3.9)

set(DASHBOARDCLIENT_SRC
	"TypeDefinition/IdentificationType.cpp"
	"TypeDefinition/StacklightType.cpp"
	"TypeDefinition/ToolListType.cpp"
	"TypeDefinition/ProductionJobListType.cpp"
	"TypeDefinition/NotificationFolderType.cpp"
	"TypeDefinition/StateModeListType.cpp"
	"DashboardClient.cpp"
	"Converter/ModelToJson.cpp"
)

find_package(nlohmann_json 3.6.1 REQUIRED)

add_library(DashboardClient ${DASHBOARDCLIENT_SRC})

target_link_libraries(DashboardClient PUBLIC ModelOpcUaLib)
target_link_libraries(DashboardClient PUBLIC Util)
target_link_libraries(DashboardClient PUBLIC nlohmann_json::nlohmann_json)

target_include_directories(DashboardClient PUBLIC 
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
$<INSTALL_INTERFACE:include>
)

