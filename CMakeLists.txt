cmake_minimum_required(VERSION 3.16)
project(IoT VERSION 1.0.0 LANGUAGES CXX)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Автоматическое подключение MOC, UIC и RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Поиск Qt (укажите нужные модули)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# 1. Сначала ищем C-библиотеку Paho MQTT
# find_package(PahoMqttC REQUIRED)
# 2. Затем ищем C++-библиотеку (она зависит от C-версии)
# find_package(PahoMqttCpp REQUIRED)
find_package(eclipse-paho-mqtt-c REQUIRED)

# Добавление исходных файлов
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mqtt_callback.cpp
    src/topic.cpp
    src/subscribe.cpp
)

# Добавление заголовочных файлов
set(HEADERS
    src/mainwindow.h
    src/mqtt_callback.h
    src/topic.h
    src/subscribe.h
)

# Добавление UI-форм
set(FORMS
    src/mainwindow.ui
)

# Добавление ресурсов (если есть)
# set(RESOURCES
#     resources/icons.qrc
#     resources/images.qrc
# )

# Создание исполняемого файла
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${FORMS}
    # ${RESOURCES}
)

# Подключение библиотек Qt
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    
    eclipse-paho-mqtt-c::paho-mqtt3a-static
    # PahoMqttCpp::paho-mqttpp3-static
)
