
set(CPP_REDIS_DIR $ENV{CMAKE_PREFIX_PATH} CACHE PATH "Path to directory 'cmake' of a build SDK")

### \TODO use also release
find_library (CPP_REDIS_LIB cpp_redis HINTS ${CPP_REDIS_DIR}/lib/Debug)
if("${CPP_REDIS_LIB}" STREQUAL "CPP_REDIS_LIB-NOTFOUND")
	message(SEND_ERROR "### Could not found cpp_redis, please specify CMAKE_PREFIX_PATH")
else()
	message("### Found cpp_redis library: '${CPP_REDIS_LIB}'")
endif()

find_library (TACOPIE_LIB tacopie HINTS ${CPP_REDIS_DIR}/lib/Debug)
if("${TACOPIE_LIB}" STREQUAL "TACOPIE_LIB-NOTFOUND")
	message(SEND_ERROR "### Could not found tacopie, please specify CMAKE_PREFIX_PATH")
else()
	message("### Found tacopie library: '${TACOPIE_LIB}'")
endif()

find_path (CPP_REDIS_INCLUDE cpp_redis)
if("${CPP_REDIS_INCLUDE}" STREQUAL "CPP_REDIS_INCLUDE-NOTFOUND")
	message(SEND_ERROR "### Could not found cpp_redis header, please specify CMAKE_PREFIX_PATH")
else()
	message("### Found cpp_redis include: '${CPP_REDIS_INCLUDE}'")
endif()

add_library(cpp_redis::cpp_redis INTERFACE IMPORTED)
target_link_libraries(cpp_redis::cpp_redis INTERFACE ${CPP_REDIS_LIB})
target_link_libraries(cpp_redis::cpp_redis INTERFACE ${TACOPIE_LIB})
target_include_directories(cpp_redis::cpp_redis INTERFACE ${CPP_REDIS_INCLUDE})
