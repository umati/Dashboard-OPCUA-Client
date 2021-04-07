cmake_minimum_required(VERSION 3.9)

message("### opcua_dashboardclient: loading project")

project(Dashboard_OpcUaClient)
enable_testing()
set(CMAKE_CXX_STANDARD 14)
#Adress sanitizer
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

message("########################### ${CREATE_COVERAGE}")
if (CREATE_COVERAGE)
    SET(GCC_COVERAGE_COMPILE_FLAGS "-g -O0 --coverage -fprofile-arcs -ftest-coverage")
    SET(GCC_COVERAGE_LINK_FLAGS "-lgcov --coverage -fprofile-arcs -ftest-coverage")
    SET(CMAKE_CXX_FLAGS "${GCC_COVERAGE_COMPILE_FLAGS} ${CMAKE_CXX_FLAGS}")
    SET(CMAKE_C_FLAGS "${GCC_COVERAGE_COMPILE_FLAGS} ${CMAKE_C_FLAGS}")
endif ()

message("### opcua_dashboardclient: adding cmake files to find used libraries from /cmake directory")
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)

message("### opcua_dashboardclient: Set common directories for all executables in this project")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

message("### opcua_dashboardclient: Adding the target")
add_custom_target(Dashboard_OpcUaClient)

add_definitions(-DELPP_THREAD_SAFE -DELPP_USE_STD_THREADING -DELPP_NO_DEFAULT_LOG_FILE)
find_package(easyloggingpp REQUIRED)
find_package(open62541 REQUIRED)
find_package(Open62541Cpp REQUIRED)
include_directories(${CPP_MOSQUITTO_INCLUDE})
message("### opcua_dashboardclient: Adding subdirectories")
add_subdirectory(Util)
add_subdirectory(ModelOpcUa)
add_subdirectory(DashboardClient)
add_subdirectory(OpcUaClient)
add_subdirectory(MachineObserver)

message("### opcua_dashboardclient: Adding test directory")
#add_subdirectory(Tests)

message("### opcua_dashboardclient: Adding the executable c++ file")
find_package(Threads)
add_executable(DashboardOpcUaClient DashboardOpcUaClient.cpp)

message("### opcua_dashboardclient: Setting Dashboard Publisher")
target_link_libraries(DashboardOpcUaClient PUBLIC OpcUaClientLib)
target_link_libraries(DashboardOpcUaClient PUBLIC MachineObserver)

set(DASHBOARD_PUBLISHER "MQTT_PAHO" CACHE STRING "Publisher Backend")
set_property(CACHE DASHBOARD_PUBLISHER PROPERTY STRINGS MQTT_MOSQUITTO MQTT_PAHO REDIS)


if (DASHBOARD_PUBLISHER STREQUAL "MQTT_MOSQUITTO")
    message("### opcua_dashboardclient: Adding Publisher mosquitto")
    add_subdirectory(MqttPublisher)
    target_compile_definitions(DashboardOpcUaClient PUBLIC PUBLISHER_MQTT_MOSQUITTO=1)
    target_link_libraries(DashboardOpcUaClient PUBLIC MqttPublisher)
endif ()

if (DASHBOARD_PUBLISHER STREQUAL "MQTT_PAHO")
    # Promote target PahoMqttC::PahoMqttC for dll copy
    message("### opcua_dashboardclient: Adding Publisher paho")
    find_package(PahoMqttCpp REQUIRED)
    add_subdirectory(MqttPublisher_Paho)
    target_compile_definitions(DashboardOpcUaClient PUBLIC PUBLISHER_MQTT_PAHO=1)
    target_link_libraries(DashboardOpcUaClient PUBLIC MqttPublisher_Paho)
endif ()


if (WIN32)
    message("### opcua_dashboardclient: Adding custom command to add windows opcua libs")
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
endif ()

if (WIN32)
    if (DASHBOARD_PUBLISHER STREQUAL "MQTT_MOSQUITTO")
        message("### opcua_dashboardclient: Adding custom command to add windows mosquitto libs")
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
    endif ()
endif ()

if (WIN32)
    if (DASHBOARD_PUBLISHER STREQUAL "MQTT_PAHO")
        message("### opcua_dashboardclient: Adding custom command to add windows paho libs")
        add_custom_command(TARGET DashboardOpcUaClient POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE_DIR:PahoMqttC::PahoMqttC>/../bin/paho-mqtt3a.dll
                $<TARGET_FILE_DIR:DashboardOpcUaClient>
                COMMENT "Copy PahoMqttC::PahoMqttC shared library to bin"
                )
    endif ()
endif ()

message("### opcua_dashboardclient: Adding custom command to copy the example config")
add_custom_command(TARGET DashboardOpcUaClient POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        configuration.json.example
        $<TARGET_FILE_DIR:DashboardOpcUaClient>
        COMMENT "Copy Example Configuration"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

message("### opcua_dashboardclient: Installing DashboardOpcUaClient targets")
install(TARGETS DashboardOpcUaClient EXPORT DashboardOpcUaClient RUNTIME DESTINATION bin)

target_link_libraries(DashboardOpcUaClient PUBLIC pthread Open62541Cpp::Open62541Cpp)
message("### opcua_dashboardclient: Exporting DashboardOpcUaClient")
install(EXPORT DashboardOpcUaClient DESTINATION lib/DashboardOpcUaClient/cmake NAMESPACE DashboardOpcUaClient:: EXPORT_LINK_INTERFACE_LIBRARIES)
