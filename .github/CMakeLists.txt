cmake_minimum_required(VERSION 3.15)

project(BuildWithDependencies VERSION 1.2.0)

include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

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
    URL "https://ftp.openbsd.org/pub/OpenBSD/LibreSSL/libressl-3.7.0.tar.gz "
    URL_HASH SHA256=3fc1290f4007ec75f6e9acecbb25512630d1b9ab8c53ba79844e395868c3e006
    PATCH_COMMAND "${Patch_EXECUTABLE}" -p1 --forward -d ${CMAKE_CURRENT_BINARY_DIR}/libreSslSource <
                  ${CMAKE_CURRENT_SOURCE_DIR}/ciPatches/libressl-no_postfix.patch || echo "Patch returned non-zero"
    CMAKE_ARGS -DLIBRESSL_APPS:BOOL=0 -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
    INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
)

message("Building PahoCExternal with PAHO_WITH_SSL=" ${PAHO_WITH_SSL})
externalproject_add(
    PahoCExternal
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../deps/paho.mqtt.c
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
               -DPAHO_WITH_SSL=${PAHO_WITH_SSL}
               -DPAHO_BUILD_STATIC:BOOL=1
               -DPAHO_BUILD_SHARED:BOOL=0
               -DPAHO_ENABLE_TESTING:BOOL=0
               -DOPENSSL_ROOT_DIR=${CMAKE_INSTALL_PREFIX}
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
                -DPAHO_ENABLE_TESTING:BOOL=0
                -DOPENSSL_ROOT_DIR=${CMAKE_INSTALL_PREFIX}
    DEPENDS PahoCExternal libreSSLExternal
    INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
)

externalproject_add(
    open62541External
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../deps/open62541
    CMAKE_ARGS -DUA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS:BOOL=ON -DUA_ENABLE_SUBSCRIPTIONS_EVENTS:BOOL=ON
               -DUA_NAMESPACE_ZERO:STRING=FULL -DUA_ENABLE_ENCRYPTION:STRING=LIBRESSL
               -DLIBRESSL_ROOT_DIR:PATH=${CMAKE_INSTALL_PREFIX} -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
    INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
    DEPENDS libreSSLExternal
)

externalproject_add(
    open62541CppExternal
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../deps/open62541Cpp
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
    INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
    DEPENDS open62541External GTestExternal
)

externalproject_add(
    Dashboard-Client
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX} -DPAHO_WITH_SSL:BOOL=${DPAHO_WITH_SSL}
    INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
    DEPENDS open62541CppExternal EasyloggingppExternal JsonExternal PahoCppExternal
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/Dashboard-Client-build
)
