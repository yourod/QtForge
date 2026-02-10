cmake_minimum_required(VERSION 3.16)

project(QtForge VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(QTFORGE_BUILD_TESTS_DEFAULT ON)
else()
    set(QTFORGE_BUILD_TESTS_DEFAULT OFF)
endif()

option(YOURLIB_BUILD_TESTS "Build QtForge unit tests" ${QTFORGE_BUILD_TESTS_DEFAULT})
option(YOURLIB_ENABLE_COVERAGE "Enable coverage instrumentation for GCC/Clang" OFF)

if(YOURLIB_ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-O0 -g --coverage)
        add_link_options(--coverage)
    else()
        message(WARNING "YOURLIB_ENABLE_COVERAGE is ON, but compiler is not GCC/Clang. Coverage flags are skipped.")
    endif()
endif()

# Automatically search for Qt6 in common installation locations
# This makes CMake fully self-contained and independent of folder structure
if(NOT Qt6_DIR AND NOT DEFINED ENV{Qt6_DIR})
    # Check common Qt installation paths
    set(QT_POSSIBLE_PATHS
        "$ENV{HOME}/Qt"
        "$ENV{HOME}/.local/Qt"
        "/usr/local/Qt"
        "/opt/Qt"
        "C:/Qt"  # Windows
        "$ENV{ProgramFiles}/Qt"  # Windows
    )
    
    # Find Qt6Config.cmake in common locations
    set(QT6_FOUND FALSE)
    foreach(QT_BASE_PATH ${QT_POSSIBLE_PATHS})
        if(EXISTS "${QT_BASE_PATH}" AND NOT QT6_FOUND)
            file(GLOB QT_VERSIONS "${QT_BASE_PATH}/*")
            foreach(QT_VERSION_PATH ${QT_VERSIONS})
                if(IS_DIRECTORY "${QT_VERSION_PATH}" AND NOT QT6_FOUND)
                    # Check for gcc_64, msvc2019_64, clang_64, etc.
                    file(GLOB QT_ARCH_DIRS "${QT_VERSION_PATH}/*")
                    foreach(QT_ARCH_DIR ${QT_ARCH_DIRS})
                        if(IS_DIRECTORY "${QT_ARCH_DIR}" AND NOT QT6_FOUND)
                            set(QT6_CONFIG_PATH "${QT_ARCH_DIR}/lib/cmake/Qt6/Qt6Config.cmake")
                            if(EXISTS "${QT6_CONFIG_PATH}")
                                get_filename_component(QT6_DIR "${QT6_CONFIG_PATH}" DIRECTORY)
                                set(Qt6_DIR "${QT6_DIR}" CACHE PATH "Qt6 directory" FORCE)
                                set(QT6_FOUND TRUE)
                                message(STATUS "Found Qt6 at: ${Qt6_DIR}")
                            endif()
                        endif()
                    endforeach()
                endif()
            endforeach()
        endif()
    endforeach()
endif()

# If still not found, try to use qmake6 or qmake to find Qt
if(NOT Qt6_DIR AND NOT DEFINED ENV{Qt6_DIR})
    find_program(QMAKE6 qmake6 PATHS
        "$ENV{HOME}/Qt/*/gcc_64/bin"
        "$ENV{HOME}/Qt/*/msvc2019_64/bin"
        "$ENV{HOME}/Qt/*/clang_64/bin"
        "/usr/bin"
        "/usr/local/bin"
    )
    
    if(QMAKE6)
        execute_process(
            COMMAND ${QMAKE6} -query QT_INSTALL_PREFIX
            OUTPUT_VARIABLE QT_INSTALL_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(QT_INSTALL_PREFIX)
            set(QT6_CONFIG_PATH "${QT_INSTALL_PREFIX}/lib/cmake/Qt6/Qt6Config.cmake")
            if(EXISTS "${QT6_CONFIG_PATH}")
                get_filename_component(QT6_DIR "${QT6_CONFIG_PATH}" DIRECTORY)
                set(Qt6_DIR "${QT6_DIR}" CACHE PATH "Qt6 directory" FORCE)
                message(STATUS "Found Qt6 via qmake6 at: ${Qt6_DIR}")
            endif()
        endif()
    endif()
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Quick)

qt_add_library(QtForgeLib STATIC
    QtForge.h
    QtForge.cpp
)

target_link_libraries(QtForgeLib PUBLIC
    Qt6::Core
    Qt6::Quick
)

target_include_directories(QtForgeLib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

set_target_properties(QtForgeLib PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

if(YOURLIB_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Install targets
install(TARGETS QtForgeLib
    EXPORT QtForgeTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES QtForge.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install CMake config files
include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/QtForgeConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/QtForgeConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QtForge
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/QtForgeConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/QtForgeConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/QtForgeConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QtForge
)

install(EXPORT QtForgeTargets
    FILE QtForgeTargets.cmake
    NAMESPACE QtForge::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QtForge
)
