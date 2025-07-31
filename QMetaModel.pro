QT += core

TEMPLATE = lib
CONFIG += c++17
TARGET = QMetaModel

include(QMetaModel.pri)

SUBDIRS += external/yaml-cpp \
           src

# yaml-cpp integration
INCLUDEPATH += $$PWD/external/yaml-cpp/include
LIBS += -L$$PWD/external/yaml-cpp/build -lyaml-cpp
