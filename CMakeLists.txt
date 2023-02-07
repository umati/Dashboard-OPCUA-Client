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
# add_subdirectory(Tests)

message("### opcua_dashboardclient: Adding the executable c++ file")
find_package(Threads)
add_executable(DashboardOpcUaClient DashboardOpcUaClient.cpp Main.cpp)

message("### opcua_dashboardclient: Setting Dashboard Publisher")
target_link_libraries(DashboardOpcUaClient PUBLIC OpcUaClientLib)
target_link_libraries(DashboardOpcUaClient PUBLIC MachineObserver)

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

message("### OpenSSL Library Found:" ${PAHO_WITH_SSL})

if(PAHO_WITH_SSL)
    set(OPENSSL_ROOT_DIR ${CMAKE_INSTALL_PREFIX})
    find_package(OpenSSL REQUIRED)
    message("### OpenSSL Library Found: " ${OPENSSL_FOUND})
    message("### OpenSSL Library Path: " ${OPENSSL_SSL_LIBRARY})
    message("### OpenSSL Crypto Library Path: " ${OPENSSL_CRYPTO_LIBRARY})
    message("### OpelSSL Include Dir: " ${OPENSSL_INCLUDE_DIR})
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
    TARGETS DashboardOpcUaClient
    EXPORT DashboardOpcUaClient
    RUNTIME DESTINATION bin
)

set(CERTGEN_INSTALL_FILES Tools/certGen/createCertificate.bat Tools/certGen/createCertificate.sh Tools/certGen/openssl.cnf.template 
                          Tools/certGen/README.md Tools/certGen/settings.template.conf
)

install(FILES ${CERTGEN_INSTALL_FILES} DESTINATION bin/Tools/certGen)

target_link_libraries(DashboardOpcUaClient PUBLIC Threads::Threads Open62541Cpp::Open62541Cpp)
message("### opcua_dashboardclient: Exporting DashboardOpcUaClient")
install(
    EXPORT DashboardOpcUaClient
    DESTINATION lib/DashboardOpcUaClient/cmake
    NAMESPACE DashboardOpcUaClient::
    EXPORT_LINK_INTERFACE_LIBRARIES
)
