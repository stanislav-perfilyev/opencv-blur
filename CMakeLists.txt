cmake_minimum_required(VERSION 3.16)
project(Blur)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT DEFINED QT_PATH)
    if(DEFINED ENV{QT_PATH})
        set(QT_PATH $ENV{QT_PATH})
    else()
        set(QT_PATH "D:/Qt/6.10.2/mingw_64")
    endif()
endif()
set(CMAKE_PREFIX_PATH "${QT_PATH}/lib/cmake")

find_package(Qt6 COMPONENTS Core Gui Widgets Concurrent REQUIRED)

# ── Shared logic library (testable without QApplication) ──────────────────
qt_add_library(blur_lib STATIC
    src/mainwindow.cpp
    include/mainwindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Blur.ui
)
target_include_directories(blur_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(blur_lib PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Concurrent)

# ── Application ───────────────────────────────────────────────────────────
qt_add_executable(Blur src/main.cpp)
target_link_libraries(Blur PRIVATE blur_lib)

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if(NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if(NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif()
    endif()
    foreach(plugin qwindows)
        if(EXISTS "${QT_INSTALL_PATH}/plugins/platforms/${plugin}${DEBUG_SUFFIX}.dll")
            add_custom_command(TARGET Blur POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                    "$<TARGET_FILE_DIR:Blur>/plugins/platforms/")
            add_custom_command(TARGET Blur POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                    "${QT_INSTALL_PATH}/plugins/platforms/${plugin}${DEBUG_SUFFIX}.dll"
                    "$<TARGET_FILE_DIR:Blur>/plugins/platforms/")
        endif()
    endforeach()
    foreach(plugin qjpeg qpng qgif)
        if(EXISTS "${QT_INSTALL_PATH}/plugins/imageformats/${plugin}${DEBUG_SUFFIX}.dll")
            add_custom_command(TARGET Blur POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                    "$<TARGET_FILE_DIR:Blur>/plugins/imageformats/")
            add_custom_command(TARGET Blur POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                    "${QT_INSTALL_PATH}/plugins/imageformats/${plugin}${DEBUG_SUFFIX}.dll"
                    "$<TARGET_FILE_DIR:Blur>/plugins/imageformats/")
        endif()
    endforeach()
    foreach(QT_LIB Core Gui Widgets Concurrent)
        add_custom_command(TARGET Blur POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:Blur>")
    endforeach()
endif()

# ── Unit tests (GTest) ────────────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

qt_add_executable(test_blur tests/test_blur.cpp)
target_link_libraries(test_blur PRIVATE blur_lib GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(test_blur)
