cmake_minimum_required(VERSION 3.0)

project(wsjcpp-async-jobs-pool)

include(${CMAKE_CURRENT_SOURCE_DIR}/src.wsjcpp/CMakeLists.txt)

set(CMAKE_CXX_STANDARD 11)
set(EXECUTABLE_OUTPUT_PATH ${wsjcpp-async-jobs-pool_SOURCE_DIR})

# Sources

# include header dirs
list (APPEND WSJCPP_INCLUDE_DIRS "src")

list (APPEND WSJCPP_SOURCES "src/wsjcpp_async_jobs_pool.cpp")
list (APPEND WSJCPP_SOURCES "src/wsjcpp_async_jobs_pool.h")

list (APPEND WSJCPP_SOURCES "src/main.cpp")

include_directories(${WSJCPP_INCLUDE_DIRS})

add_executable (wsjcpp-async-jobs-pool ${WSJCPP_SOURCES})

target_link_libraries(wsjcpp-async-jobs-pool ${WSJCPP_LIBRARIES} )

install(
    TARGETS
        wsjcpp-async-jobs-pool
    RUNTIME DESTINATION
        /usr/bin
)
