#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QUuid>
#include <QHash>
#include <QFuture>
#include <QFutureWatcher>
#include <QTimer>
#include <QVariantMap>
#include <QStringList>

#include "ModelCore.h"
#include "ModelSchema.h"
#include "../QueryHandler.hpp"
#include "../QueryResult.hpp"
#include "../QueryContext.hpp"

namespace QForge {
namespace nsModel {

class TableModel;

struct AsyncOperation {
    QUuid id;
    QString queryName;
    QVariantMap params;
    QFuture<QueryResult> future;
    QFutureWatcher<QueryResult>* watcher;
    
    AsyncOperation() : watcher(nullptr) {}
    ~AsyncOperation() { 
        if (watcher) {
            watcher->deleteLater();
        }
    }
};

class TableModelPrivate : public QObject
{
    Q_OBJECT
public:
    explicit TableModelPrivate(TableModel* q);
    ~TableModelPrivate();

    // =8F80;870F8O
    bool loadSchema(const QString& configPath);
    bool validateSchema();
    
    // K?>;=5=85 70?@>A>2
    QueryResult executeQuery(const QString& queryName, const QVariantMap& params);
    QueryResult executeSqlQuery(const QueryContext& context);
    QUuid executeQueryAsync(const QString& queryName, const QVariantMap& params);
    
    // #?@02;5=85 40==K<8
    void updateModelData(const QueryResult& result);
    void clearData();
    
    // #B8;8BK
    QString formatErrorMessage(const QString& error) const;
    QVariant processColumnValue(const QVariant& value, int columnIndex) const;

private slots:
    void onAsyncQueryFinished();

public:
    TableModel* const q_ptr;
    Q_DECLARE_PUBLIC(TableModel)
    
    // A=>2=K5 :><?>=5=BK
    ModelCore* modelCore;
    ModelSchema* schema;
    
    // AB>G=8:8 40==KE
    QueryHandler queryHandler;
    QSqlDatabase* database;
    
    // !>AB>O=85
    bool isInitialized;
    QString lastError;
    
    // A8=E@>==K5 >?5@0F88
    QHash<QUuid, AsyncOperation*> activeOperations;
    
    // 0==K5 <>45;8 (107>20O @50;870F8O)
    QList<QVariantList> modelData;
    QStringList horizontalHeaders;
};

} // namespace nsModel
} // namespace QForge