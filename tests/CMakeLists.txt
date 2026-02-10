include(FetchContent)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)

FetchContent_MakeAvailable(googletest)

add_executable(qtforge_tests
    test_main.cpp
    helpers/qtforge_test_utils.cpp
    test_qtforge__engine_loading.cpp
    test_qtforge__bindings.cpp
    test_qtforge__contexts.cpp
    test_qtforge__property_map_and_singleton.cpp
    test_qtforge__component_property_registration.cpp
    test_qtforge__component_creation.cpp
)

target_link_libraries(qtforge_tests
    PRIVATE
        QtForgeLib
        GTest::gtest
)

target_include_directories(qtforge_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(qtforge_tests
    PRIVATE
        QTFORGE_TEST_QML_DIR="${CMAKE_CURRENT_SOURCE_DIR}/qml"
)

include(GoogleTest)
gtest_discover_tests(qtforge_tests)

if(YOURLIB_ENABLE_COVERAGE)
    find_program(GCOVR_EXECUTABLE gcovr)
    if(GCOVR_EXECUTABLE)
        add_custom_target(coverage
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${GCOVR_EXECUTABLE}
                --root ${PROJECT_SOURCE_DIR}
                --filter ${PROJECT_SOURCE_DIR}
                --exclude ${PROJECT_SOURCE_DIR}/tests
                --txt-metric branch
                --txt ${CMAKE_BINARY_DIR}/coverage.txt
                --html-details ${CMAKE_BINARY_DIR}/coverage.html
                --print-summary
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS qtforge_tests
            USES_TERMINAL
        )
    else()
        message(WARNING "YOURLIB_ENABLE_COVERAGE is ON but gcovr was not found in PATH.")
    endif()
endif()
