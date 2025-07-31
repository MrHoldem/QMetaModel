QT += core gui widgets sql

CONFIG += c++17

TARGET = csv_demo
TEMPLATE = app

# Исходники демо
SOURCES += \
    main.cpp \
    ProductWidget.cpp \
    CsvQueryHandler.cpp

HEADERS += \
    ProductWidget.h \
    CsvQueryHandler.h

# Подключаем библиотеку QMetaModel
INCLUDEPATH += ../../src

# Линковка с QMetaModel - используем правильные пути
win32 {
    CONFIG(debug, debug|release) {
        LIBS += -L$$PWD/../../build/Desktop_Qt_6_8_0_MinGW_64_bit-Debug/src/debug -lQMetaModel
    } else {
        LIBS += -L$$PWD/../../build/Desktop_Qt_6_8_0_MinGW_64_bit-Release/src/release -lQMetaModel
    }
} else {
    LIBS += $$PWD/../../src/libQMetaModel.so
}

# yaml-cpp
DEFINES += YAML_CPP_STATIC_DEFINE
INCLUDEPATH += ../../external/yaml-cpp/include

win32 {
    # Windows (MinGW) - use build directory
    LIBS += $$PWD/../../build/external/yaml-cpp-win/libyaml-cpp.a
    # Additional linker flags for Windows
    QMAKE_LFLAGS += -static-libgcc -static-libstdc++
} else {
    # Linux/Unix - use build_linux directory
    LIBS += $$PWD/../../external/yaml-cpp/build_linux/libyaml-cpp.a
}

# Файлы данных будут загружаться из исходной директории
