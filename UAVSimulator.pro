QT += core gui widgets

CONFIG += c++17
TEMPLATE = app
TARGET = UAVSimulator

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    manualcontroller.cpp \
    terrainmodel.cpp \
    uavmodel.cpp \
    uavtrajectory.cpp \
    uavvisualitem.cpp \
    uavzvisualizer.cpp \
    waypointrenderer.cpp

HEADERS += \
    mainwindow.h \
    manualcontroller.h \
    terrainmodel.h \
    uavmodel.h \
    uavtrajectory.h \
    uavvisualitem.h \
    uavzvisualizer.h \
    waypointrenderer.h

FORMS += \
    mainwindow.ui
