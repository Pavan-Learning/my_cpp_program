cmake_minimum_required(VERSION 3.21.1)

project("thread_safe_logger"
    VERSION 1.0.0
     LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(threading
    src/main.cpp
    src/Logger_adapter.cpp
    src/Logger_thread.cpp
    src/Thread_safe_queue.cpp
    src/legacy_logger.cpp
)

target_include_directories(threading PRIVATE inc)

find_package(Threads REQUIRED)
target_link_libraries(threading PRIVATE Threads::Threads)

