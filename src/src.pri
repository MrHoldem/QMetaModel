# Sources and headers for QMetaModel

HEADERS += \
    $$PWD/HandlerRegistry.h \
    $$PWD/QueryContext.hpp \
    $$PWD/QueryHandler.hpp \
    $$PWD/QueryResult.hpp \
    $$PWD/TableModel.h \
    $$PWD/private/ModelCore.h \
    $$PWD/private/ModelSchema.h \
    $$PWD/private/SqlQueryHandlerFactory.h \
    $$PWD/private/TableModelPrivate.h

SOURCES += \
    $$PWD/HandlerRegistry.cpp \
    $$PWD/TableModel.cpp \
    $$PWD/private/ModelCore.cpp \
    $$PWD/private/ModelSchema.cpp \
    $$PWD/private/SqlQueryHandlerFactory.cpp \
    $$PWD/private/TableModelPrivate.cpp

INCLUDEPATH += $$PWD/private
