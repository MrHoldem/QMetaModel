#include "TableModel.h"
#include "private/TableModelPrivate.h"
#include <QRegularExpression>
#include "private/ModelSchema.h"
#include <QDebug>

namespace QForge {
namespace nsModel {

TableModel::TableModel(const QString& config_path, QObject* parent)
    : QAbstractTableModel(parent)
    , d_ptr(new TableModelPrivate(this))
{
    Q_D(TableModel);
    d->loadSchema(config_path);
}

TableModel::TableModel(const QString& config_path, const QueryHandler& queryHandler, QObject* parent)
    : QAbstractTableModel(parent)
    , d_ptr(new TableModelPrivate(this))
{
    Q_D(TableModel);
    d->queryHandler = queryHandler;
    d->loadSchema(config_path);
}

TableModel::TableModel(const QString& config_path, QSqlDatabase* db, QObject* parent)
    : QAbstractTableModel(parent)
    , d_ptr(new TableModelPrivate(this))
{
    Q_D(TableModel);
    d->database = db;
    d->loadSchema(config_path);
}

QueryResult TableModel::execute(const QString& query, const QVariantMap& params)
{
    Q_D(TableModel);
    
    onExecutionStarted(query, params);
    
    QueryResult result = d->executeQuery(query, params);
    
    if (result.ok) {
        onExecutionFinished(query, result);
    } else {
        onExecutionError(query, result.errors_log.join("; "));
    }
    
    return result;
}

QUuid TableModel::executeAsync(const QString& query, const QVariantMap& params)
{
    Q_D(TableModel);
    
    QUuid operationId = d->executeQueryAsync(query, params);
    
    onExecutionStarted(query, params);
    emit executionStarted(operationId);
    
    return operationId;
}

QueryResult TableModel::getQueryResult(const QUuid& queryId) const
{
    Q_D(const TableModel);
    
    if (d->activeOperations.contains(queryId)) {
        const AsyncOperation* operation = d->activeOperations[queryId];
        if (operation->future.isFinished()) {
            return operation->future.result();
        }
    }
    
    QueryResult result;
    result.ok = false;
    result.log("Query not found or not finished");
    return result;
}

bool TableModel::isValid() const
{
    Q_D(const TableModel);
    return d->isInitialized && d->schema != nullptr;
}

void TableModel::setQueryHandler(const QueryHandler& queryHandler)
{
    Q_D(TableModel);
    d->queryHandler = queryHandler;
}

void TableModel::setSqlDataBase(QSqlDatabase* db)
{
    Q_D(TableModel);
    d->database = db;
}

const QueryHandler& TableModel::getQueryHandler() const
{
    Q_D(const TableModel);
    return d->queryHandler;
}

QString TableModel::getLastError() const
{
    Q_D(const TableModel);
    return d->lastError;
}

// QAbstractTableModel interface methods

int TableModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    Q_D(const TableModel);
    return d->modelData.size();
}

int TableModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    Q_D(const TableModel);
    
    if (!d->schema) return 0;
    
    return d->horizontalHeaders.size();
}

QVariant TableModel::data(const QModelIndex& index, int role) const
{
    Q_D(const TableModel);
    
    if (!index.isValid() || index.row() >= d->modelData.size()) {
        return QVariant();
    }
    
    const QVariantList& row = d->modelData[index.row()];
    if (index.column() >= row.size()) {
        return QVariant();
    }
    
    const QVariant& value = row[index.column()];
    
    switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return d->processColumnValue(value, index.column());
            
        case Qt::ToolTipRole:
            if (d->schema && index.column() < d->schema->columns.size()) {
                return d->schema->columns[index.column()].tooltip;
            }
            break;
            
        case Qt::TextAlignmentRole:
            if (d->schema && index.column() < d->schema->columns.size()) {
                const ::QForge::Column& column = d->schema->columns[index.column()];
                switch (column.alignment) {
                    case ::QForge::TextAlignment::Left: return Qt::AlignLeft;
                    case ::QForge::TextAlignment::Center: return Qt::AlignCenter;
                    case ::QForge::TextAlignment::Right: return Qt::AlignRight;
                    case ::QForge::TextAlignment::Justify: return Qt::AlignJustify;
                }
            }
            break;
            
        default:
            break;
    }
    
    return QVariant();
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_D(const TableModel);
    
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    
    if (orientation == Qt::Horizontal) {
        if (section >= 0 && section < d->horizontalHeaders.size()) {
            return d->horizontalHeaders[section];
        }
    } else {
        // 5@B8:0;L=K5 703>;>2:8
        if (d->schema) {
            switch (d->schema->verticalHeaders.type) {
                case ::QForge::HeaderType::Numeric:
                    return section + d->schema->verticalHeaders.startIndex;
                case ::QForge::HeaderType::Alphabetic:
                    // @>AB0O @50;870F8O A, B, C...
                    if (section < 26) {
                        return QChar('A' + section);
                    }
                    break;
                case ::QForge::HeaderType::Custom:
                    if (section < d->schema->verticalHeaders.customLabels.size()) {
                        return d->schema->verticalHeaders.customLabels[section];
                    }
                    break;
            }
        }
        return section + 1; // > C<>;G0=8N =C<5@0F8O A 1
    }
    
    return QVariant();
}

