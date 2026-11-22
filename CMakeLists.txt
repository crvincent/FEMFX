cmake_minimum_required(VERSION 3.20)

project(AMD_FEMFX
    VERSION 0.1.0
    DESCRIPTION "AMD FEMFX CPU physics library"
    LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_subdirectory(amd_femfx)
