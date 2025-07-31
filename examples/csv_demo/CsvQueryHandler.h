#pragma once

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDateTime>
#include "QueryHandler.hpp"
#include "QueryResult.hpp"
#include "QueryContext.hpp"

namespace QForge {
namespace nsModel {

class CsvQueryHandler
{
public:
    explicit CsvQueryHandler(const QString& csvFilePath);
    
    // Главный обработчик запросов
    QueryResult operator()(const QueryContext& context);
    
private:
    QString csvPath;
    QList<QVariantMap> csvData;
    QStringList headers;
    
    // Загрузка данных из CSV
    bool loadCsvData();
    
    // Парсинг значения в зависимости от типа
    QVariant parseValue(const QString& value, const QString& header);
    
    // Применение фильтра
    QList<QVariantMap> applyFilter(const QString& filterExpression);
};

} // namespace nsModel
} // namespace QForge