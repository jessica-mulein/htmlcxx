# root CMakeLists.txt
set(CMAKE_VERBOSE_MAKEFILE True)
cmake_minimum_required(VERSION 3.15)
cmake_policy(VERSION 3.15)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(HTMLCXX2_VERSION "0.1.10")
set(HTMLCXX2_VERSION_MAJOR "0")
set(HTMLCXX2_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(HTMLCXX2_INCLUDE_ROOT ${HTMLCXX2_ROOT}/src)
set(HTMLCXX2_INCLUDE_DIR ${HTMLCXX2_INCLUDE_ROOT})

project(htmlcxx2
        LANGUAGES CXX
        VERSION ${HTMLCXX2_VERSION}
        #    SOVERSION ${HTMLCXX2_VERSION_MAJOR}
        DESCRIPTION "Header only fork of htmlcxx. Contains only html parser."
)

include_directories(SYSTEM PUBLIC ${HTMLCXX2_INCLUDE_ROOT})

# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # Optionally set things like CMAKE_CXX_STANDARD, CMAKE_POSITION_INDEPENDENT_CODE here
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    # Let's ensure -std=c++xx instead of -std=g++xx
    set(CMAKE_CXX_EXTENSIONS OFF)

    # Let's nicely support folders in IDE's
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    # Testing only available if this is the main app
    # Note this needs to be done in the main CMakeLists
    # since it calls enable_testing, which must be in the
    # main CMakeLists.
    enable_testing()
    include(CTest)

    # Docs only available if this is the main app
    find_package(Doxygen)
    if(Doxygen_FOUND)
        #add_subdirectory(doc)
    else()
        message(STATUS "Doxygen not found, not building docs")
    endif()

endif()


# Testing only available if this is the main app
# Emergency override MODERN_CMAKE_BUILD_TESTING provided as well
if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING) AND BUILD_TESTING)
    add_subdirectory(test)
endif()

add_library(htmlcxx2 INTERFACE)
target_include_directories(htmlcxx2 INTERFACE ${HTMLCXX2_INCLUDE_ROOT}/)