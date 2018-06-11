cmake_minimum_required(VERSION 3.2)

project("RadiusCacher" CXX)

##------------------------------------------------------------------------------
## Initialization
##

# Enable tests
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

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
set(LIB_DIR ${CMAKE_SOURCE_DIR}/lib)

##------------------------------------------------------------------------------
## Options
##
#option(VERBOSE_LEVEL "Verbose level for application" 3)

##------------------------------------------------------------------------------
## Dependencies
##

# MFL
add_subdirectory("${LIB_DIR}/mfl")
list(APPEND INCLUDE_DIRS ${MFL_INCLUDE_DIR})

# FMT
add_subdirectory("${LIB_DIR}/fmt")
list(APPEND INCLUDE_DIRS "${LIB_DIR}/fmt")
list(APPEND LIBRARIES fmt::fmt-header-only)

# Threads
find_package(Threads)
list(APPEND LIBRARIES "${CMAKE_THREAD_LIBS_INIT}")

# Boost
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost 1.67.0 COMPONENTS system)
if (NOT Boost_FOUND)
  message(STATUS "Could not find Boost. Adding external project")
  include(${LIB_DIR}/boost/boost.cmake)
  list(APPEND INCLUDE_DIRS ${BOOST_INCLUDE_DIR})
  list(APPEND LIBRARIES boost::system)
else ()
  message(STATUS "Found Boost. Not adding external project")
  list(APPEND INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
  list(APPEND LIBRARIES ${Boost_LIBRARIES})
endif (NOT Boost_FOUND)

# LibMemcached
find_package(LibMemcached REQUIRED)
if (NOT LIBMEMCACHED_FOUND)
  message(STATUS "Could not find libmemcached package. Adding external project")
  include(${LIB_DIR}/libmemcached/libmemcached.cmake)
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
    ${CPP_SOURCE_DIR}/main.cpp
    ${CPP_SOURCE_DIR}/config.cpp
  )

list(APPEND HEADERS
    ${CPP_SOURCE_DIR}/server.hpp
    ${CPP_SOURCE_DIR}/cache.hpp
    ${CPP_SOURCE_DIR}/radius.hpp
    ${CPP_SOURCE_DIR}/logger.hpp
    ${CPP_SOURCE_DIR}/config.hpp
    ${CPP_SOURCE_DIR}/radius_cacher.hpp
    )

##------------------------------------------------------------------------------
## Targets
##

add_executable(radius-cacher ${SOURCES} ${HEADERS})

# Add make time constants
if (DEFINED RC_VERBOSE_LEVEL)
  add_definitions(-DRC_VERBOSE_LEVEL=${RC_VERBOSE_LEVEL})
  message(STATUS Using verbose level ${RC_VERBOSE_LVEL})
endif()
if (DEFINED RC_CALLBACK_COUNT)
  add_definitions(-DRC_CALLBACK_COUNT=${RC_CALLBACK_COUNT})
  message(STATUS Using callback count ${RC_VERBOSE_LVEL})
endif()
if (DEFINED RC_BUFFER_SIZE)
  add_definitions(-DRC_BUFFER_SIZE=${RC_BUFFER_SIZE})
  message(STATUS Using buffer size ${RC_VERBOSE_LVEL})
endif()
if (DEFINED RC_DISABLE_CACHE_OPERATIONS)
  add_definitions(-DRC_DISABLE_CACHE_OPERATIONS=${RC_DISABLE_CACHE_OPERATIONS})
  message(STATUS Using disable cache operations ${RC_DISABLE_CACHE_OPERATIONS})
endif()

# Link with FIND_PACKAGE
target_link_libraries(radius-cacher PRIVATE ${LIBRARIES})

# Add manual includes
target_include_directories(radius-cacher PRIVATE ${INCLUDE_DIRS})
