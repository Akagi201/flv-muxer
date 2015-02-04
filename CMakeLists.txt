#
# @file CMakeLists.txt
# @author Akagi201
# @date 2015/02/04
#

cmake_minimum_required(VERSION 2.8.4)
project(flv_muxer)

set(SOURCE_FILES src/main.c src/flv-muxer.c)

include_directories("/usr/local/include" "${PROJECT_SOURCE_DIR}/deps")

link_directories("/usr/local/lib")

add_executable(flv_muxer ${SOURCE_FILES})
