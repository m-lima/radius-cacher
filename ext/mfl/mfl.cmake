include(ExternalProject)
include(GNUInstallDirs)

ExternalProject_Add(mfl-project
  PREFIX deps/mfl
  GIT_REPOSITORY "https://github.com/m-lima/mfl.git"
  GIT_TAG "master"
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/pack/mfl
  STAMP_DIR ${CMAKE_CURRENT_LIST_DIR}/pack/tmp/mfl
  TMP_DIR ${CMAKE_CURRENT_LIST_DIR}/pack/tmp/mfl
  INSTALL_COMMAND ""
  )

add_library(mfl INTERFACE)

target_compile_definitions(mfl INTERFACE FMT_HEADER_ONLY=1)

target_include_directories(mfl INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/pack/mfl/include
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/pack/mfl/ext/fmt/pack/fmt/include>
  $<INSTALL_INTERFACE:include>)

add_dependencies(mfl mfl-project)

