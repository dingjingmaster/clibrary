cmake_minimum_required(VERSION 3.27)
project(clibrary LANGUAGES C CXX)

set(DEBUG ON)
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(FLAGS "-O3 -Wall")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS}")

add_definitions(-g -DDEBUG=${DEBUG} -DQT_NO_KEYWORDS -DQT_MESSAGELOGCONTEXT)

find_package(PkgConfig)
find_package(Qt5 COMPONENTS Core)

pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(QT_CORE REQUIRED Qt5Core)
pkg_check_modules(GOBJECT REQUIRED gobject-2.0)

add_subdirectory(c)
add_subdirectory(qt5)
add_subdirectory(glib)
add_subdirectory(demo)
add_subdirectory(test)
