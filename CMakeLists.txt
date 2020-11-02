cmake_minimum_required(VERSION 3.13)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

set(APP_NAME "MMDVMHost")
project(${APP_NAME} CXX)

option(ENABLE_OLED "Enable OLED support" OFF)

if(GIT_VERSION)
  set(GIT_HASH ${GIT_VERSION})
else()
  set(GIT_HASH "0000000000")
endif()
file(WRITE ${CMAKE_SOURCE_DIR}/GitVersion.h "const char *gitversion = \"${GIT_HASH}\";")

file(GLOB SOURCES "*.cpp")
file(GLOB HEADERS "*.h")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -Wall -std=c++0x $ENV{CXXFLAGS}")
set(DEPLIBS "")

find_package(Threads REQUIRED QUIET)
list(APPEND DEPLIBS ${CMAKE_THREAD_LIBS_INIT})

if(ENABLE_OLED)
  find_package(ArduiPi_OLED REQUIRED)
  include_directories(${ARDUIPI_OLED_INCLUDE_DIRS})
  list(APPEND DEPLIBS ${ARDUIPI_OLED_LIBRARIES})

  find_package(I2c REQUIRED)
  include_directories(${I2C_INCLUDE_DIRS})
  list(APPEND DEPLIBS ${I2C_LIBRARIES})

  add_definitions(-DOLED)
else()
  list(REMOVE_ITEM SOURCES ${CMAKE_SOURCE_DIR}/OLED.cpp)
  list(REMOVE_ITEM HEADERS ${CMAKE_SOURCE_DIR}/OLED.h)
endif()

add_executable(${APP_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${APP_NAME} ${DEPLIBS})
