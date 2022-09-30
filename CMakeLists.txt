project(LOCAL_UTIL)

set(SOURCES_LIST localUtil.c)
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    list(APPEND SOURCES_LIST localUtil_xnu.c)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    list(APPEND SOURCES_LIST localUtil_windows.c)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    list(APPEND SOURCES_LIST localUtil_linux.c)
endif()
add_library(localUtil STATIC ${SOURCES_LIST})

set(SOURCES_LIST localUtil.cpp)
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    list(APPEND SOURCES_LIST localUtil_linux.cpp)
endif()
add_library(localUtil_cpp STATIC ${SOURCES_LIST})

# must have json-c installed with
# sudo apt-get install libjson-c-dev
# if (${MY_HOST_COMPILER} STREQUAL "linux")
#     add_library(jsonUtil STATIC jsonUtil.c)
#     target_include_directories(jsonUtil PRIVATE
#         /usr/include/json-c)
# endif()