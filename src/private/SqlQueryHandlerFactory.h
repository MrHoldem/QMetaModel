#ifndef SQLQUERYHANDLERFACTORY_H
#define SQLQUERYHANDLERFACTORY_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariantMap>
#include <QString>
#include <QDebug>

#include "QueryHandler.hpp"

namespace QForge {

namespace nsModel {

/**
 * @brief Утилитный статический класс, возвращающий QueryHandler,
 * работающий с конкретным экземпляром QSqlDatabase.
 */
class SqlQueryHandlerFactory
{
public:
    /**
     * @brief Создаёт QueryHandler, связанный с данной базой данных.
     * @param db Указатель на открытую базу данных.
     * @return QueryHandler, выполняющий запросы через указанную БД.
     */
    static QueryHandler getHandler(QSqlDatabase* db);
};

}

} // namespace QForge
#endif // SQLQUERYHANDLERFACTORY_H
