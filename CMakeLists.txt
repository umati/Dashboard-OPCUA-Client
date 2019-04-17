cmake_minimum_required(VERSION 3.9)

project(Dashboard_OpcUaClient)
enable_testing()
set(CMAKE_CXX_STANDARD 14)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Set common directories for all executables in this project
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_custom_target(Dashboard_OpcUaClient)

add_definitions(-DELPP_THREAD_SAFE -DELPP_USE_STD_THREADING -DELPP_NO_DEFAULT_LOG_FILE)
find_package(easyloggingpp REQUIRED)

add_subdirectory(Util)

add_subdirectory(ModelOpcUa)
add_subdirectory(DashboardClient)
add_subdirectory(OpcUaClient)


add_subdirectory(Tests)

add_executable(DashboardOpcUaClient DashboardOpcUaClient.cpp)
target_link_libraries(DashboardOpcUaClient PUBLIC OpcUaClientLib)

set(DASHBOARD_PUBLISHER "MQTT" CACHE STRING "Publisher Backend")
set_property(CACHE DASHBOARD_PUBLISHER PROPERTY STRINGS MQTT REDIS)


if(DASHBOARD_PUBLISHER STREQUAL "MQTT")
	add_subdirectory(MqttPublisher)
	target_compile_definitions(DashboardOpcUaClient PUBLIC PUBLISHER_MQTT=1)
	target_link_libraries(DashboardOpcUaClient PUBLIC MqttPublisher)
endif()

if(DASHBOARD_PUBLISHER STREQUAL "REDIS")
	add_subdirectory(RedisPublisher)
	target_link_libraries(DashboardOpcUaClient PUBLIC RedisPublisher)
	target_compile_definitions(DashboardOpcUaClient PUBLIC PUBLISHER_REDIS=1)
endif()

#Copy UA-SDK dlls to output directory
add_custom_command(TARGET DashboardOpcUaClient POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
        libeay32.dll
		libeay32d.dll
		libxml2.dll
		libxml2d.dll
		ssleay32.dll
		ssleay32d.dll
		uastack.dll
		uastackd.dll
		$<TARGET_FILE_DIR:DashboardOpcUaClient>
	WORKING_DIRECTORY "${UASDK_BASE_DIR}/bin/"
	COMMENT "Copy UA Dlls to bin"
)

add_custom_command(TARGET DashboardOpcUaClient POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
        libcrypto-1_1.dll
		libssl-1_1.dll
		mosquitto.dll
		mosquittopp.dll
		$<TARGET_FILE_DIR:DashboardOpcUaClient>
	WORKING_DIRECTORY "${CPP_MOSQUITTO_DIR}/bin/"
	COMMENT "Copy Mosquitto Dlls to bin"
)
