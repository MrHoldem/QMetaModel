#ifndef QUERYHANDLER_H
#define QUERYHANDLER_H

#include <functional>
#include "QueryContext.hpp"
#include "QueryResult.hpp"

namespace QForge {

namespace nsModel {

/**
 * @brief Тип пользовательского обработчика запросов
 */
using QueryHandler = std::function<QueryResult(const QueryContext&)>;

} // namespace QForge

}

#endif // QUERYHANDLER_H
