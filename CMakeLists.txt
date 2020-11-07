cmake_minimum_required(VERSION 3.13)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

set(APP_NAME "DMRHost")
project(${APP_NAME} CXX)

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

add_executable(${APP_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${APP_NAME} ${DEPLIBS})

include(GNUInstallDirs)
install (TARGETS ${APP_NAME} RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install (FILES ${PROJECT_SOURCE_DIR}/MMDVM.ini DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}")
