cmake_minimum_required(VERSION 2.6)

project(QuacksTwit)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(compiler_option)

option(USE_CURL "Use Curl, Not using other system library(ex. Accounts for OSX)" OFF)

find_program(PYTHON NAMES python)
if(NOT PYTHON)
  message(FATAL_ERROR "Python is needed to generate data classes")
endif()

if(USE_CURL)
  set(QUACKSTWIT_BACKEND CURL)
else()
  if(APPLE)
    find_library(ACCOUNTS_FW Accounts)
    find_library(SOCIAL_FW Social)
    find_library(DEFAULT_FW Foundation)
    if(ACCOUNTS_FW AND SOCIAL_FW)
      set(QUACKSTWIT_BACKEND Accounts)
    else()
      set(USE_CURL ON)
    endif()
  elseif(ANDROID)
  else()
    set(QUACKSTWIT_BACKEND CURL)
  endif()
endif()

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  )

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(DEF_DIR ${CMAKE_CURRENT_SOURCE_DIR}/def)
set(GEN_PY ${CMAKE_CURRENT_SOURCE_DIR}/gen.py)

list(APPEND QUACKSTWIT_REST_DEFS
  ${DEF_DIR}/rest/statuses.json)
list(APPEND QUACKSTWIT_STREAM_DEFS
  ${DEF_DIR}/stream.json)

set(QUACKSTWIT_REST_OUT
  ${SRC_DIR}/rest.cpp
  ${INC_DIR}/rest.h)
add_custom_command(
  OUTPUT ${QUACKSTWIT_REST_OUT}
  DEPENDS ${QUACKSTWIT_REST_DEFS}
  COMMAND python ${GEN_PY} -n Quacks::Twit -o rest ${QUACKSTWIT_REST_DEFS})

set(QUACKSTWIT_STREAM_OUT
  ${SRC_DIR}/stream.cpp
  ${INC_DIR}/stream.h)
add_custom_command(
  OUTPUT ${QUACKSTWIT_STREAM_OUT}
  DEPENDS ${QUACKSTWIT_STREAM_DEFS}
  COMMAND python ${GEN_PY} -n Quacks::Twit -o stream ${QUACKSTWIT_STREAM_DEFS})

list(APPEND QUACKSTWIT_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/include/accountStore.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/accountStore.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/account.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/account.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/stream.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/stream.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/rest.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/rest.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/request.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/request.cpp)

if("${QUACKSTWIT_BACKEND}" STREQUAL "Accounts")
  list(APPEND QUACKSTWIT_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/apple/account.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/src/apple/request.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/src/apple/accountStore.mm)
  list(REMOVE_ITEM QUACKSTWIT_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/account.cpp)
  list(APPEND QUACKSTWIT_LIBS ${ACCOUNTS_FW} ${SOCIAL_FW} ${DEFAULT_FW})
else()
endif()

add_library(QuacksTwit ${QUACKSTWIT_SRCS})
target_link_libraries(QuacksTwit ${QUACKSTWIT_LIBS})

set(QUACKSTWITCLI_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/cli/main.cpp)
if(MSVC)
  set(exe_option WIN32)
elseif(APPLE)
  set(exe_option MACOSX_BUNDLE)
endif()

add_executable(QuacksTwitCli
  ${exe_option}
  ${QUACKSTWITCLI_SRCS})
target_link_libraries(QuacksTwitCli QuacksTwit)

if(APPLE)
  set_target_properties(QuacksTwitCli PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/cli/Info.plist
    MACOSX_BUNDLE_GUI_IDENTIFIER "kr.shipduck.$(PRODUCT_NAME:rfc1034identifier)"
    MACOSX_BUNDLE_BUNDLE_NAME "$(PRODUCT_NAME)"
    MACOSX_BUNDLE_LONG_VERSION_STRING 0.1
    MACOSX_BUNDLE_SHORT_VERSION_STRING 1.0
    MACOSX_BUNDLE_BUNDLE_VERSION 1)
endif()

USE_CPP11(TARGET QuacksTwit)
USE_CPP11(TARGET QuacksTwitCli)
