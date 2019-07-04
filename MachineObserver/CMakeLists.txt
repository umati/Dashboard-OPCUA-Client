cmake_minimum_required(VERSION 3.9)

find_package(UASdkCpp REQUIRED)

set(CLIENT_SRC
	"MachineObserver.cpp"
	"DashboardMachineObserver.cpp"
	"PublishTopicFactory.cpp"
)

add_library(MachineObserver ${CLIENT_SRC})
target_link_libraries(MachineObserver OpcUaClientLib)
target_include_directories(OpcUaClientLib PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
  )
