cmake_minimum_required(VERSION 3.28)
project(password)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.7.3/msvc2022_64")

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        WebSockets
        Network
        REQUIRED)

add_executable(password
        src/main.cpp
        src/PasswordWindow.h
        src/PasswordWindow.cpp
        src/ScreenPage.h
        src/ScreenPage.cpp
        src/AttackPanel.h
        src/AttackPanel.cpp
        src/AttackScreen.h
        src/AttackScreen.cpp
        src/PasswordWsServer.h
        src/PasswordWsServer.cpp
        src/AppConfig.h
        src/AppConfig.cpp
        src/UdpBeacon.h
        src/UdpBeacon.cpp
        src/cybershow/common/CyberAppMode.h
        src/cybershow/common/CyberAppMode.cpp
        src/cybershow/common/CyberOperationalLog.h
        src/cybershow/common/CyberOperationalLog.cpp
        src/cybershow/common/CyberOrchestratorProtocol.h
        src/cybershow/ui/BottomNavBar.h
        src/cybershow/ui/BottomNavBar.cpp
        src/cybershow/ui/CyberBackgroundWidget.h
        src/cybershow/ui/CyberBackgroundWidget.cpp
        src/cybershow/ui/CyberTheme.h
        src/cybershow/ui/CyberTheme.cpp)

target_link_libraries(password
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::WebSockets
        Qt::Network
)

target_include_directories(password PRIVATE
        src
        src/cybershow/common
        src/cybershow/ui)

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB Core Gui Widgets WebSockets Network)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()

