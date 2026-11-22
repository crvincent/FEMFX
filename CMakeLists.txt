cmake_minimum_required(VERSION 3.20)

project(AMD_FEMFX
    VERSION 0.1.0
    DESCRIPTION "AMD FEMFX CPU physics library"
    LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/highway/hwy/highway.h")
    message(FATAL_ERROR
        "Google Highway is missing. Initialize submodules with: "
        "git submodule update --init --recursive")
endif()

add_subdirectory(amd_femfx)
