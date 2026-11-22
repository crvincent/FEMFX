set(FEMFX_BACKEND "RUNTIME" CACHE STRING "FEMFX CPU math backend")
set_property(CACHE FEMFX_BACKEND PROPERTY STRINGS RUNTIME SCALAR)
set(FEMFX_EXPECT_ARM64 OFF CACHE BOOL "Require the configured target to be ARM64")

if(NOT CMAKE_SYSTEM_PROCESSOR)
    set(FEMFX_TARGET_PROCESSOR "${CMAKE_HOST_SYSTEM_PROCESSOR}")
else()
    set(FEMFX_TARGET_PROCESSOR "${CMAKE_SYSTEM_PROCESSOR}")
endif()
string(TOLOWER "${FEMFX_TARGET_PROCESSOR}" FEMFX_TARGET_PROCESSOR_LOWER)

if(FEMFX_TARGET_PROCESSOR_LOWER MATCHES "^(aarch64|arm64)")
    set(FEMFX_IS_ARM64 TRUE)
elseif(FEMFX_TARGET_PROCESSOR_LOWER MATCHES "^(amd64|x86_64|x64|i[3-6]86|x86)")
    set(FEMFX_IS_X64 TRUE)
else()
    message(FATAL_ERROR "Unsupported target processor '${FEMFX_TARGET_PROCESSOR}'. FEMFX supports x86/x64 and ARM64.")
endif()

if(FEMFX_EXPECT_ARM64 AND NOT FEMFX_IS_ARM64)
    message(FATAL_ERROR
        "The ARM64 preset requires a native ARM64 environment, but the detected target is "
        "'${FEMFX_TARGET_PROCESSOR}'.")
endif()

if(FEMFX_IS_ARM64 AND WIN32 AND MSVC AND NOT CMAKE_CXX_COMPILER_ARCHITECTURE_ID STREQUAL "ARM64")
    message(FATAL_ERROR
        "Windows ARM64 requires an ARM64 Visual C++ compiler. "
        "Open an ARM64 developer environment with the MSVC ARM64 build tools installed.")
endif()

string(TOUPPER "${FEMFX_BACKEND}" FEMFX_BACKEND)

if(FEMFX_BACKEND STREQUAL "AUTO")
    set(FEMFX_BACKEND RUNTIME)
endif()

if(NOT FEMFX_BACKEND STREQUAL "RUNTIME" AND NOT FEMFX_BACKEND STREQUAL "SCALAR")
    message(FATAL_ERROR "Unsupported FEMFX_BACKEND '${FEMFX_BACKEND}'. Use RUNTIME or SCALAR.")
endif()

file(GLOB_RECURSE FEMFX_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
list(APPEND FEMFX_SOURCES "${PROJECT_SOURCE_DIR}/highway/hwy/targets.cc")
list(APPEND FEMFX_SOURCES "${PROJECT_SOURCE_DIR}/highway/hwy/abort.cc")

add_library(amd_femfx STATIC ${FEMFX_SOURCES})
add_library(AMD::FEMFX ALIAS amd_femfx)

set_target_properties(amd_femfx PROPERTIES
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
    EXPORT_NAME FEMFX)

target_compile_features(amd_femfx PUBLIC cxx_std_17)

target_include_directories(amd_femfx
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc/Vectormath>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Vectormath>"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Bvh"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Common"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ConstraintSolver"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/FindContacts"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/PrimitiveCollision"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Scene"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Simulation"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/SparseSolvers"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Threading"
        "${CMAKE_CURRENT_SOURCE_DIR}/inc/Vectormath")
target_include_directories(amd_femfx PRIVATE
    "${PROJECT_SOURCE_DIR}/highway")

target_compile_definitions(amd_femfx
    PUBLIC
        FEMFX_SIMD_${FEMFX_BACKEND}=1)
target_compile_definitions(amd_femfx PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>)

# Windows and common 64-bit integer widths are part of the legacy implementation.
if(WIN32)
    target_compile_definitions(amd_femfx PRIVATE WIN32 NOMINMAX)
endif()

if(MSVC)
    target_compile_options(amd_femfx PRIVATE
        /W4
        /EHsc
        /utf-8)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(amd_femfx PRIVATE
        -Wall
        -Wextra)
endif()

install(TARGETS amd_femfx
    EXPORT AMD_FEMFXTargets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/inc/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    FILES_MATCHING PATTERN "*.h")

install(EXPORT AMD_FEMFXTargets
    FILE AMD_FEMFXTargets.cmake
    NAMESPACE AMD::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/AMD_FEMFX")

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/AMD_FEMFXConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/AMD_FEMFXConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/AMD_FEMFX")

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/AMD_FEMFXConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/AMD_FEMFXConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/AMD_FEMFXConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/AMD_FEMFX")

install(FILES
    "${PROJECT_SOURCE_DIR}/LICENSE.txt"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/AMD_FEMFX/licenses"
    RENAME FEMFX-LICENSE.txt)
install(FILES
    "${PROJECT_SOURCE_DIR}/NOTICES.txt"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/AMD_FEMFX/licenses"
    RENAME FEMFX-NOTICES.txt)
install(FILES
    "${PROJECT_SOURCE_DIR}/highway/LICENSE"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/AMD_FEMFX/licenses"
    RENAME HIGHWAY-LICENSE.txt)

message(STATUS "AMD FEMFX target processor: ${FEMFX_TARGET_PROCESSOR}")
if(FEMFX_IS_ARM64)
    message(STATUS "AMD FEMFX target architecture: ARM64")
elseif(FEMFX_IS_X64)
    message(STATUS "AMD FEMFX target architecture: x86/x64")
endif()
message(STATUS "AMD FEMFX backend: ${FEMFX_BACKEND}")
message(STATUS "AMD FEMFX compiler: ${CMAKE_CXX_COMPILER_ID}")
