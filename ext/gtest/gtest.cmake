include(ExternalProject)
include(GNUInstallDirs)

ExternalProject_Add(gtest-project
  PREFIX deps/gtest
  GIT_REPOSITORY "https://github.com/google/googletest.git"
  GIT_TAG "master"
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/pack/gtest
  STAMP_DIR ${CMAKE_CURRENT_LIST_DIR}/pack/tmp/gtest
  TMP_DIR ${CMAKE_CURRENT_LIST_DIR}/pack/tmp/gtest
  INSTALL_COMMAND ""
  )

ExternalProject_Get_Property(gtest-project binary_dir)

add_library(gtest STATIC IMPORTED)
set_target_properties(gtest PROPERTIES
    IMPORTED_LOCATION "${binary_dir}/googlemock/gtest/libgtest.a"
    IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/pack/gtest/googletest/include"
    )
add_dependencies(gtest gtest-project)

add_library(gtest_main STATIC IMPORTED)
set_target_properties(gtest_main PROPERTIES
    IMPORTED_LOCATION "${binary_dir}/googlemock/gtest/libgtest_main.a"
    IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/pack/gtest/googletest/include"
    )
add_dependencies(gtest_main gtest-project)

add_library(gmock STATIC IMPORTED)
set_target_properties(gmock PROPERTIES
    IMPORTED_LOCATION "${binary_dir}/googlemock/libgmock.a"
    IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/pack/gtest/googlemock/include"
    )
add_dependencies(gmock gtest-project)

add_library(gmock_main STATIC IMPORTED)
set_target_properties(gmock_main PROPERTIES
    IMPORTED_LOCATION "${binary_dir}/googlemock/libgmock_main.a"
    IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/pack/gtest/googlemock/include"
    )
add_dependencies(gmock_main gtest-project)
