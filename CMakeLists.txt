cmake_minimum_required(VERSION 3.2)

project("RadiusCacher" CXX)

##------------------------------------------------------------------------------
## Initialization
##

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set release by default
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Current state
if (WIN32)
  message(STATUS "WIN32")
endif ()
if (MSVC)
  message(STATUS "MSVC")
endif ()
if (APPLE)
  message(STATUS "APPLE")
endif ()

message(STATUS "Using C: " ${CMAKE_C_COMPILER_ID})
message(STATUS "Using CXX: " ${CMAKE_CXX_COMPILER_ID})

# Set high warning levels
if (MSVC)
  if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif ()
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wno-long-long -pedantic")
endif ()

# Make $HOME compatible with windows
STRING(REGEX REPLACE "\\\\" "/" ENV_HOME_DIR "$ENV{HOME}")

# Add custom modules
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/findcmake")

# Set base directories
set(CPP_SOURCE_DIR "src")
set(EXT_DIR ${CMAKE_SOURCE_DIR}/ext)

##------------------------------------------------------------------------------
## Options
##
#option(VERBOSE_LEVEL "Verbose level for application" 3)

##------------------------------------------------------------------------------
## Dependencies
##

## MFL
include("${EXT_DIR}/mfl/mfl.cmake")
list(APPEND LIBRARIES mfl)

# Threads
find_package(Threads)
list(APPEND LIBRARIES "${CMAKE_THREAD_LIBS_INIT}")

# Boost
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost 1.67.0 QUIET COMPONENTS system)
if (NOT Boost_FOUND)
  message(STATUS "Could not find Boost. Adding external project")
  include(${EXT_DIR}/boost/boost.cmake)
  list(APPEND INCLUDE_DIRS ${BOOST_INCLUDE_DIR})
  list(APPEND LIBRARIES boost::system)
else ()
  message(STATUS "Found Boost. Not adding external project")
  list(APPEND INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
  list(APPEND LIBRARIES ${Boost_LIBRARIES})
endif (NOT Boost_FOUND)

# LibMemcached
find_package(LibMemcached QUIET REQUIRED)
if (NOT LIBMEMCACHED_FOUND)
  message(STATUS "Could not find libmemcached package. Adding external project")
  include(${EXT_DIR}/libmemcached/libmemcached.cmake)
  list(APPEND INCLUDE_DIRS ${LIBMEMCACHED_INCLUDE_DIR})
  list(APPEND LIBRARIES ${LIBMEMCACHED_LIBRARY})
else ()
  message(STATUS "Found libmemcached package. Not adding external project")
  list(APPEND INCLUDE_DIRS ${LIBMEMCACHED_INCLUDE_DIR})
  list(APPEND LIBRARIES ${LIBMEMCACHED_LIBRARY})
endif (NOT LIBMEMCACHED_FOUND)

##------------------------------------------------------------------------------
## Sources
##

list(APPEND SOURCES
    ${CPP_SOURCE_DIR}/config.cpp
    ${CPP_SOURCE_DIR}/filter.cpp
  )

list(APPEND HEADERS
    ${CPP_SOURCE_DIR}/server.hpp
    ${CPP_SOURCE_DIR}/cache.hpp
    ${CPP_SOURCE_DIR}/radius.hpp
    ${CPP_SOURCE_DIR}/logger.hpp
    ${CPP_SOURCE_DIR}/config.hpp
    ${CPP_SOURCE_DIR}/radius_parser.hpp
    ${CPP_SOURCE_DIR}/filter.hpp
    ${CPP_SOURCE_DIR}/action.hpp
    )

##------------------------------------------------------------------------------
## Targets
##

add_library(radius-cacher-lib STATIC ${SOURCES} ${HEADERS})

# Add make time constants
if (DEFINED RC_BUFFER_SIZE)
  add_definitions(-DRC_BUFFER_SIZE=${RC_BUFFER_SIZE})
  message(STATUS "Using buffer size ${RC_BUFFER_SIZE}")
endif()

# Link with FIND_PACKAGE
target_link_libraries(radius-cacher-lib PUBLIC ${LIBRARIES})

# Add manual includes
target_include_directories(radius-cacher-lib PUBLIC ${INCLUDE_DIRS})

##------------------------------------------------------------------------------
## Main
##

add_executable(radius-cacher ${CPP_SOURCE_DIR}/main.cpp)
target_link_libraries(radius-cacher PRIVATE radius-cacher-lib)

##------------------------------------------------------------------------------
## Tests
##

option(RC_TEST "make tests" ON)

if (RC_TEST)

  set(CPP_TEST_DIR "test")

  enable_testing()
  include(GoogleTest)

  # GTest
  include("${EXT_DIR}/gtest/gtest.cmake")
  list(APPEND TEST_LIBRARIES gtest)
  list(APPEND TEST_INCLUDE_DIRS ${GTEST_INCLUDE_DIRS})

  # Test sources
  list(APPEND TESTS
      ${CPP_TEST_DIR}/test_main.cpp
      ${CPP_TEST_DIR}/test_config.cpp
      ${CPP_TEST_DIR}/test_filter.cpp
      ${CPP_TEST_DIR}/test_radius_parser.cpp
      )

  # Test executable
  add_executable(radius-cacher-tests ${TESTS})
  target_link_libraries(radius-cacher-tests PRIVATE radius-cacher-lib ${TEST_LIBRARIES})
  target_include_directories(radius-cacher-tests PRIVATE ${TEST_INCLUDE_DIRS})

  # CTest integration
  gtest_discover_tests(radius-cacher-tests)

  # Copy the resources
  file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/res/test)
  file(GLOB RESOURCES ${CPP_TEST_DIR}/res/*)
  file(COPY ${RESOURCES} DESTINATION ${CMAKE_BINARY_DIR}/res/test)

  # Run tests before main build
  add_custom_target(test-all
      COMMAND GTEST_COLOR=1 ctest -C $<CONFIGURATION> -j 4)
  add_dependencies(test-all radius-cacher-tests)
  add_dependencies(radius-cacher test-all)

endif()

