#include "TableModelPrivate.h"
#include "../TableModel.h"
#include <QDebug>
#include <QtConcurrent>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDateTime>

namespace QForge {
namespace nsModel {

TableModelPrivate::TableModelPrivate(TableModel* q)
    : QObject(nullptr)
    , q_ptr(q)
    , modelCore(nullptr)
    , schema(nullptr)
    , database(nullptr)
    , isInitialized(false)
{
    // modelCore будет создан в loadSchema
}

TableModelPrivate::~TableModelPrivate()
{
    clearData();
    
    // G8I05< 0:B82=K5 >?5@0F88
    for (auto it = activeOperations.begin(); it != activeOperations.end(); ++it) {
        delete it.value();
    }
    activeOperations.clear();
    
    delete modelCore;
    // schema C40;O5BAO 2 ModelCore
}

bool TableModelPrivate::loadSchema(const QString& configPath)
{
    // Создаем ModelCore с путем к конфигу
    if (!modelCore) {
        modelCore = new ModelCore(configPath, queryHandler);
    }
    
    if (!modelCore->isValid()) {
        QStringList errors = modelCore->getErrors();
        lastError = "Failed to load schema from " + configPath + ". Errors: " + errors.join("; ");
        qDebug() << "ModelCore errors:" << errors;
        return false;
    }
    
    // Получаем схему из ModelCore
    schema = const_cast<QForge::ModelSchema*>(&modelCore->getSchema());
    
    // =8F80;878@C5< 703>;>2:8 87 AE5<K
    horizontalHeaders.clear();
    for (const auto& header : schema->horizontalHeaders) {
        horizontalHeaders.append(header);
    }
    
    // A;8 703>;>2:8 =5 7040=K, 8A?>;L7C5< 8<5=0 :>;>=>:
    if (horizontalHeaders.isEmpty()) {
        for (const auto& column : schema->columns) {
            horizontalHeaders.append(column.name);
        }
    }
    
    isInitialized = validateSchema();
    return isInitialized;
}

bool TableModelPrivate::validateSchema()
{
    if (!schema) {
        lastError = "Schema not loaded";
        return false;
    }
    
    // 07>2K5 ?@>25@:8
    if (schema->name.isEmpty()) {
        lastError = "Schema name is empty";
        return false;
    }
    
    if (schema->columns.isEmpty()) {
        lastError = "No columns defined in schema";
        return false;
    }
    
    // @>25@O5<, GB> 5ABL E>BO 1K >48= 70?@>A
    if (schema->queries.isEmpty()) {
        lastError = "No queries defined in schema";
        return false;
    }
    
    return true;
}

QueryResult TableModelPrivate::executeQuery(const QString& queryName, const QVariantMap& params)
{
    if (!isInitialized) {
        QueryResult result;
        result.ok = false;
        result.log("Model not initialized");
        return result;
    }
    
    if (!schema->queries.contains(queryName)) {
        QueryResult result;
        result.ok = false;
        result.log(QString("Query '%1' not found").arg(queryName));
        return result;
    }
    
    // !>7405< :>=B5:AB 70?@>A0
    QueryContext context;
    context.queryName = queryName;
    context.bindings = params;
    
    const QForge::Query& queryDef = schema->queries[queryName];
    context.sql = queryDef.sql;
    
    QueryResult result;
    
    try {
        if (queryHandler) {
            // A?>;L7C5< ?>;L7>20B5;LA:89 >1@01>BG8:
            result = queryHandler(context);
        } else if (database) {
            // A?>;L7C5< SQL 107C 40==KE
            result = executeSqlQuery(context);
        } else {
            result.ok = false;
            result.log("No query handler or database configured");
        }
        
        if (result.ok) {
            updateModelData(result);
        }
        
    } catch (const std::exception& e) {
        result.ok = false;
        result.log(QString("Query execution failed: %1").arg(e.what()));
    }
    
    if (!result.ok) {
        lastError = result.errors_log.join("; ");
    }
    
    return result;
}

QueryResult TableModelPrivate::executeSqlQuery(const QueryContext& context)
{
    QueryResult result;
    
    if (!database || !database->isOpen()) {
        result.ok = false;
        result.log("Database not connected");
        return result;
    }
    
    const QForge::Query& queryDef = schema->queries[context.queryName];
    QString sql = queryDef.sql;
    
    // >4AB02;O5< ?0@0<5B@K 2 SQL
    for (auto it = context.bindings.begin(); it != context.bindings.end(); ++it) {
        QString placeholder = QString("${%1}").arg(it.key());
        sql.replace(placeholder, it.value().toString());
    }
    
    QSqlQuery query(*database);
    query.prepare(sql);
    
    if (!query.exec()) {
        result.ok = false;
        result.log(query.lastError().text());
        return result;
    }
    
    // !>18@05< @57C;LB0BK
    while (query.next()) {
        QVariantList row;
        for (int i = 0; i < query.record().count(); ++i) {
            row.append(query.value(i));
        }
        // Преобразуем в QVariantMap для QueryResult
        QVariantMap rowMap;
        for (int j = 0; j < row.size() && j < schema->columns.size(); ++j) {
            rowMap[schema->columns[j].name] = row[j];
        }
        result.rows.append(rowMap);
    }
    
    result.ok = true;
    
    return result;
}

QUuid TableModelPrivate::executeQueryAsync(const QString& queryName, const QVariantMap& params)
{
    QUuid operationId = QUuid::createUuid();
    
    AsyncOperation* operation = new AsyncOperation();
    operation->id = operationId;
    operation->queryName = queryName;
    operation->params = params;
    
    // !>7405< Future 4;O 0A8=E@>==>3> 2K?>;=5=8O
    operation->future = QtConcurrent::run([this, queryName, params]() {
        return executeQuery(queryName, params);
    });
    
    // !>7405< Watcher 4;O >BA;56820=8O 7025@H5=8O
    operation->watcher = new QFutureWatcher<QueryResult>();
    QObject::connect(operation->watcher, &QFutureWatcher<QueryResult>::finished,
                     this, &TableModelPrivate::onAsyncQueryFinished);
    
    operation->watcher->setFuture(operation->future);
    
    activeOperations[operationId] = operation;
    
    return operationId;
}

void TableModelPrivate::onAsyncQueryFinished()
{
    Q_Q(TableModel);
    
    QFutureWatcher<QueryResult>* watcher = static_cast<QFutureWatcher<QueryResult>*>(sender());
    if (!watcher) return;
    
    // 0E>48< >?5@0F8N ?> watcher
    QUuid operationId;
    for (auto it = activeOperations.begin(); it != activeOperations.end(); ++it) {
        if (it.value()->watcher == watcher) {
            operationId = it.key();
            break;
        }
    }
    
    if (operationId.isNull()) return;
    
    AsyncOperation* operation = activeOperations.take(operationId);
    QueryResult result = watcher->result();
    
    if (result.ok) {
        emit q->executionFinished(operationId);
        q->onExecutionFinished(operation->queryName, result);
    } else {
        QString errorMsg = result.errors_log.join("; ");
        emit q->executionFailed(operationId, errorMsg);
        q->onExecutionError(operation->queryName, errorMsg);
    }
    
    delete operation;
}

void TableModelPrivate::updateModelData(const QueryResult& result)
{
    Q_Q(TableModel);
    
    q->beginResetModel();
    
    modelData.clear();
    
    // Преобразуем QVariantMap обратно в QVariantList для отображения
    for (const QVariantMap& rowMap : result.rows) {
        QVariantList row;
        for (const QForge::Column& column : schema->columns) {
            row.append(rowMap.value(column.name));
        }
        modelData.append(row);
    }
    
    q->endResetModel();
}

void TableModelPrivate::clearData()
{
    Q_Q(TableModel);
    
    if (!modelData.isEmpty()) {
        q->beginResetModel();
        modelData.clear();
        q->endResetModel();
    }
}

QString TableModelPrivate::formatErrorMessage(const QString& error) const
{
    if (!schema) return error;
    
    QString message = schema->defaultErrorHandling.message;
    message.replace("${last_error}", error);
    
    return message;
}

QVariant TableModelPrivate::processColumnValue(const QVariant& value, int columnIndex) const
{
    if (!schema || columnIndex < 0 || columnIndex >= schema->columns.size()) {
        return value;
    }
    
    const QForge::Column& column = schema->columns[columnIndex];
    
    // @8<5=O5< D>@<0B8@>20=85 2 7028A8<>AB8 >B B8?0 :>;>=:8
    switch (column.type) {
        case QForge::ColumnType::DateTime:
            if (value.canConvert<QDateTime>()) {
                return value.toDateTime().toString(Qt::ISODate);
            }
            break;
            
        case QForge::ColumnType::Boolean:
            if (value.canConvert<bool>()) {
                return value.toBool() ? "Yes" : "No";
            }
            break;
            
        default:
            break;
    }
    
    return value;
}

} // namespace nsModel
} // namespace QForge
