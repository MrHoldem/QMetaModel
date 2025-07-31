#include "ModelSchema.h"

namespace QForge {

// ========== UTILITY METHODS ==========

Column* ModelSchema::findColumn(const QString& columnName) {
    for (auto& column : columns) {
        if (column.name == columnName) {
            return &column;
        }
    }
    return nullptr;
}

const Column* ModelSchema::findColumn(const QString& columnName) const {
    for (const auto& column : columns) {
        if (column.name == columnName) {
            return &column;
        }
    }
    return nullptr;
}

Query* ModelSchema::findQuery(const QString& queryName) {
    auto it = queries.find(queryName);
    return (it != queries.end()) ? &it.value() : nullptr;
}

const Query* ModelSchema::findQuery(const QString& queryName) const {
    auto it = queries.find(queryName);
    return (it != queries.end()) ? &it.value() : nullptr;
}

bool ModelSchema::isValid() const {
    return validate().isEmpty();
}

QStringList ModelSchema::validate() const {
    QStringList errors;
    
    // Check basic required fields
    if (name.isEmpty()) {
        errors << "Model name is required";
    }
    
    if (columns.isEmpty()) {
        errors << "At least one column must be defined";
    }
    
    // Check primary key consistency
    if (!primaryKeyColumns.isEmpty()) {
        for (const QString& pkColumn : primaryKeyColumns) {
            const Column* column = findColumn(pkColumn);
            if (!column) {
                errors << QString("Primary key column '%1' not found in columns").arg(pkColumn);
            } else if (!column->isPrimaryKey) {
                errors << QString("Column '%1' is listed as primary key but isPrimaryKey is false").arg(pkColumn);
            }
        }
    }
    
    // Check if any columns are marked as primary key but not in primaryKeyColumns list
    for (const auto& column : columns) {
        if (column.isPrimaryKey && !primaryKeyColumns.contains(column.name)) {
            errors << QString("Column '%1' is marked as primary key but not in primaryKeyColumns list").arg(column.name);
        }
    }
    
    // Check horizontal headers count matches visible columns
    QStringList visibleColumns = getVisibleColumns();
    if (!horizontalHeaders.isEmpty() && horizontalHeaders.size() != visibleColumns.size()) {
        errors << QString("Horizontal headers count (%1) doesn't match visible columns count (%2)")
                  .arg(horizontalHeaders.size())
                  .arg(visibleColumns.size());
    }
    
    // Check loadQuery exists
    if (source == DataSource::Query) {
        if (loadQuery.isEmpty()) {
            errors << "Load query is required when data source is Query";
        } else if (!queries.contains(loadQuery)) {
            errors << QString("Load query '%1' not found in queries").arg(loadQuery);
        }
    }
    
    // Check sorting column names exist
    for (const auto& sortRule : sorting) {
        if (!findColumn(sortRule.columnName)) {
            errors << QString("Sort column '%1' not found in columns").arg(sortRule.columnName);
        }
    }
    
    // Check query arguments validity
    for (auto it = queries.begin(); it != queries.end(); ++it) {
        const QString& queryName = it.key();
        const Query& query = it.value();
        
        if (query.sql.isEmpty()) {
            errors << QString("Query '%1' has empty SQL").arg(queryName);
        }
        
        // Check argument names are unique within query
        QStringList argNames;
        for (const auto& arg : query.arguments) {
            if (arg.name.isEmpty()) {
                errors << QString("Query '%1' has argument with empty name").arg(queryName);
            } else if (argNames.contains(arg.name)) {
                errors << QString("Query '%1' has duplicate argument name '%2'").arg(queryName, arg.name);
            } else {
                argNames << arg.name;
            }
        }
    }
    
    // Check column names are unique
    QStringList columnNames;
    for (const auto& column : columns) {
        if (column.name.isEmpty()) {
            errors << "Column with empty name found";
        } else if (columnNames.contains(column.name)) {
            errors << QString("Duplicate column name '%1'").arg(column.name);
        } else {
            columnNames << column.name;
        }
    }
    
    // Check reference integrity for foreign key columns
    for (const auto& column : columns) {
        if (!column.referenceTable.isEmpty()) {
            if (column.referenceColumn.isEmpty()) {
                errors << QString("Column '%1' has reference table but no reference column").arg(column.name);
            }
        }
    }
    
    // Check calculated field dependencies
    for (const auto& column : columns) {
        if (column.isCalculated) {
            if (column.calculationExpression.isEmpty()) {
                errors << QString("Calculated column '%1' has empty calculation expression").arg(column.name);
            }
            
            for (const QString& depColumn : column.dependentColumns) {
                if (!findColumn(depColumn)) {
                    errors << QString("Calculated column '%1' depends on non-existent column '%2'").arg(column.name, depColumn);
                }
            }
        }
    }
    
    return errors;
}

void ModelSchema::addColumn(const Column& column) {
    // Check if column with same name already exists
    if (findColumn(column.name)) {
        return; // Don't add duplicate
    }
    
    columns.append(column);
    
    // Update primary key list if needed
    if (column.isPrimaryKey && !primaryKeyColumns.contains(column.name)) {
        primaryKeyColumns.append(column.name);
    }
    
    // Update horizontal headers if needed
    if (column.isVisible && !column.displayName.isEmpty()) {
        horizontalHeaders.append(column.displayName);
    } else if (column.isVisible) {
        horizontalHeaders.append(column.name);
    }
    
    modifiedAt = QDateTime::currentDateTime();
}

void ModelSchema::removeColumn(const QString& columnName) {
    // Remove from columns list
    for (int i = 0; i < columns.size(); ++i) {
        if (columns[i].name == columnName) {
            columns.removeAt(i);
            break;
        }
    }
    
    // Remove from primary key list
    primaryKeyColumns.removeAll(columnName);
    
    // Update horizontal headers
    // This is simplified - in real implementation you'd need to track header positions
    for (int i = 0; i < horizontalHeaders.size(); ++i) {
        // This is a simplified check - you might want to store column-to-header mapping
        if (horizontalHeaders[i] == columnName) {
            horizontalHeaders.removeAt(i);
            break;
        }
    }
    
    // Remove from sorting rules
    for (int i = sorting.size() - 1; i >= 0; --i) {
        if (sorting[i].columnName == columnName) {
            sorting.removeAt(i);
        }
    }
    
    modifiedAt = QDateTime::currentDateTime();
}

void ModelSchema::addQuery(const QString& name, const Query& query) {
    queries[name] = query;
    modifiedAt = QDateTime::currentDateTime();
}

void ModelSchema::removeQuery(const QString& name) {
    queries.remove(name);
    
    // Update loadQuery if it was pointing to removed query
    if (loadQuery == name) {
        if (!queries.isEmpty()) {
            loadQuery = queries.keys().first();
        } else {
            loadQuery.clear();
        }
    }
    
    modifiedAt = QDateTime::currentDateTime();
}

QStringList ModelSchema::getVisibleColumns() const {
    QStringList visibleColumns;
    for (const auto& column : columns) {
        if (column.isVisible) {
            visibleColumns.append(column.name);
        }
    }
    return visibleColumns;
}

QStringList ModelSchema::getEditableColumns() const {
    QStringList editableColumns;
    for (const auto& column : columns) {
        if (column.isEditable && column.isVisible) {
            editableColumns.append(column.name);
        }
    }
    return editableColumns;
}

QStringList ModelSchema::getPrimaryKeyColumns() const {
    return primaryKeyColumns;
}

} // namespace QForge