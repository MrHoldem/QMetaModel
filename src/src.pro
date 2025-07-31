QT += core sql

TEMPLATE = lib
CONFIG += c++17
TARGET = QMetaModel

include(src.pri)

# yaml-cpp integration
INCLUDEPATH += $$PWD/../external/yaml-cpp/include
LIBS += $$PWD/../external/yaml-cpp/build/libyaml-cpp.a
