QT += core sql

TEMPLATE = lib
CONFIG += c++17
TARGET = QMetaModel

include(src.pri)

# yaml-cpp integration
DEFINES += YAML_CPP_STATIC_DEFINE
INCLUDEPATH += $$PWD/../external/yaml-cpp/include
LIBS += $$PWD/../external/yaml-cpp/build/libyaml-cpp.a
