#-------------------------------------------------
#
# Project created by QtCreator 2014-02-01T21:40:31
#
#-------------------------------------------------
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mysqlpat
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp\
	curvetracker.cpp\
	plot.cpp \
    graphschoser.cpp

HEADERS  += mainwindow.h\
	curvetracker.h\
	plot.h \
    graphschoser.h

FORMS    += mainwindow.ui \
    graphschoser.ui


unix {
	include (/usr/local/qwt-6.1.1-svn/features/qwt.prf)
}

win32 {
	include (C:/Qwt-6.1.0/features/qwt.prf)
}

