# Sources and headers for QMetaModel

HEADERS += \
    $$PWD/src/EngineRegistry.h \
    $$PWD/src/TableModel.h \
    $$PWD/src/private/ModelCore.h \
    $$PWD/src/private/TableModelPrivate.h

SOURCES += \
    $$PWD/src/EngineRegistry.cpp \
    $$PWD/src/TableModel.cpp \
    $$PWD/src/private/ModelCore.cpp \
    $$PWD/src/private/TableModelPrivate.cpp

INCLUDEPATH += $$PWD/src $$PWD/src/private
