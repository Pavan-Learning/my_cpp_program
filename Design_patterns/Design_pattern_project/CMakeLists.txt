cmake_minimum_required(VERSION 3.21.1)

project("thread_safe_logger"
    VERSION 1.0.0
     LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Library of logger sources (shared between app and tests)
add_library(logger_lib STATIC
    src/Logger_adapter.cpp
    src/Logger_thread.cpp
    src/Thread_safe_queue.cpp
    src/legacy_logger.cpp
)
target_include_directories(logger_lib PUBLIC inc)
find_package(Threads REQUIRED)
target_link_libraries(logger_lib PUBLIC Threads::Threads)

# Main executable
add_executable(threading src/main.cpp)
target_link_libraries(threading PRIVATE logger_lib)

# Tests
enable_testing()
find_package(GTest REQUIRED)

add_executable(test_thread_safe_queue test/test_thread_safe_queue.cpp)
target_link_libraries(test_thread_safe_queue PRIVATE logger_lib GTest::gtest GTest::gtest_main)
add_test(NAME ThreadSafeQueueTests COMMAND test_thread_safe_queue)

add_executable(test_logger_thread test/test_logger_thread.cpp)
target_link_libraries(test_logger_thread PRIVATE logger_lib GTest::gtest GTest::gtest_main)
add_test(NAME LoggerThreadTests COMMAND test_logger_thread)

add_executable(test_logger_adapter test/test_logger_adapter.cpp)
target_link_libraries(test_logger_adapter PRIVATE logger_lib GTest::gtest GTest::gtest_main)
add_test(NAME LoggerAdapterTests COMMAND test_logger_adapter)

