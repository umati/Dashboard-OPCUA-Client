cmake_minimum_required(VERSION 3.15)

project(BuildWithDependencies VERSION 0.1)


include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

ExternalProject_Add(PahoCExternal
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../deps/paho.mqtt.c
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX} -DPAHO_WITH_SSL:BOOL=0 -DPAHO_BUILD_STATIC:BOOL=0 -DPAHO_ENABLE_TESTING:BOOL=0
	INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
)

ExternalProject_Add(PahoCppExternal
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../deps/paho.mqtt.cpp
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX} -DPAHO_BUILD_SHARED:BOOL=1 -DPAHO_WITH_SSL:BOOL=0 -DPAHO_BUILD_STATIC:BOOL=0
	DEPENDS PahoCExternal
	INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
)

ExternalProject_Add(JsonExternal
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../deps/json
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX} -DJSON_BuildTests:BOOL=0
	INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
)

ExternalProject_Add(EasyloggingppExternal
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../deps/easyloggingpp
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX} -Dbuild_static_lib:BOOL=1
	INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
)

ExternalProject_Add(GTestExternal
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../deps/googletest
	CMAKE_ARGS -Dgtest_force_shared_crt:BOOL=ON -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
	INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
)

ExternalProject_Add(mbedTLSExternal
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../deps/mbedtls
	CMAKE_ARGS -DENABLE_PROGRAMS:BOOL=0 -DENABLE_TESTING:BOOL=0 -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
	INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
)

ExternalProject_Add(open62541External
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../deps/open62541
	CMAKE_ARGS -DUA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS:BOOL=ON -DUA_ENABLE_SUBSCRIPTIONS_EVENTS:BOOL=ON -DUA_NAMESPACE_ZERO:STRING=FULL -DUA_ENABLE_ENCRYPTION:BOOL=1 -DUA_ENABLE_ENCRYPTION_MBEDTLS:BOOL=1 -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
	INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
	DEPENDS mbedTLSExternal
)

ExternalProject_Add(open62541CppExternal
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../deps/open62541Cpp
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
	INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
	DEPENDS open62541External GTestExternal
)

ExternalProject_Add(Dashboard-Client
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
	INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
	DEPENDS open62541CppExternal EasyloggingppExternal JsonExternal PahoCppExternal
	BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/Dashboard-Client-build
)