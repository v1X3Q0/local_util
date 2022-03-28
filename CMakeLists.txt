project(LOCAL_UTIL)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(SOURCES_LIST localUtil.c localUtil_xnu.c)
else()
    set(SOURCES_LIST localUtil.c)
endif()


add_library(localUtil STATIC ${SOURCES_LIST})
add_library(localUtil_cpp STATIC localUtil.cpp)

if (${MY_HOST_COMPILER} STREQUAL "linux")
    add_library(jsonUtil STATIC jsonUtil.c)
    target_include_directories(jsonUtil PRIVATE
        /usr/include/json-c)
endif()