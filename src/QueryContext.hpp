#ifndef QUERYCONTEXT_HPP
#define QUERYCONTEXT_HPP

#include <QString>
#include <QVariantMap>

namespace QForge
{

namespace nsModel {

/**
 * @brief Контекст запроса, передаваемый в пользовательский обработчик
 */
struct QueryContext
{
    QString queryName; //!< Название запроса.
    QString sql; //!< Содержание запроса (вместе со всеми :placeholders).
    QVariantMap bindings; //!< Аргументы.
};

}

}
#endif // QUERYCONTEXT_HPP
