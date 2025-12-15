QT += core gui widgets

CONFIG += c++17
TEMPLATE = app
TARGET = UAVSimulator

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    terrainmodel.cpp \
    uavmodel.cpp \
    uavtrajectory.cpp \
    uavvisualitem.cpp \
    uavzvisualizer.cpp

HEADERS += \
    mainwindow.h \
    state.h \
    terrainmodel.h \
    uavmodel.h \
    uavtrajectory.h \
    uavvisualitem.h \
    uavzvisualizer.h

FORMS += \
    mainwindow.ui
