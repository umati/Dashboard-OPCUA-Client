# Finds Unified Automation C++ SDK
# This will define the following variables
# UASDKCPP_FOUND
# and the following imported target
# UaSDKCpp::UaSDKCpp
#
# Set UNIFIED_AUTOMATION_CPP_SERVER_SDK_DIR to cmake in Unified Automation SDK

### \todo Allow multiple _libDir? Then a separator keyword is necessary
function(to_abs_libraries _out_var_abs_libs_var _libDir)
    set(_out_var_abs_libs)
    set(LINK_LIBRARIES_KEYWORDS debug optimized)
    # ARGN contains all arguments after the last specified argument
    foreach (ITEM ${ARGN})
        if (${ITEM} IN_LIST LINK_LIBRARIES_KEYWORDS OR IS_ABSOLUTE ${ITEM})
            #message(STATUS "keepArg: " "${ITEM}")
            list(APPEND _out_var_abs_libs "${ITEM}")
        else ()
            #message(STATUS "relLib: " "${ITEM}")
            #TODO rewrite to custom variable names
            unset(abs_path_of_lib CACHE) # Delete any preset variable, so find_library will not use a cached value!
            find_library(abs_path_of_lib NAMES ${ITEM} PATHS ${_libDir} NO_DEFAULT_PATH)
            if ("${abs_path_of_lib}" STREQUAL "abs_path_of_lib-NOTFOUND")
                # message(STATUS "Could not locate '${ITEM}' assume a public library")
                # Abs path could not be found, use the name instead
                list(APPEND _out_var_abs_libs "${ITEM}")
            else ()
                #message(STATUS "Abspath ${abs_path_of_lib}")
                list(APPEND _out_var_abs_libs "${abs_path_of_lib}")
            endif ()
            unset(abs_path_of_lib CACHE) # Hide internal variable from CACHE
        endif ()
    endforeach ()
    set(${_out_var_abs_libs_var} ${_out_var_abs_libs} PARENT_SCOPE)
endfunction()

message(STATUS "### UNIFIED_AUTOMATION_CPP_SERVER_SDK_DIR: ${UNIFIED_AUTOMATION_CPP_SERVER_SDK_DIR}")


# include Unified Automation SDK
set(UNIFIED_AUTOMATION_CPP_SERVER_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../uasdk/cmake")
#set(UNIFIED_AUTOMATION_CPP_SERVER_SDK_DIR "/mnt/c/Projektdaten/umati/sdk2/cmake/")
#set(UNIFIED_AUTOMATION_CPP_SERVER_SDK_DIR  CACHE PATH "Path to directory 'cmake' of a build SDK")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${UNIFIED_AUTOMATION_CPP_SERVER_SDK_DIR})
message("### current dir: ${CMAKE_CURRENT_SOURCE_DIR}/../../uasdk/cmake")
find_package(UaOpenSSL REQUIRED)
find_package(UaLibXml2 REQUIRED)

include(MessageUtils)
include(InstallPDBFiles)
include(InstallIfNewer)
include(ConfigureCompiler)
include(ConfigureUaStack)
include(ConfigureCppSdk)

#Must be before add_library
#link_directories(${UA_LIB_DIR})

add_library(UaSDKCpp::UaSDKCpp INTERFACE IMPORTED)

# Include directories and libraries
target_include_directories(UaSDKCpp::UaSDKCpp INTERFACE ${UABASE_INCLUDE})

target_include_directories(UaSDKCpp::UaSDKCpp INTERFACE ${UASTACK_INCLUDE})
target_include_directories(UaSDKCpp::UaSDKCpp INTERFACE ${UACLIENT_INCLUDE})
target_include_directories(UaSDKCpp::UaSDKCpp INTERFACE ${UAPKI_INCLUDE})
target_include_directories(UaSDKCpp::UaSDKCpp INTERFACE ${UAXMLPARSER_INCLUDE})
#target_include_directories(UaSDKCpp::UaSDKCpp INTERFACE ${UACOREMODULE_INCLUDE})
#target_include_directories(UaSDKCpp::UaSDKCpp INTERFACE ${UAMODULE_INCLUDE})
target_include_directories(UaSDKCpp::UaSDKCpp INTERFACE ${LIBXML2_INCLUDE_DIR})
target_include_directories(UaSDKCpp::UaSDKCpp INTERFACE ${OPENSSL_INCLUDE_DIR})

message("### opcua_dashboardclient/cmake UAMODULE_INCLUDE was set to ${UAMODULE_INCLUDE}")
message("### opcua_dashboardclient/cmake UACOREMODULE_INCLUDE was set to ${UACOREMODULE_INCLUDE}")
message("### opcua_dashboardclient/cmake UAMODULE_LIBRARY was set to ${UAMODULE_LIBRARY}")
message("### opcua_dashboardclient/cmake UACOREMODULE_LIBRARY was set to ${UACOREMODULE_LIBRARY}")
#set(UASDK_WITH_XMLPARSER ON CACHE BOOL "")

if (UASDK_WITH_XMLPARSER)
    message("###########################################")
    # to_abs_libraries(UA_XML_ABS_PATHS_TO_LIBS})
    #target_link_libraries(UaSDKCpp::UaSDKCpp INTERFACE ${UA_XML_ABS_PATHS_TO_LIBS})
endif ()
to_abs_libraries(UA_COMMON_LIBS_ABS_PATHS
        ${UA_LIB_DIR}
        #	${UAMODULE_LIBRARY}
        #	${UACOREMODULE_LIBRARY}
        ${UACLIENT_LIBRARY}
        ${UAPKI_LIBRARY}
        ${UABASE_LIBRARY}
        ${UASTACK_LIBRARY}
        ${UAXML_LIBRARY}
        ${LIBXML2_LIBRARIES}
        ${SYSTEM_LIBS})
target_link_libraries(UaSDKCpp::UaSDKCpp INTERFACE ${UA_COMMON_LIBS_ABS_PATHS})


if (UASTACK_WITH_OPENSSL)
    to_abs_libraries(UA_OPENSSL_ABS_PATHS_TO_LIBS ${UA_LIB_DIR} ${OPENSSL_LIBRARIES})
    target_link_libraries(UaSDKCpp::UaSDKCpp INTERFACE ${UA_OPENSSL_ABS_PATHS_TO_LIBS})
endif ()

if (UAMODULE_LIBRARY)
    message("### opcua_dashboardclient/cmake UAMODULE_LIBRARY was set")
endif ()

if (UACOREMODULE_LIBRARY)
    message("### opcua_dashboardclient/cmake UACOREMODULE_LIBRARY was set")
    SET(UASDKCPP_FOUND 1)
endif ()
