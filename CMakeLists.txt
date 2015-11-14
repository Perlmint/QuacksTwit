cmake_minimum_required(VERSION 2.6)

project(QuacksTwit)

if(POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(compiler_option)

option(USE_CURL "Use Curl, Not using other system library(ex. Accounts for OSX)" OFF)

find_program(PYTHON NAMES python)
if(NOT PYTHON)
  message(FATAL_ERROR "Python is needed to generate data classes")
endif()

if(NOT USE_CURL)
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

if(USE_CURL)
  message("-- Use CURL")
  set(QUACKSTWIT_BACKEND CURL)
  find_library(CURL_LIB curl libcurl
    HINTS ${CMAKE_CURRENT_SOURCE_DIR}/ext/curl)
  if(NOT CURL_LIB_FOUND)
    message("-- prebuilt CURL not found, try building from source")
    set(BUILD_CURL_EXE OFF)
    set(BUILD_CURL_TESTS OFF)
    set(CURL_STATICLIB OFF)
    set(CURL_HTTP_ONLY ON)
    set(CURL_DISABLE_FTP ON)
    set(CURL_DISABLE_TELNET ON)
    set(CURL_DISABLE_RTSP ON)
    set(CURL_DISABLE_POP3 ON)
    set(CURL_DISABLE_IMAP ON)
    set(CURL_DISABLE_SMTP ON)
    set(CURL_DISABLE_GOPHER ON)
    set(CURL_DISABLE_TFTP ON)
    set(CURL_DISABLE_DICT ON)
    set(CURL_DISABLE_FILE ON)
    set(CMAKE_USE_LIBSSH2 OFF)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ext/curl)
    list(APPEND QUACKSTWIT_LIBS libcurl)
  else()
    message("-- prebuilt CURL found, Use it")
    list(APPEND QUACKSTWIT_LIBS ${CURL_LIB})
  endif()
endif()

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(EXT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext)

set(DEF_DIR ${CMAKE_CURRENT_SOURCE_DIR}/def)
set(GEN_PY ${CMAKE_CURRENT_SOURCE_DIR}/gen.py)

list(APPEND QUACKSTWIT_REST_DEFS
  ${DEF_DIR}/rest/statuses.json)
list(APPEND QUACKSTWIT_STREAM_DEFS
  ${DEF_DIR}/stream.json)

include_directories(
  ${INC_DIR}
  ${SRC_DIR}
  ${EXT_DIR}
  )

macro(gen NAME)
  set(GEN_OUT
    ${SRC_DIR}/gen/${NAME}.cpp
    ${INC_DIR}/gen/${NAME}.h)
  if("${NAME}" STREQUAL "type")
    set(GEN_OPT -t -o type)
  else()
    set(GEN_OPT -o ${NAME})
  endif()
  add_custom_command(
    OUTPUT ${GEN_OUT}
    DEPENDS ${ARGN} ${GEN_PY}
    COMMAND python ${GEN_PY} ${GEN_OPT} -n Quacks::Twit ${ARGN})
  list(APPEND QUACKSTWIT_SRCS ${GEN_OUT})
endmacro(gen)

gen(rest
  ${DEF_DIR}/rest/statuses.json)
gen(stream
  ${DEF_DIR}/stream.json)
gen(type
  ${DEF_DIR}/type/twit.json
  ${DEF_DIR}/type/user.json)

list(APPEND QUACKSTWIT_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/include/accountStore.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/accountStore.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/account.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/request.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/parseUtil.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/color.h)

if("${QUACKSTWIT_BACKEND}" STREQUAL "Accounts")
  list(APPEND QUACKSTWIT_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/apple/account.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/src/apple/request.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/src/apple/accountStore.mm)
  list(APPEND QUACKSTWIT_LIBS ${ACCOUNTS_FW} ${SOCIAL_FW} ${DEFAULT_FW})
elseif("${QUACKSTWIT_BACKEND}" STREQUAL "CURL")
  list(APPEND QUACKSTWIT_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/curl/account.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/curl/request.cpp)
endif()

add_library(QuacksTwit ${QUACKSTWIT_SRCS})
target_link_libraries(QuacksTwit ${QUACKSTWIT_LIBS})

set(QUACKSTWITCLI_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/cli/main.cpp)
if(APPLE)
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
