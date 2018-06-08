include(ExternalProject)
include(GNUInstallDirs)

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

# set(BOOST_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/boost)
# set(BOOST_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/boost/stage/lib)

# add_library(boost::chrono STATIC IMPORTED)
# set_property(TARGET boost::chrono PROPERTY IMPORTED_LOCATION ${BOOST_LIB_DIR}/libboost_chrono${BOOST_LIBRARY_SUFFIX})
# add_dependencies(boost::chrono boost-project)

# add_library(boost::date_time STATIC IMPORTED)
# set_property(TARGET boost::date_time PROPERTY IMPORTED_LOCATION ${BOOST_LIB_DIR}/libboost_date_time${BOOST_LIBRARY_SUFFIX})
# add_dependencies(boost::date_time boost-project)

# add_library(boost::regex STATIC IMPORTED)
# set_property(TARGET boost::regex PROPERTY IMPORTED_LOCATION ${BOOST_LIB_DIR}/libboost_regex${BOOST_LIBRARY_SUFFIX})
# add_dependencies(boost::regex boost-project)

# add_library(boost::system STATIC IMPORTED)
# set_property(TARGET boost::system PROPERTY IMPORTED_LOCATION ${BOOST_LIB_DIR}/libboost_system${BOOST_LIBRARY_SUFFIX})
# add_dependencies(boost::system boost-project)

# add_library(boost::filesystem STATIC IMPORTED)
# set_property(TARGET boost::filesystem PROPERTY IMPORTED_LOCATION ${BOOST_LIB_DIR}/libboost_filesystem${BOOST_LIBRARY_SUFFIX})
# set_property(TARGET boost::filesystem PROPERTY INTERFACE_LINK_LIBRARIES boost::system)
# add_dependencies(boost::filesystem boost-project)

# add_library(boost::random STATIC IMPORTED)
# set_property(TARGET boost::random PROPERTY IMPORTED_LOCATION ${BOOST_LIB_DIR}/libboost_random${BOOST_LIBRARY_SUFFIX})
# add_dependencies(boost::random boost-project)

# add_library(boost::unit_test_framework STATIC IMPORTED)
# set_property(TARGET boost::unit_test_framework PROPERTY IMPORTED_LOCATION ${BOOST_LIB_DIR}/libboost_unit_test_framework${BOOST_LIBRARY_SUFFIX})
# add_dependencies(boost::unit_test_framework boost-project)

# add_library(boost::thread STATIC IMPORTED)
# set_property(TARGET boost::thread PROPERTY IMPORTED_LOCATION ${BOOST_LIB_DIR}/libboost_thread${BOOST_LIBRARY_SUFFIX})
# set_property(TARGET boost::thread PROPERTY INTERFACE_LINK_LIBRARIES boost::chrono boost::date_time boost::regex)
# add_dependencies(boost::thread boost-project)

