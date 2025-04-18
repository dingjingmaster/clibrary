cmake_minimum_required(VERSION 3.10)
project(clibrary LANGUAGES C CXX)

set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 2)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION_TWEAK 0)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Debug" FORCE)
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(FLAGS "-O3 -Wall")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS}")
    add_definitions(-DDEBUG=0 -w)
elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_VERBOSE_MAKEFILE ON)
    add_definitions(-g -DDEBUG=1 -Werror)
endif ()

include_directories(${CMAKE_SOURCE_DIR})
add_definitions(-DQT_NO_KEYWORDS -DQT_MESSAGELOGCONTEXT -D__CLIB_H_INSIDE__ -DPACKAGE_NAME=\"clib\")

enable_testing()

find_package(PkgConfig)
find_package(Qt5 COMPONENTS Core)

pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(QT_CORE REQUIRED Qt5Core)
pkg_check_modules(GOBJECT REQUIRED gobject-2.0)

add_subdirectory(c)
#add_subdirectory(qt5)
add_subdirectory(glib)
#add_subdirectory(demo)
add_subdirectory(test)

include(data/data.cmake)
