project(LOCAL_UTIL)

add_library(localUtil STATIC localUtil.c)
add_library(localUtil_cpp STATIC localUtil.cpp)

add_library(jsonUtil STATIC jsonUtil.c)
target_include_directories(jsonUtil PRIVATE
    /usr/include/json-c)