#include "CsvQueryHandler.h"
#include <QStringList>
#include <QThread>

namespace QForge {
namespace nsModel {

CsvQueryHandler::CsvQueryHandler(const QString& csvFilePath)
    : csvPath(csvFilePath)
{
    qDebug() << "CsvQueryHandler: Loading CSV from" << csvFilePath;
    bool success = loadCsvData();
    qDebug() << "CsvQueryHandler: Load result:" << success;
}

bool CsvQueryHandler::loadCsvData()
{
    QFile file(csvPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open CSV file:" << csvPath;
        return false;
    }
    
    QTextStream stream(&file);
    
    // Читаем заголовки
    if (!stream.atEnd()) {
        QString headerLine = stream.readLine();
        headers = headerLine.split(',');
        
        // Убираем лишние пробелы
        for (auto& header : headers) {
            header = header.trimmed();
        }
    }
    
    // Читаем данные
    csvData.clear();
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        QStringList values = line.split(',');
        
        if (values.size() == headers.size()) {
            QVariantMap row;
            for (int i = 0; i < headers.size(); ++i) {
                row[headers[i]] = parseValue(values[i].trimmed(), headers[i]);
            }
            csvData.append(row);
        }
    }
    
    file.close();
    qDebug() << "Loaded" << csvData.size() << "rows from CSV";
    return true;
}

QVariant CsvQueryHandler::parseValue(const QString& value, const QString& header)
{
    // Простая логика парсинга по имени колонки
    if (header == "id" || header == "quantity") {
        return value.toInt();
    } else if (header == "price") {
        return value.toDouble();
    } else if (header == "in_stock") {
        return value.toLower() == "true" || value == "1";
    } else if (header == "last_updated") {
        return QDateTime::fromString(value, "yyyy-MM-dd hh:mm:ss");
    }
    
    return value; // Строка по умолчанию
}

QueryResult CsvQueryHandler::operator()(const QueryContext& context)
{
    QueryResult result;
    
    qDebug() << "Executing query:" << context.queryName << "SQL:" << context.sql;
    qDebug() << "Query bindings:" << context.bindings;
    
    QString processedSql = context.sql;
    
    // Подставляем параметры в SQL
    for (auto it = context.bindings.begin(); it != context.bindings.end(); ++it) {
        QString placeholder = QString("${%1}").arg(it.key());
        QString value = it.value().toString();
        processedSql.replace(placeholder, value);
    }
    
    qDebug() << "Processed SQL:" << processedSql;
    
    // Обработка специальных команд
    if (processedSql == "LOAD_CSV") {
        // Перезагружаем данные
        if (loadCsvData()) {
            result.rows = csvData;
            result.ok = true;
        } else {
            result.ok = false;
            result.log("Failed to load CSV file");
        }
    } else if (processedSql.startsWith("FILTER ")) {
        // Применяем фильтр
        QThread::sleep(10);
        QString filterExpr = processedSql.mid(7); // Убираем "FILTER "
        result.rows = applyFilter(filterExpr);
        result.ok = true;
    } else {
        // По умолчанию возвращаем все данные
        result.rows = csvData;
        result.ok = true;
    }
    
    qDebug() << "Query returned" << result.rows.size() << "rows";
    return result;
}

QList<QVariantMap> CsvQueryHandler::applyFilter(const QString& filterExpression)
{
    QList<QVariantMap> filtered;
    
    qDebug() << "Applying filter:" << filterExpression;
    
    // Простой парсер фильтров вида "column = value"
    QStringList parts = filterExpression.split('=');
    if (parts.size() != 2) {
        qDebug() << "Cannot parse filter expression, returning all data";
        return csvData; // Если не можем распарсить, возвращаем все
    }
    
    QString column = parts[0].trimmed();
    QString value = parts[1].trimmed();
    
    // Удаляем кавычки если есть
    if (value.startsWith('"') && value.endsWith('"')) {
        value = value.mid(1, value.length() - 2);
    }
    
    qDebug() << "Filter column:" << column << "value:" << value;
    qDebug() << "Total rows to filter:" << csvData.size();
    
    for (const auto& row : csvData) {
        if (row.contains(column)) {
            if (column == "in_stock") {
                // Специальная обработка для boolean
                bool boolValue = row[column].toBool();
                bool targetValue = (value == "true");
                qDebug() << "Boolean filter: row value =" << boolValue << "target =" << targetValue;
                if (boolValue == targetValue) {
                    filtered.append(row);
                }
            } else {
                QString rowValue = row[column].toString();
                qDebug() << "String filter: row value =" << rowValue << "target =" << value;
                if (rowValue == value) {
                    filtered.append(row);
                }
            }
        } else {
            qDebug() << "Row doesn't contain column:" << column;
        }
    }
    
    qDebug() << "Filtered result:" << filtered.size() << "rows";
    return filtered;
}

} // namespace nsModel
} // namespace QForge
