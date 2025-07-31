#pragma once

#include <QAbstractTableModel>
#include <QUuid>
#include <QVariantMap>

#include "QueryHandler.hpp"
#include "QueryResult.hpp"

namespace QForge {

class ModelSchema; // В namespace QForge
struct Column;     // Forward declaration для Column

namespace nsModel {

class TableModelPrivate;
class QSqlDatabase;

class TableModel : public QAbstractTableModel {
    Q_OBJECT
    Q_DISABLE_COPY(TableModel)
    Q_DECLARE_PRIVATE(TableModel)
    TableModelPrivate* const d_ptr;

public:
    explicit TableModel(const QString& config_path, QObject* parent = nullptr);
    explicit TableModel(const QString& config_path, const QueryHandler& queryHandler = {}, QObject* parent = nullptr);
    explicit TableModel(const QString& config_path, QSqlDatabase* db, QObject* parent = nullptr);

    QueryResult execute(const QString& query, const QVariantMap& params = {});

    QUuid executeAsync(const QString& query, const QVariantMap& params = {});

    QueryResult getQueryResult(const QUuid& queryId) const;

    bool isValid() const;

    void setQueryHandler(const QueryHandler& queryHandler);

    void setSqlDataBase(QSqlDatabase* db);

    const QueryHandler& getQueryHandler() const;

    QString getLastError() const;

    //... QAbstractTableModel interface methods
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

signals:
    void executionStarted(const QUuid& queryId);
    void executionFinished(const QUuid& queryId);
    void executionFailed(const QUuid& queryId, const QString& error);

protected:
    virtual void onExecutionStarted(const QString& queryName, const QVariantMap& params);
    virtual void onExecutionFinished(const QString& queryName, const QueryResult& result);
    virtual void onExecutionError(const QString& queryName, const QString& error);
    
    // Валидация данных
    QString validateValue(const QVariant& value, const QForge::Column& column) const;
    
    const ::QForge::ModelSchema& getSchema() const;
};

}

}