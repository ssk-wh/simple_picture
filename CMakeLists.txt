cmake_minimum_required(VERSION 3.16)
project(EasyPicture VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Generate version.h from template
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
)

# Qt 5.11+
find_package(Qt5 5.11 REQUIRED COMPONENTS Core Gui Widgets Concurrent Svg Test)

# 主程序源文件
set(CORE_SOURCES
    src/core/ImageLoader.cpp
    src/core/ImageCache.cpp
    src/core/ImageNavigator.cpp
)

set(CORE_HEADERS
    src/core/ImageLoader.h
    src/core/ImageCache.h
    src/core/ImageNavigator.h
)

set(UI_SOURCES
    src/ui/ImageView.cpp
    src/ui/MainWindow.cpp
    src/ui/ChangelogDialog.cpp
)

set(UI_HEADERS
    src/ui/ImageView.h
    src/ui/MainWindow.h
    src/ui/ChangelogDialog.h
)

set(APP_SOURCES
    src/main.cpp
    ${CORE_SOURCES}
    ${CORE_HEADERS}
    ${UI_SOURCES}
    ${UI_HEADERS}
    resources.qrc
)

if(WIN32)
    list(APPEND APP_SOURCES resources/app.rc)
    add_executable(EasyPicture WIN32 ${APP_SOURCES})
else()
    add_executable(EasyPicture ${APP_SOURCES})
    install(TARGETS EasyPicture DESTINATION /usr/bin)
    install(FILES resources/easypicture.desktop DESTINATION /usr/share/applications)
    install(FILES resources/app-icon.svg DESTINATION /usr/share/icons/hicolor/scalable/apps RENAME easypicture.svg)
endif()

target_include_directories(EasyPicture PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(EasyPicture PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Concurrent
    Qt5::Svg
)

# 单元测试
option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.14.0
        )
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()

    add_executable(EasyPictureTests
        tests/test_main.cpp
        tests/test_ImageLoader.cpp
        tests/test_ImageCache.cpp
        tests/test_ImageNavigator.cpp
        ${CORE_SOURCES}
        ${CORE_HEADERS}
    )

    target_include_directories(EasyPictureTests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(EasyPictureTests PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Concurrent
        Qt5::Svg
        Qt5::Test
        GTest::gtest
    )

    include(GoogleTest)
    gtest_discover_tests(EasyPictureTests)
endif()
