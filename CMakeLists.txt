cmake_minimum_required(VERSION 3.9)

message("### opcua_dashboardclient: loading project")


project(Dashboard_OpcUaClient)
enable_testing()
set(CMAKE_CXX_STANDARD 14)

message("########################### CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
message("########################### CREATE_COVERAGE=${CREATE_COVERAGE}")
if(CREATE_COVERAGE)
    message("########################### COVERAGE enabled")
    set(GCC_COVERAGE_COMPILE_FLAGS "-g -O0 --coverage -fprofile-arcs -ftest-coverage")
    set(GCC_COVERAGE_LINK_FLAGS "-lgcov --coverage -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_FLAGS "${GCC_COVERAGE_COMPILE_FLAGS} ${CMAKE_CXX_FLAGS}")
    set(CMAKE_C_FLAGS "${GCC_COVERAGE_COMPILE_FLAGS} ${CMAKE_C_FLAGS}")
endif()

message("########################### MEMORY_SANITZER=${MEMORY_SANITZER}")
if(MEMORY_SANITZER)
    message("########################### MEMORY_SANITZER enabled")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -fno-omit-frame-pointer -fsanitize=leak")
    set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -g -fno-omit-frame-pointer -fsanitize=leak")
endif()

message("### opcua_dashboardclient: adding cmake files to find used libraries from /cmake directory")
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)

message("### opcua_dashboardclient: Set common directories for all executables in this project")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

###########
# Version #
###########
# The current version information. On the master branch, we take the version
# number from the latest release plus the "-undefined" label. Will be
# overwritten with more detailed information if git is available.
set(UMATI_CLIENT_VER_MAJOR 2)
set(UMATI_CLIENT_VER_MINOR 1)
set(UMATI_CLIENT_VER_PATCH 0)
set(UMATI_CLIENT_VER_LABEL "-rc1") # like "-rc1" or "-g4538abcd" or "-g4538abcd-dirty"
set(UMATI_CLIENT_VER_COMMIT "unknown-commit")

# Overwrite the version information based on git if available
include(SetGitBasedVersion)
set_umati_dashboard_client_version()

# Examples for the version string are:
# 1.2
# 1.2.3
# 1.2.3-rc1
# 1.2.3-rc1-dirty
# 1.2.3-5-g4538abcd
# 1.2.3-5-g4538abcd-dirty
set(DASHBOARD_CLIENT_VERSION "${UMATI_CLIENT_VER_MAJOR}.${UMATI_CLIENT_VER_MINOR}.${UMATI_CLIENT_VER_PATCH}${UMATI_CLIENT_VER_LABEL}")
MESSAGE("### opcua_dashboardclient Version: ${DASHBOARD_CLIENT_VERSION}")

configure_file(Util/ClientVersion.cpp.in Util/ClientVersion.cpp @ONLY)


message("### opcua_dashboardclient: Adding the target")
add_custom_target(Dashboard_OpcUaClient)

add_definitions(-DELPP_THREAD_SAFE -DELPP_USE_STD_THREADING -DELPP_NO_DEFAULT_LOG_FILE)
find_package(easyloggingpp REQUIRED)
find_package(open62541 REQUIRED)
find_package(Open62541Cpp REQUIRED)
find_package(Threads REQUIRED)
include_directories(${CPP_MOSQUITTO_INCLUDE})
message("### opcua_dashboardclient: Adding subdirectories")
add_subdirectory(Util)
add_subdirectory(ModelOpcUa)
add_subdirectory(DashboardClient)
add_subdirectory(OpcUaClient)
add_subdirectory(MachineObserver)

message("### opcua_dashboardclient: Adding test directory")
add_subdirectory(Tests/unit)

message("### opcua_dashboardclient: Adding the executable c++ file")
find_package(Threads)
add_executable(DashboardOpcUaClient DashboardOpcUaClient.cpp Main.cpp ${CMAKE_BINARY_DIR}/Util/ClientVersion.cpp)
add_executable(WssTroubleshooter WssTroubleshooter.cpp)

message("### opcua_dashboardclient: Setting Dashboard Publisher")
target_link_libraries(DashboardOpcUaClient PUBLIC OpcUaClientLib)
target_link_libraries(DashboardOpcUaClient PUBLIC MachineObserver)
target_link_libraries(WssTroubleshooter PUBLIC OpcUaClientLib PahoMqttCpp::paho-mqttpp3-static)

set(DASHBOARD_PUBLISHER
    "MQTT_PAHO"
    CACHE STRING "Publisher Backend"
)
set_property(CACHE DASHBOARD_PUBLISHER PROPERTY STRINGS MQTT_MOSQUITTO MQTT_PAHO REDIS)

if(DASHBOARD_PUBLISHER STREQUAL "MQTT_MOSQUITTO")
    message("### opcua_dashboardclient: Adding Publisher mosquitto")
    add_subdirectory(MqttPublisher)
    target_compile_definitions(DashboardOpcUaClient PUBLIC PUBLISHER_MQTT_MOSQUITTO=1)
    target_link_libraries(DashboardOpcUaClient PUBLIC MqttPublisher)
endif()

if(DASHBOARD_PUBLISHER STREQUAL "MQTT_PAHO")
    # Promote target PahoMqttC::PahoMqttC for dll copy
    message("### opcua_dashboardclient: Adding Publisher paho")
    find_package(PahoMqttCpp REQUIRED)
    add_subdirectory(MqttPublisher_Paho)
    target_compile_definitions(DashboardOpcUaClient PUBLIC PUBLISHER_MQTT_PAHO=1)
    target_link_libraries(DashboardOpcUaClient PUBLIC MqttPublisher_Paho)
    target_compile_definitions(DashboardOpcUaClient PUBLIC NOMINMAX)
endif()

message("### opcua_dashboardclient: Adding custom command to copy the example config")
add_custom_command(
    TARGET DashboardOpcUaClient
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy configuration.json.example $<TARGET_FILE_DIR:DashboardOpcUaClient>
    COMMENT "Copy Example Configuration"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

message("### opcua_dashboardclient: Installing DashboardOpcUaClient targets")
install(
    TARGETS DashboardOpcUaClient WssTroubleshooter
    EXPORT DashboardOpcUaClient
    RUNTIME DESTINATION bin
)

set(CERTGEN_INSTALL_FILES Tools/certGen/createCertificate.bat Tools/certGen/createCertificate.sh Tools/certGen/openssl.cnf.template 
                          Tools/certGen/README.md Tools/certGen/settings.template.conf
)

install(FILES ${CERTGEN_INSTALL_FILES} DESTINATION share/DashboardOpcUaClient/Tools/certGen)

target_link_libraries(DashboardOpcUaClient PUBLIC Threads::Threads Open62541Cpp::Open62541Cpp)
message("### opcua_dashboardclient: Exporting DashboardOpcUaClient")
install(
    EXPORT DashboardOpcUaClient
    DESTINATION lib/DashboardOpcUaClient/cmake
    NAMESPACE DashboardOpcUaClient::
    EXPORT_LINK_INTERFACE_LIBRARIES
)
message("### opcua_dashbboardclient: package stage")
set(BUILD_DEB_PACKAGE TRUE CACHE BOOL "Flag that defines whether to build a debian package")

if(${BUILD_DEB_PACKAGE})
    message("### opcua_dashboardclient: build debian package")
    set(DEB_PACKAGE_LIBCPP_VERSION "8.4.0" CACHE STRING "The debian package libstdc++6 dependency version")
    #unset(BINARY_DIR)
    #ExternalProject_Get_property(Dashboard-Client BINARY_DIR) 
    set(BINARY_DIR_Dashboard-Client ${CMAKE_BINARY_DIR})   
    #set(CPACK_INSTALL_CMAKE_PROJECTS  ${BINARY_DIR_Dashboard-Client} "Dashboard-Client" "ALL" "/")
    set(CPACK_PACKAGE_NAME "DashboardOpcUaClient")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OPC UA client based on open62541 to subscribe to servers implementing umati endorsed companion specification and publish via MQTT to the demo dashboard https://umati.app")
    set(CPACK_PACKAGE_VERSION ${DASHBOARD_CLIENT_VERSION})
    set(CPACK_PACKAGE_DIRECTORY ${CMAKE_INSTALL_PREFIX})
    set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_README "${PROJECT_SOURCE_DIR}/README.md")
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Marc Fischer")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libstdc++6 (>= ${DEB_PACKAGE_LIBCPP_VERSION}), openssl(>= 1.1.1), ca-certificates")
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
    set(CPACK_DEB_COMPONENT_INSTALL YES)
    INCLUDE(CPack)
endif()
