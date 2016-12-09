project(mysqlpat)
cmake_minimum_required(VERSION 3.0.2)
aux_source_directory(. SRC_LIST)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)


# Find the QtWidgets library
find_package(Qt5Widgets)

option(BUILTIN_QWT "BUILTIN_QWT" OFF)
if (NOT BUILTIN_QWT)
    find_package(Qwt)
endif(NOT BUILTIN_QWT)

if (NOT QWT_FOUND)
    message(WARNING "Failed find system Qwt Library - using built-in Qwt")
    set(QWT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/qwt/qwt/src)
    if (NOT EXISTS ${QWT_SRC_DIR})
        message(FATAL_ERROR "Qwt submodule not configured (git submodule update --init --recursive)")
    endif ()
    find_package(Qt5PrintSupport REQUIRED)
    find_package(Qt5Svg REQUIRED)
    find_package(Qt5Concurrent REQUIRED)
    find_package(Qt5OpenGL REQUIRED)
    set(QWT_LIBRARIES Qt5::PrintSupport Qt5::Svg Qt5::Concurrent Qt5::OpenGL)
    set(QWT_INCLUDE_DIRS ${QWT_SRC_DIR})
    aux_source_directory(${QWT_SRC_DIR} SRC_LIST)
endif()

include_directories(${QWT_INCLUDE_DIRS})

set (UIS
mainwindow.ui
graphschoser.ui
)

qt5_wrap_ui(UI_HEADERS ${UIS})

add_executable(${PROJECT_NAME} ${SRC_LIST} ${UI_HEADERS})



# Use the Widgets module from Qt 5.
target_link_libraries(${PROJECT_NAME} Qt5::Widgets ${QWT_LIBRARIES})
