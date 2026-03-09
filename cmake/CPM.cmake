# CPM.cmake - CMake Package Manager
# Version 0.38.6
# https://github.com/cpm-cmake/CPM.cmake
# SPDX-License-Identifier: MIT

set(CPM_DOWNLOAD_VERSION 0.38.6)
set(CPM_HASH_SUM "11c3fa5f1ba14f15d31c2fb63dbc8628ee133d81c8d764caad9a8db9e0bacb07")

if(CPM_SOURCE_CACHE)
  set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif(DEFINED ENV{CPM_SOURCE_CACHE})
  set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else()
  set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif()

get_filename_component(CPM_DOWNLOAD_LOCATION_DIR ${CPM_DOWNLOAD_LOCATION} DIRECTORY)

if(NOT EXISTS ${CPM_DOWNLOAD_LOCATION})
  file(MAKE_DIRECTORY ${CPM_DOWNLOAD_LOCATION_DIR})
  message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
  file(DOWNLOAD
       https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
       ${CPM_DOWNLOAD_LOCATION}
       EXPECTED_HASH SHA256=${CPM_HASH_SUM}
  )
endif()

include(${CPM_DOWNLOAD_LOCATION})