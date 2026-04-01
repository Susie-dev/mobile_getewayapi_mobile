cmake_minimum_required(VERSION 3.16)
project(ClientApp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 COMPONENTS Quick Network Sql Positioning REQUIRED)

qt_add_executable(appClientApp
    main.cpp
    DataSimulator.cpp
    DataSimulator.h
    OrderDatabase.cpp
    OrderDatabase.h
    OrderModel.cpp
    OrderModel.h
    LLMService.cpp
    LLMService.h
)

qt_add_qml_module(appClientApp
    URI ClientApp
    VERSION 1.0
    QML_FILES 
        Main.qml
        LoginPage.qml
        CreateOrderPage.qml
        TransportPage.qml
)

target_link_libraries(appClientApp
    PRIVATE Qt6::Quick Qt6::Network Qt6::Sql Qt6::Positioning
)
