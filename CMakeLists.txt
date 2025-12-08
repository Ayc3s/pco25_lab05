cmake_minimum_required(VERSION 3.13)
project(PCO25_LAB05)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-g)

find_package(Qt5 COMPONENTS Core Gui Widgets Test)
if (NOT Qt5_FOUND)
    find_package(Qt6 COMPONENTS Core Gui Widgets Test REQUIRED)
endif()

set(SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/bikinginterface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/display.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/velo.qrc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/bikestation.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/person.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/van.cpp
)

set(HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/include/bikinginterface.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/display.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/mainwindow.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/bike.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/bikestation.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/person.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/van.h
)

add_executable(pco_labo_biking ${SOURCES} ${HEADERS}
        include/config.h)
target_include_directories(pco_labo_biking PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(WITH_TSAN)
    target_compile_options(pco_labo_biking PRIVATE -fsanitize=thread)
    target_link_options(pco_labo_biking PRIVATE -fsanitize=thread)
endif()

if (NOT Qt5_FOUND)
    target_link_libraries(pco_labo_biking PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test pcosynchro)
else()
    target_link_libraries(pco_labo_biking PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Test pcosynchro)
endif()

file(COPY images/ DESTINATION ${CMAKE_BINARY_DIR}/images/)
