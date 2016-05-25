QT += core
QT += network
QT += xml
QT -= gui

CONFIG += c++11

TARGET = MeteoServer
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    meteoserver.cpp

HEADERS += \
    meteoserver.h
