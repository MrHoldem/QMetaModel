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

# Путь к собранной библиотеке (статическая линковка для WSL)
unix {
    # В WSL библиотека собирается в build/wsl_makefiles/
    LIBS += $$PWD/../build/wsl_makefiles/libQMetaModel.so
} else {
    # Windows - используем обычную схему
    contains(CONFIG, debug, debug|release) {
        LIBS += -L$$OUT_PWD/../src/debug -lQMetaModel
    } else {
        LIBS += -L$$OUT_PWD/../src/release -lQMetaModel
    }
}
