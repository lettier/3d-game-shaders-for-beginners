list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

if (WIN32)
  include(WindowsUtils)
endif()

cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(game-shaders LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# specify custom panda3d root directory for most Mac OS users
if(APPLE AND NOT DEFINED ENV{panda3d_ROOT})
  set(panda3d_ROOT "/Developer/Panda3D")
endif()

find_package(panda3d REQUIRED COMPONENTS p3framework p3direct)

add_executable(${PROJECT_NAME} main.cxx)
target_link_libraries(${PROJECT_NAME} PRIVATE panda3d::p3framework panda3d::p3direct)
