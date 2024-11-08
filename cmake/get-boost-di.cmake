include(FetchContent)

set(BOOSTDI_SRC_DIR ${CMAKE_BINARY_DIR}/_deps/boost-di)

FetchContent_Declare(
   boost-di-header
   URL https://raw.githubusercontent.com/boost-ext/di/cpp14/include/boost/di.hpp
   DOWNLOAD_NO_EXTRACT TRUE
)

FetchContent_Populate(boost-di-header)

variable_watch(boost-di-header_SOURCE_DIR)

add_library(BoostDI INTERFACE)

target_include_directories(BoostDI
   INTERFACE
   boost-di-header_SOURCE_DIR
)