bool TableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    Q_D(TableModel);
    
    if (!index.isValid() || role != Qt::EditRole) {
        return false;
    }
    
    if (index.row() >= d->modelData.size() || index.column() >= d->modelData[index.row()].size()) {
        return false;
    }
    
    // @>25@O5<, @07@5H5=> ;8 @540:B8@>20=85 MB>9 :>;>=:8
    if (d->schema && index.column() < d->schema->columns.size()) {
        const QForge::Column& column = d->schema->columns[index.column()];
        
        if (!column.isEditable) {
            return false;
        }
        
        // Проверяем валидацию
        QString validationError = validateValue(value, column);
        if (!validationError.isEmpty()) {
            // В реальном приложении здесь можно показать сообщение об ошибке
            qWarning() << "Validation failed for column" << column.name << ":" << validationError;
            return false;
        }
    }
    
    d->modelData[index.row()][index.column()] = value;
    emit dataChanged(index, index, {role});
    
    return true;
}

Qt::ItemFlags TableModel::flags(const QModelIndex& index) const
{
    Q_D(const TableModel);
    
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    
    // Проверяем, редактируема ли колонка
    if (d->schema && index.column() < d->schema->columns.size()) {
        const QForge::Column& column = d->schema->columns[index.column()];
        if (column.isEditable && d->schema->isEditable) {
            flags |= Qt::ItemIsEditable;
        }
    }
    
    return flags;
}

QString TableModel::validateValue(const QVariant& value, const QForge::Column& column) const
{
    const QForge::Validator& validator = column.validator;
    
    if (validator.type == QForge::ValidatorType::None) {
        return QString(); // Нет валидации
    }
    
    switch (validator.type) {
        case QForge::ValidatorType::Range: {
            bool ok;
            double numValue = value.toDouble(&ok);
            if (!ok) {
                return QString("Значение должно быть числом");
            }
            
            double minVal = validator.range.min.toDouble();
            double maxVal = validator.range.max.toDouble();
            
            if (numValue < minVal || numValue > maxVal) {
                return QString("Значение должно быть от %1 до %2")
                    .arg(minVal)
                    .arg(maxVal);
            }
            break;
        }
        
        case QForge::ValidatorType::Length: {
            QString strValue = value.toString();
            if (strValue.length() < validator.length.minLength || strValue.length() > validator.length.maxLength) {
                return QString("Длина должна быть от %1 до %2 символов")
                    .arg(validator.length.minLength)
                    .arg(validator.length.maxLength);
            }
            break;
        }
        
        case QForge::ValidatorType::Regexp: {
            QString strValue = value.toString();
            QRegularExpression regex(validator.pattern);
            if (!regex.match(strValue).hasMatch()) {
                return QString("Значение не соответствует требуемому формату");
            }
            break;
        }
        
        case QForge::ValidatorType::Required: {
            if (value.isNull() || value.toString().isEmpty()) {
                return QString("Поле обязательно для заполнения");
            }
            break;
        }
        
        default:
            break;
    }
    
    return QString(); // Валидация прошла успешно
}

// Protected virtual methods for customization

void TableModel::onExecutionStarted(const QString& queryName, const QVariantMap& params)
{
    Q_UNUSED(queryName)
    Q_UNUSED(params)
    // 07>20O @50;870F8O - =8G53> =5 45;05<
    // 0A;54=8:8 <>3CB ?5@5>?@545;8BL 4;O ;>38@>20=8O 8 B.4.
}

void TableModel::onExecutionFinished(const QString& queryName, const QueryResult& result)
{
    Q_UNUSED(queryName)
    Q_UNUSED(result)
    // 07>20O @50;870F8O - =8G53> =5 45;05<
}

void TableModel::onExecutionError(const QString& queryName, const QString& error)
{
    Q_UNUSED(queryName)
    
    Q_D(const TableModel);
    
    if (!d->schema) return;
    
    // 1@010BK205< >H81:C A>3;0A=> =0AB@>9:0< AE5<K
    switch (d->schema->defaultErrorHandling.onError) {
        case ::QForge::ErrorHandling::ShowMessage:
            qWarning() << "Query error:" << d->formatErrorMessage(error);
            break;
            
        case ::QForge::ErrorHandling::Log:
            qDebug() << "Query error:" << error;
            break;
            
        case ::QForge::ErrorHandling::Ignore:
            // 8G53> =5 45;05<
            break;
    }
}

const ::QForge::ModelSchema& TableModel::getSchema() const
{
    Q_D(const TableModel);
    if (!d->schema) {
        static ::QForge::ModelSchema* emptySchema = new ::QForge::ModelSchema();
        return *emptySchema;
    }
    
    return *d->schema;
}

} // namespace nsModel
} // namespace QForge