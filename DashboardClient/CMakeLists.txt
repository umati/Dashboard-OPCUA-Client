cmake_minimum_required(VERSION 3.9)

set(DASHBOARDCLIENT_SRC
	"TypeDefinition/IdentificationType.cpp"
	"DashboardClient.cpp"
)

add_library(DashboardClient ${DASHBOARDCLIENT_SRC})

target_link_libraries(DashboardClient PUBLIC ModelOpcUaLib)
target_link_libraries(DashboardClient PUBLIC Util)

target_include_directories(DashboardClient PUBLIC 
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
$<INSTALL_INTERFACE:include>
)

