QT += core testlib

CONFIG += console c++17
TEMPLATE = app
TARGET = QMetaModelTests

include(tests.pri)

# Путь к заголовкам библиотеки
INCLUDEPATH += $$PWD/../src

# Путь к собранной библиотеке
contains(CONFIG, debug, debug|release) {
    LIBS += -L$$OUT_PWD/../src/debug -lQMetaModel
} else {
    LIBS += -L$$OUT_PWD/../src/release -lQMetaModel
}
