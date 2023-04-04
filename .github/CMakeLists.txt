cmake_minimum_required(VERSION 3.15)

project(BuildWithDependencies VERSION 2.0.0)

include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

## build options
set(BUILD_DEB_PACKAGE FALSE CACHE BOOL "Flag that defines whether to build a debian package")
set(DEB_PACKAGE_LIBCPP_VERSION "8.4.0" CACHE STRING "The debian package libstdc++6 dependency version")
set(PAHO_WITH_SSL TRUE CACHE BOOL "Flag that defines whether to build paho.mqtt with SSL")

externalproject_add(
    JsonExternal
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../deps/json
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX} -DJSON_BuildTests:BOOL=0
    INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
)

externalproject_add(
    EasyloggingppExternal
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../deps/easyloggingpp
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX} -Dbuild_static_lib:BOOL=1
    INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
)

externalproject_add(
    GTestExternal
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../deps/googletest
    CMAKE_ARGS -Dgtest_force_shared_crt:BOOL=ON -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
    INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
)

find_package(Patch REQUIRED)
# Patch command might be applied already, so errors need to be ignored.
externalproject_add(
    libreSSLExternal
    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/libreSslDownload
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/libreSslSource
    URL "https://ftp.openbsd.org/pub/OpenBSD/LibreSSL/libressl-3.7.1.tar.gz "
    URL_HASH SHA256=98086961a2b8b657ed0fea3056fb2db14294b6bfa193c15a5236a0a35c843ded
    PATCH_COMMAND "${Patch_EXECUTABLE}" -p1 --forward -d ${CMAKE_CURRENT_BINARY_DIR}/libreSslSource <
                  ${CMAKE_CURRENT_SOURCE_DIR}/ciPatches/libressl-no_postfix.patch || echo "Patch returned non-zero"
    CMAKE_ARGS  -DLIBRESSL_APPS:BOOL=0 
                -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX} 
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
)

message("Building PahoCExternal with PAHO_WITH_SSL=" ${PAHO_WITH_SSL})
externalproject_add(
    PahoCExternal
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../deps/paho.mqtt.c
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
               -DPAHO_WITH_SSL=${PAHO_WITH_SSL}
               -DPAHO_BUILD_SHARED:BOOL=0
               -DPAHO_BUILD_STATIC:BOOL=1
               -DPAHO_ENABLE_TESTING:BOOL=1 
               -DLIBRESSL_ROOT_DIR:PATH=${CMAKE_INSTALL_PREFIX}/
               -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    DEPENDS libreSSLExternal
    PATCH_COMMAND "${Patch_EXECUTABLE}" -p1 --forward -d ${CMAKE_CURRENT_SOURCE_DIR}/../deps/paho.mqtt.c <
                  ${CMAKE_CURRENT_SOURCE_DIR}/ciPatches/paho.mqtt.c.patch || echo "Patch returned non-zero"
    INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
)

externalproject_add(
    PahoCppExternal
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../deps/paho.mqtt.cpp
    CMAKE_ARGS  -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
                -DPAHO_WITH_SSL=${PAHO_WITH_SSL}
                -DPAHO_BUILD_SHARED:BOOL=0
                -DPAHO_BUILD_STATIC:BOOL=1
                -DPAHO_ENABLE_TESTING:BOOL=1
                -DLIBRESSL_ROOT_DIR:PATH=${CMAKE_INSTALL_PREFIX}/
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    DEPENDS PahoCExternal libreSSLExternal
    PATCH_COMMAND "${Patch_EXECUTABLE}" -p1 --forward -d ${CMAKE_CURRENT_SOURCE_DIR}/../deps/paho.mqtt.cpp <
                  ${CMAKE_CURRENT_SOURCE_DIR}/ciPatches/paho.mqtt.cpp.patch || echo "Patch returned non-zero"
    INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
)

externalproject_add(
    open62541External
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../deps/open62541
    CMAKE_ARGS  -DUA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS:BOOL=ON
                -DUA_ENABLE_SUBSCRIPTIONS_EVENTS:BOOL=ON
                -DUA_NAMESPACE_ZERO:STRING=FULL
                -DUA_ENABLE_ENCRYPTION:STRING=LIBRESSL
                -DLIBRESSL_ROOT_DIR:PATH=${CMAKE_INSTALL_PREFIX}
                -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
    DEPENDS libreSSLExternal
)

externalproject_add(
    open62541CppExternal
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../deps/open62541Cpp
    CMAKE_ARGS  -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
    DEPENDS open62541External GTestExternal
)

externalproject_add(
    Dashboard-Client
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..
    CMAKE_ARGS  -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
                -DLIBRESSL_ROOT_DIR:PATH=${CMAKE_INSTALL_PREFIX}/
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
    DEPENDS open62541CppExternal EasyloggingppExternal JsonExternal PahoCppExternal
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/Dashboard-Client-build
)


if(${BUILD_DEB_PACKAGE})
    message("### Build debian package")
    unset(BINARY_DIR)
    ExternalProject_Get_property(Dashboard-Client BINARY_DIR) 
    set(BINARY_DIR_Dashboard-Client ${BINARY_DIR})   
    set(CPACK_INSTALL_CMAKE_PROJECTS  ${BINARY_DIR_Dashboard-Client} "Dashboard-Client" "ALL" "/")
    set(CPACK_PACKAGE_NAME "DashboardOpcUaClient")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OPC UA client based on open62541 to subscribe to servers implementing umati endorsed companion specification and publish via MQTT to the demo dashboard https://umati.app")
    set(CPACK_PACKAGE_VERSION ${CMAKE_PROJECT_VERSION})
    set(CPACK_PACKAGE_DIRECTORY ${CMAKE_INSTALL_PREFIX})
    set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/../LICENSE")
    set(CPACK_RESOURCE_FILE_README "${PROJECT_SOURCE_DIR}/../README.md")
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Marc Fischer")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libstdc++6 (>= ${DEB_PACKAGE_LIBCPP_VERSION}), openssl(>= 1.1.1), ca-certificates")
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
    set(CPACK_DEB_COMPONENT_INSTALL YES)
    INCLUDE(CPack)
endif()
