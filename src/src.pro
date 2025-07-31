QT += core gui sql concurrent

TEMPLATE = lib
CONFIG += c++17
TARGET = QMetaModel

include(src.pri)

# yaml-cpp integration
DEFINES += YAML_CPP_STATIC_DEFINE
INCLUDEPATH += $$PWD/../external/yaml-cpp/include

# Platform-specific yaml-cpp library paths
win32 {
    # Windows (MinGW) - use build directory
    LIBS += $$PWD/../build/external/yaml-cpp-win/libyaml-cpp.a
    # Additional linker flags for Windows
    QMAKE_LFLAGS += -static-libgcc -static-libstdc++
} else {
    # Linux/Unix - use build_linux directory
    LIBS += $$PWD/../external/yaml-cpp/build_linux/libyaml-cpp.a
}
