QT += core gui widgets

CONFIG += c++17
TEMPLATE = app
TARGET = UAVSimulator

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    uavmodel.cpp \
    uavtrajectory.cpp \
    uavzvisualizer.cpp

HEADERS += \
    mainwindow.h \
    state.h \
    uavmodel.h \
    uavtrajectory.h \
    uavzvisualizer.h

FORMS += \
    mainwindow.ui
