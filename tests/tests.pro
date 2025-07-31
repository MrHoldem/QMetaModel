QT += core testlib

CONFIG += console c++17
TEMPLATE = app
TARGET = QMetaModelTests

include(tests.pri)

# Путь к заголовкам библиотеки
INCLUDEPATH += $$PWD/../src

# yaml-cpp integration (копируем из src.pro)
DEFINES += YAML_CPP_STATIC_DEFINE
INCLUDEPATH += $$PWD/../external/yaml-cpp/include

# Platform-specific yaml-cpp library paths
win32 {
    # Windows (MinGW) - use dedicated build directory
    LIBS += $$PWD/../build/external/yaml-cpp-win/libyaml-cpp.a
    # Разрешаем множественные определения символов для MinGW
    QMAKE_LFLAGS += -Wl,--allow-multiple-definition
} else {
    # Linux/Unix - use dedicated build directory
    LIBS += $$PWD/../build/external/yaml-cpp/libyaml-cpp.a
}

# Путь к собранной библиотеке 
unix {
    # Библиотека собирается в src/
    LIBS += $$PWD/../src/libQMetaModel.so
} else {
    # Windows - используем статическую библиотеку из основной директории сборки
    LIBS += $$PWD/../libQMetaModel.a
}
