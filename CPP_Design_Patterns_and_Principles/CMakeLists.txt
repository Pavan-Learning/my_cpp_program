cmake_minimum_required(VERSION 3.16)
project(CppDesignPatternsAndPrinciples LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
enable_testing()

file(GLOB_RECURSE EXAMPLE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/patterns/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/solid/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/principles/*.cpp")

foreach(EXAMPLE_SOURCE IN LISTS EXAMPLE_SOURCES)
    get_filename_component(EXAMPLE_NAME "${EXAMPLE_SOURCE}" NAME_WE)
    add_executable("${EXAMPLE_NAME}" "${EXAMPLE_SOURCE}")
    target_include_directories("${EXAMPLE_NAME}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options("${EXAMPLE_NAME}" PRIVATE -Wall -Wextra -Wpedantic -Werror)
    elseif(MSVC)
        target_compile_options("${EXAMPLE_NAME}" PRIVATE /W4 /WX /permissive-)
    endif()
    add_test(NAME "${EXAMPLE_NAME}" COMMAND "${EXAMPLE_NAME}")
    set_tests_properties("${EXAMPLE_NAME}" PROPERTIES TIMEOUT 5)
endforeach()