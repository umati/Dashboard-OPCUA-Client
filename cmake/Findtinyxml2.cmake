find_library(TINYXML2_LIB tinyxml2 HINTS ${TINYXML2_DIR})
if ("${TINYXML2_LIB}" STREQUAL "TINYXML2_LIB-NOTFOUND")
    message(SEND_ERROR "### Could not found TINYXML2, please specify CMAKE_PREFIX_PATH")
else ()
    message("### Found TINYXML2_LIB library: '${TINYXML2_LIB}'")
endif ()


find_path(TINYXML2_INCLUDE tinyxml2.h HINTS ${TINYXML2_DIR} PATH_SUFFIXES tinyxml2)
if ("${TINYXML2_INCLUDE}" STREQUAL "TINYXML2_INCLUDE-NOTFOUND")
    message(SEND_ERROR "### Could not found TINYXML2 header, please specify CMAKE_PREFIX_PATH")
else ()
    message("### Found TINYXML2 include: '${TINYXML2_INCLUDE}'")
endif ()

add_library(tinyxml2::tinyxml2 INTERFACE IMPORTED)
target_link_libraries(tinyxml2::tinyxml2 INTERFACE ${TINYXML2_LIB})
target_link_libraries(tinyxml2::tinyxml2 INTERFACE ${TINYXML2PP_LIB})
target_include_directories(tinyxml2::tinyxml2 INTERFACE ${TINYXML2_INCLUDE})
