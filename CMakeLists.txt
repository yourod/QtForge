cmake_minimum_required(VERSION 3.16)

set(CMAKE_AUTOMOC ON)

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
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:include>
)

if(DEFINED PROJECT_VERSION)
    set_target_properties(QtForgeLib PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
endif()

