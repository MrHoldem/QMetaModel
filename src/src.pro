QT += core gui sql

TEMPLATE = lib
CONFIG += c++17
TARGET = QMetaModel

include(src.pri)

# yaml-cpp integration
DEFINES += YAML_CPP_STATIC_DEFINE
INCLUDEPATH += $$PWD/../external/yaml-cpp/include

# Platform-specific yaml-cpp library paths
win32 {
    # Windows (MinGW) - use dedicated build directory
    LIBS += $$PWD/../build/external/yaml-cpp-win/libyaml-cpp.a
} else {
    # Linux/Unix - use dedicated build directory
    LIBS += $$PWD/../build/external/yaml-cpp/libyaml-cpp.a
}
