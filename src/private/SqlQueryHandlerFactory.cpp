#include "SqlQueryHandlerFactory.h"

#include "QueryContext.hpp"
#include "QueryResult.hpp"

QForge::nsModel::QueryHandler QForge::nsModel::SqlQueryHandlerFactory::getHandler(QSqlDatabase *db)
{
    return [db](const QueryContext& ctx) -> QueryResult {
        QueryResult result;

        if (!db || !db->isValid() || !db->isOpen()) {
            result.ok = false;
            result.log("Ошибка подключения к базе данных: указатель невалиден или база не открыта.");
            return result;
        }

        QSqlQuery query(*db);
        if (!query.prepare(ctx.sql)) {
            result.ok = false;
            result.log("Ошибка при подготовке SQL-запроса: " + query.lastError().text());
            return result;
        }

        for (auto it = ctx.bindings.constBegin(); it != ctx.bindings.constEnd(); ++it) {
            query.bindValue(it.key(), it.value());
        }

        if (!query.exec()) {
            result.ok = false;
            result.log("Ошибка при выполнении SQL-запроса: " + query.lastError().text());
            return result;
        }

        while (query.next()) {
            QVariantMap row;
            const QSqlRecord rec = query.record();
            for (int i = 0; i < rec.count(); ++i) {
                row.insert(rec.fieldName(i), query.value(i));
            }
            result.rows.append(row);
        }

        result.ok = true;
        return result;
    };
}
