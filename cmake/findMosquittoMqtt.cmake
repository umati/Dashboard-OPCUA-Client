set(CPP_MOSQUITTO_DIR $ENV{CMAKE_PREFIX_PATH} CACHE PATH "Path to directory 'cmake' of a build SDK")

find_library (CPP_MOSQUITTO_LIB mosquitto HINTS ${CPP_MOSQUITTO_DIR})
if("${CPP_MOSQUITTO_LIB}" STREQUAL "CPP_MOSQUITTO_LIB-NOTFOUND")
	message(SEND_ERROR "Could not found CPP_MOSQUITTO, please specify CMAKE_PREFIX_PATH")
else()
	message("Found CPP_MOSQUITTO_LIB library: '${CPP_MOSQUITTO_LIB}'")
endif()

find_library (CPP_MOSQUITTOPP_LIB mosquittopp HINTS ${CPP_MOSQUITTO_DIR})
if("${CPP_MOSQUITTOPP_LIB}" STREQUAL "CPP_MOSQUITTOPP_LIB-NOTFOUND")
	message(SEND_ERROR "Could not found CPP_MOSQUITTO, please specify CMAKE_PREFIX_PATH")
else()
	message("Found CPP_MOSQUITTOPP_LIB library: '${CPP_MOSQUITTOPP_LIB}'")
endif()


find_path (CPP_MOSQUITTO_INCLUDE mosquitto.h HINTS ${CPP_MOSQUITTO_DIR} PATH_SUFFIXES mosquitto)
if("${CPP_MOSQUITTO_INCLUDE}" STREQUAL "CPP_MOSQUITTO_INCLUDE-NOTFOUND")
	message(SEND_ERROR "Could not found CPP_MOSQUITTO header, please specify CMAKE_PREFIX_PATH")
else()
	message("Found CPP_MOSQUITTO include: '${CPP_MOSQUITTO_INCLUDE}'")
endif()

add_library(MosquittoMqtt::MosquittoMqtt INTERFACE IMPORTED)
target_link_libraries(MosquittoMqtt::MosquittoMqtt INTERFACE ${CPP_MOSQUITTO_LIB})
target_link_libraries(MosquittoMqtt::MosquittoMqtt INTERFACE ${CPP_MOSQUITTOPP_LIB})
target_include_directories(MosquittoMqtt::MosquittoMqtt INTERFACE ${CPP_MOSQUITTO_INCLUDE})
