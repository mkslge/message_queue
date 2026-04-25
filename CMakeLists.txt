cmake_minimum_required(VERSION 3.15)
project(MessageQueue)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(messagequeue_lib INTERFACE)
target_include_directories(messagequeue_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(messagequeue_lib INTERFACE cxx_std_20)

add_executable(messagequeue main.cpp)
target_link_libraries(messagequeue PRIVATE messagequeue_lib)

include(CTest)

if(BUILD_TESTING)
    file(GLOB TEST_SOURCES CONFIGURE_DEPENDS tests/*.cpp)

    if(TEST_SOURCES)
        find_package(GTest CONFIG QUIET)

        if(NOT GTest_FOUND)
            include(FetchContent)
            FetchContent_Declare(
                googletest
                URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
            )

            set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
            FetchContent_MakeAvailable(googletest)
        endif()

        add_executable(messagequeue_tests ${TEST_SOURCES})
        target_link_libraries(messagequeue_tests
            PRIVATE
                messagequeue_lib
                GTest::gtest_main
        )

        include(GoogleTest)
        gtest_discover_tests(messagequeue_tests)
    endif()
endif()
