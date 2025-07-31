#include "HandlerRegistry.h"

#include <QReadWriteLock>

#include "private/SqlQueryHandlerFactory.h"

QForge::nsModel::QueryHandler current_handler; //!< Текущий хендлер.

static QReadWriteLock& lock()
{
    static QReadWriteLock m_lock;
    return m_lock;
}

void QForge::nsModel::HandlerRegistry::setDefaultHandler(const QueryHandler &handler)
{
    current_handler = handler;
}

void QForge::nsModel::HandlerRegistry::setDatabase(QSqlDatabase* db)
{
    current_handler = SqlQueryHandlerFactory::getHandler(db);
}

const QForge::nsModel::QueryHandler& QForge::nsModel::HandlerRegistry::defaultHandler()
{
    QReadLocker locker(&lock());
    return current_handler;
}
