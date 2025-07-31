#ifndef QUERYRESULT_HPP
#define QUERYRESULT_HPP

#include <QList>
#include <QVariantMap>

namespace QForge {

namespace nsModel {

/**
 * @brief Результат запроса, возвращаемый из пользовательского обработчика
 */
struct QueryResult
{
    bool ok; //!< Флаг успешного завершения.
    QList<QVariantMap> rows; //!< Ответ (строки).
    QStringList errors_log; //!< Лог ошибок.

    inline void log(const QString& error) { errors_log.append(error); }
};

}

} // namespace QForge

#endif // QUERYRESULT_HPP
