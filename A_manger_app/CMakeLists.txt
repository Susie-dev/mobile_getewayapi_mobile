cmake_minimum_required(VERSION 3.16)
project(ManagerApp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 COMPONENTS Quick Network WebEngineQuick REQUIRED)

qt_add_executable(appManagerApp
    main.cpp
    ManagerNetwork.cpp
    ManagerNetwork.h
)

qt_add_qml_module(appManagerApp
    URI ManagerApp
    VERSION 1.0
    QML_FILES 
        Main.qml
        MapDashboard.qml
        OrderListPage.qml
    RESOURCES map.html
)

target_link_libraries(appManagerApp
    PRIVATE Qt6::Quick Qt6::Network Qt6::WebEngineQuick
)
