project(mysqlpat)
cmake_minimum_required(VERSION 2.8.0)
aux_source_directory(. SRC_LIST)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)

set( VERSION_MAJOR "0" )
set( VERSION_MINOR "0" )
set( VERSION_PATCH "1" )

# Find the QtWidgets library
find_package(Qt5Widgets)

option(BUILTIN_QWT "BUILTIN_QWT" OFF)
if (NOT BUILTIN_QWT)
    find_package(Qwt)
endif(NOT BUILTIN_QWT)

if (NOT QWT_FOUND)
    message(WARNING "Failed find system Qwt Library - using built-in Qwt")
    set(QWT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/qwt/src)
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

if(APPLE)
	set(EXECUTABLE_TYPE MACOSX_BUNDLE)
endif(APPLE)

if(WIN32)
	set(EXECUTABLE_TYPE WIN32)
endif(WIN32)

if(LINUX)
	set(EXECUTABLE_TYPE "")
endif(LINUX)


add_executable(${PROJECT_NAME} ${EXECUTABLE_TYPE} ${SRC_LIST} ${UI_HEADERS})



if( APPLE )
	# Define some settings for the Bundle
	set( MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME} )
	set( MACOSX_BUNDLE_GUI_IDENTIFIER "${PROJECT_NAME}" )
	set( MACOSX_BUNDLE_ICON_FILE ${PROJECT_NAME}.icns )
	set( MACOSX_BUNDLE_INFO_STRING "\"${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}\", Copyright 2014 ${PROJECT_NAME} team" )
	set( MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}" )
	set( MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )
	set( MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )
	set( MACOSX_BUNDLE_COPYRIGHT "(C) 2014-2016 Nickolay Ihalainen" )
	set(CMAKE_OSX_DEPLOYMENT_TARGET "10.10")

	add_custom_target( osx_bundle_dirs
		COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Resources
		COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.app/Contents/MacOS
		COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/${MACOSX_BUNDLE_ICON_FILE}
		${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Resources/${MACOSX_BUNDLE_ICON_FILE}
		# Qt translations - uncomment this line when you need to install them to the bundle
		#COMMAND cp *.qm ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Resources/
		)
	add_dependencies( ${PROJECT_NAME} osx_bundle_dirs )
	# This tells cmake where to place files inside the bundle
	set_source_files_properties( ${${PROJECT_NAME}_RESOURCES} ${${PROJECT_NAME}_TRANSLATIONS}
		PROPERTIES MACOSX_PACKAGE_LOCATION Resources )
endif( APPLE )

if (WIN32)
	get_target_property(QT5_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
	get_filename_component(QT5_WINDEPLOYQT_EXECUTABLE ${QT5_QMAKE_EXECUTABLE} PATH)
	set(QT5_WINDEPLOYQT_EXECUTABLE "${QT5_WINDEPLOYQT_EXECUTABLE}/windeployqt.exe")

        set(DEPLOY_TYPE --release)
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(DEPLOY_TYPE --debug)
        endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${QT5_WINDEPLOYQT_EXECUTABLE} ${DEPLOY_TYPE} --qmldir ${CMAKE_SOURCE_DIR} $<TARGET_FILE_DIR:mysqlpat>)
endif(WIN32)



# Use the Widgets module from Qt 5.
target_link_libraries(${PROJECT_NAME} Qt5::Widgets ${QWT_LIBRARIES})
