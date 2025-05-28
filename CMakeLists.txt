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
