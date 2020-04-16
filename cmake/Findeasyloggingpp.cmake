# Finds easylogging++
# This will define the following variables
# EASYLOGGINGPP_FOUND
# and the following imported target
# easyloggingpp::easyloggingpp
# Created by Christian von Arnim (Github @ccvca)
# 
# Add the directory of this file to your CMAKE_MODULE_PATH
# e.g. by list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
# include the library by find_package
# find_package(easyloggingpp REQUIRED)

# TODO handle source installation if the library
# TODO respect REQUIRED

find_library (EASYLOGGINGPP_LIB easyloggingpp)

if("${EASYLOGGINGPP_LIB}" STREQUAL "EASYLOGGINGPP_LIB-NOTFOUND")
	message(SEND_ERROR "### Could not found easylogging, please specify CMAKE_PREFIX_PATH")
else()
	message("### Found easylogging library: '${EASYLOGGINGPP_LIB}'")
endif()

find_path(EASYLOGGINGPP_INCLUDE easylogging++.h)
if("${EASYLOGGINGPP_INCLUDE}" STREQUAL "EASYLOGGINGPP_INCLUDE-NOTFOUND")
	message(SEND_ERROR "### Could not found easylogging++.h, please specify CMAKE_PREFIX_PATH")
else()
	message("### Found easylogging++.h in:'${EASYLOGGINGPP_INCLUDE}' ")
endif()


add_library(easyloggingpplib ${EASYLOGGINGPP_INCLUDE}/easylogging++.cc)
target_compile_definitions(easyloggingpplib PUBLIC ELPP_STL_LOGGING ELPP_THREAD_SAFE ELPP_NO_DEFAULT_LOG_FILE ELPP_USE_STD_THREADING)

add_library(easyloggingpp::easyloggingpp INTERFACE IMPORTED)
#target_link_libraries(easyloggingpp::easyloggingpp INTERFACE ${EASYLOGGINGPP_LIB})
target_link_libraries(easyloggingpp::easyloggingpp INTERFACE easyloggingpplib)

target_include_directories(easyloggingpp::easyloggingpp INTERFACE ${EASYLOGGINGPP_INCLUDE})
#target_compile_definitions(easyloggingpp::easyloggingpp INTERFACE ELPP_STL_LOGGING ELPP_THREAD_SAFE)

SET(EASYLOGGINGPP_FOUND 1)
