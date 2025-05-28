cmake_minimum_required(VERSION 3.12)

project(cpp-msgpack-without-dom LANGUAGES CXX C)

add_library(${PROJECT_NAME} STATIC
    "cpp_msgpack_without_dom.cpp"
    "cpp_msgpack_without_dom.hpp"
)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}"
)

# msgpack-c
target_link_libraries(${PROJECT_NAME} PUBLIC msgpack-c)

# string-view-lite
if(NOT DEFINED STRING_VIEW_LITE AND ${CMAKE_CXX_STANDARD_AVAILABLE} LESS 17)
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_SOURCE_DIR}/deps/string-view-lite/include/nonstd"
    )
elseif(${CMAKE_CXX_STANDARD_AVAILABLE} LESS 17)
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${STRING_VIEW_LITE}/include/nonstd"
    )
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4 # Warning Level
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wno-float-conversion
    )
endif()
