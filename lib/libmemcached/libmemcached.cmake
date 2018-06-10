include(ExternalProject)
include(GNUInstallDirs)

message(STATUS "Adding libmemcached external project")

externalproject_add(libmemcached-project
  PREFIX deps/libmemcached
  URL https://launchpad.net/libmemcached/1.0/1.0.18/+download/libmemcached-1.0.18.tar.gz
  URL_HASH MD5=b3958716b4e53ddc5992e6c49d97e819
  DOWNLOAD_DIR ${CMAKE_CURRENT_LIST_DIR}/pack/downloads
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/pack/libmemcached
  STAMP_DIR ${CMAKE_CURRENT_LIST_DIR}/pack/tmp/libmemcached
  TMP_DIR ${CMAKE_CURRENT_LIST_DIR}/pack/tmp/libmemcached
  BUILD_IN_SOURCE 1
  CONFIGURE_COMMAND ./configure
  BUILD_COMMAND make
  INSTALL_COMMAND ""
  )

set(LIBMEMCACHED_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/pack/libmemcached)
# set(LIBMEMCACHED_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/pack/libmemcached)

add_library(libmemcached STATIC IMPORTED)
# set_property(TARGET libmemcached PROPERTY IMPORTED_LOCATION ${LIBMEMCACHED_LIB_DIR}/libboost_chrono${BOOST_LIBRARY_SUFFIX})
add_dependencies(libmemcached libmemcached-project)
set(LIBMEMCACHED_LIBRARY libmemcached)
