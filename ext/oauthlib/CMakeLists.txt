cmake_minimum_required(VERSION 2.6)

project(oauthlib)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

include_directories(${INC_DIR})
add_library(oauthlib
  ${SRC_DIR}/oauthlib.cpp
  ${SRC_DIR}/base64.cpp
  ${SRC_DIR}/HMAC_SHA1.cpp
  ${SRC_DIR}/SHA1.cpp
  ${SRC_DIR}/urlencode.cpp)
