#ifndef QFORGE_HANDLERREGISTRY_HPP
#define QFORGE_HANDLERREGISTRY_HPP

#include "QueryHandler.hpp"

class QSqlDatabase;

namespace QForge {

namespace nsModel {

/**
 * @brief Глобальный регистр для хранения хендлера и (опционально) связанной с ним БД
 */
class HandlerRegistry
{
public:
    static void setDatabase(QSqlDatabase *db);

    static void setDefaultHandler(const QueryHandler& handler);

    static const QueryHandler& defaultHandler();
};

}

} // namespace QForge

#endif // QFORGE_HANDLERREGISTRY_HPP
