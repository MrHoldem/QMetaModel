#include "ModelCore.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>

#include <yaml-cpp/yaml.h>

namespace QForge::nsModel {

// ========== HELPER FUNCTIONS ==========

static ColumnType stringToColumnType(const QString& typeStr) {
    static const QHash<QString, ColumnType> typeMap = {
        {"string", ColumnType::String},
        {"integer", ColumnType::Integer},
        {"int", ColumnType::Integer},
        {"double", ColumnType::Double},
        {"float", ColumnType::Double},
        {"boolean", ColumnType::Boolean},
        {"bool", ColumnType::Boolean},
        {"datetime", ColumnType::DateTime},
        {"date", ColumnType::Date},
        {"time", ColumnType::Time},
        {"uuid", ColumnType::Uuid},
        {"binary", ColumnType::Binary},
        {"json", ColumnType::Json},
        {"array", ColumnType::Array}
    };
    return typeMap.value(typeStr.toLower(), ColumnType::String);
}

static TextAlignment stringToAlignment(const QString& alignStr) {
    static const QHash<QString, TextAlignment> alignMap = {
        {"left", TextAlignment::Left},
        {"center", TextAlignment::Center},
        {"right", TextAlignment::Right},
        {"justify", TextAlignment::Justify}
    };
    return alignMap.value(alignStr.toLower(), TextAlignment::Left);
}

static SortOrder stringToSortOrder(const QString& orderStr) {
    return (orderStr.toLower() == "desc" || orderStr.toLower() == "descending") 
           ? SortOrder::Descending : SortOrder::Ascending;
}

static ErrorHandling stringToErrorHandling(const QString& errorStr) {
    static const QHash<QString, ErrorHandling> errorMap = {
        {"show_message", ErrorHandling::ShowMessage},
        {"ignore", ErrorHandling::Ignore},
        {"log", ErrorHandling::Log},
        {"callback", ErrorHandling::Callback}
    };
    return errorMap.value(errorStr.toLower(), ErrorHandling::ShowMessage);
}

static ValidatorType stringToValidatorType(const QString& validatorStr) {
    static const QHash<QString, ValidatorType> validatorMap = {
        {"regexp", ValidatorType::Regexp},
        {"range", ValidatorType::Range},
        {"length", ValidatorType::Length},
        {"custom", ValidatorType::Custom},
        {"required", ValidatorType::Required}
    };
    return validatorMap.value(validatorStr.toLower(), ValidatorType::None);
}

static ModelType stringToModelType(const QString& typeStr) {
    return (typeStr.toLower() == "tree") ? ModelType::Tree : ModelType::Table;
}

static DataSource stringToDataSource(const QString& sourceStr) {
    return (sourceStr.toLower() == "manual") ? DataSource::Manual : DataSource::Query;
}

// ========== CONSTRUCTORS ==========

ModelCore::ModelCore(const QString& configPath, const QueryHandler& handler)
    : handler(handler)
    , path(configPath)
    , schema(std::make_unique<ModelSchema>())
{
    loadFromFile();
}

ModelCore::ModelCore(const ModelSchema& modelSchema, const QueryHandler& handler)
    : handler(handler)
    , isValidFlag(true)
    , schema(std::make_unique<ModelSchema>(modelSchema))
{
    // Validate the provided schema
    QStringList validationErrors = schema->validate();
    if (!validationErrors.isEmpty()) {
        errors_log = validationErrors;
        isValidFlag = false;
    }
}

// ========== PUBLIC METHODS ==========

bool ModelCore::isValid() const {
    return isValidFlag;
}

QueryResult ModelCore::execute(const QString& queryId, const QVariantMap& params) const {
    if (!isValidFlag) {
        QueryResult result;
        result.ok = false;
        result.log("ModelCore is not in valid state");
        return result;
    }

    // Validate parameters first
    QueryResult validationResult = validateQueryParams(queryId, params);
    if (!validationResult.ok) {
        return validationResult;
    }

    // Find the query
    const Query* query = schema->findQuery(queryId);
    if (!query) {
        QueryResult result;
        result.ok = false;
        result.log(QString("Query '%1' not found").arg(queryId));
        return result;
    }

    // Prepare context
    QueryContext context;
    context.queryName = queryId;
    context.bindings = params;
    context.sql = query->sql;

    // Execute the query
    return handler(context);
}

const ModelSchema& ModelCore::getSchema() const {
    return *schema;
}

QJsonObject ModelCore::getJsonSchema() const {
    QJsonObject schemaJson;
    
    // Basic information
    schemaJson["name"] = schema->name;
    schemaJson["type"] = (schema->type == ModelType::Table) ? "table" : "tree";
    schemaJson["description"] = schema->description;
    schemaJson["source"] = (schema->source == DataSource::Query) ? "query" : "manual";
    schemaJson["is_editable"] = schema->isEditable;
    schemaJson["load_query"] = schema->loadQuery;
    
    // Headers
    QJsonArray horizontalHeaders;
    for (const QString& header : schema->horizontalHeaders) {
        horizontalHeaders.append(header);
    }
    schemaJson["horizontal_headers"] = horizontalHeaders;
    
    // Columns
    QJsonArray columnsArray;
    for (const Column& column : schema->columns) {
        QJsonObject columnObj;
        columnObj["name"] = column.name;
        columnObj["type"] = static_cast<int>(column.type); // You might want to convert to string
        columnObj["is_visible"] = column.isVisible;
        columnObj["is_editable"] = column.isEditable;
        columnObj["is_primary_key"] = column.isPrimaryKey;
        if (!column.tooltip.isEmpty()) {
            columnObj["tooltip"] = column.tooltip;
        }
        columnsArray.append(columnObj);
    }
    schemaJson["columns"] = columnsArray;
    
    // Queries
    QJsonObject queriesObj;
    for (auto it = schema->queries.begin(); it != schema->queries.end(); ++it) {
        QJsonObject queryObj;
        queryObj["sql"] = it.value().sql;
        
        if (!it.value().arguments.isEmpty()) {
            QJsonArray argsArray;
            for (const QueryArgument& arg : it.value().arguments) {
                QJsonObject argObj;
                argObj["name"] = arg.name;
                argObj["type"] = static_cast<int>(arg.type);
                argObj["is_optional"] = arg.isOptional;
                if (!arg.defaultValue.isNull()) {
                    argObj["default"] = QJsonValue::fromVariant(arg.defaultValue);
                }
                argsArray.append(argObj);
            }
            queryObj["arguments"] = argsArray;
        }
        
        queriesObj[it.key()] = queryObj;
    }
    schemaJson["queries"] = queriesObj;
    
    return schemaJson;
}

QStringList ModelCore::getQueriesList() const {
    return schema->queries.keys();
}

bool ModelCore::reload() {
    if (path.isEmpty()) {
        errors_log.append("Cannot reload: no configuration file path specified");
        return false;
    }
    
    ModelCore newCore(path, handler);
    if (!newCore.isValid()) {
        errors_log.append(QString("Failed to reload configuration from '%1'").arg(path));
        errors_log.append(newCore.getErrors());
        return false;
    }

    // Replace current state with new one
    *this = std::move(newCore);
    return true;
}

const QStringList& ModelCore::getErrors() const {
    return errors_log;
}

void ModelCore::clearErrors() {
    errors_log.clear();
}

// ========== PRIVATE METHODS ==========

bool ModelCore::loadFromFile() {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errors_log.append(QString("Failed to open config file: %1").arg(path));
        isValidFlag = false;
        return false;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // Determine file format by extension
    QFileInfo fileInfo(path);
    QString extension = fileInfo.suffix().toLower();
    
    bool parseok = false;
    if (extension == "yml" || extension == "yaml") {
        parseok = parseYaml(content);
    } else if (extension == "json") {
        parseok = parseJson(content);
    } else {
        // Try to auto-detect format
        content = content.trimmed();
        if (content.startsWith('{')) {
            parseok = parseJson(content);
        } else {
            parseok = parseYaml(content);
        }
    }
    
    if (parseok) {
        // Validate parsed schema
        QStringList validationErrors = schema->validate();
        if (!validationErrors.isEmpty()) {
            errors_log.append("Schema validation failed:");
            errors_log.append(validationErrors);
            isValidFlag = false;
            return false;
        }
        
        isValidFlag = true;
        return true;
    }
    
    isValidFlag = false;
    return false;
}

bool ModelCore::parseYaml(const QString& content) {
    try {
        YAML::Node root = YAML::Load(content.toStdString());
        if (!root.IsMap()) {
            errors_log.append("YAML root is not a map/object");
            return false;
        }

        // Parse basic information
        if (root["name"]) {
            schema->name = QString::fromStdString(root["name"].as<std::string>());
        }
        
        if (root["type"]) {
            schema->type = stringToModelType(QString::fromStdString(root["type"].as<std::string>()));
        }
        
        if (root["description"]) {
            schema->description = QString::fromStdString(root["description"].as<std::string>());
        }
        
        if (root["source"]) {
            schema->source = stringToDataSource(QString::fromStdString(root["source"].as<std::string>()));
        }
        
        if (root["is_editable"]) {
            schema->isEditable = root["is_editable"].as<bool>();
        }
        
        if (root["load_query"]) {
            schema->loadQuery = QString::fromStdString(root["load_query"].as<std::string>());
        }

        // Parse horizontal headers
        if (root["horizontal_headers"] && root["horizontal_headers"].IsSequence()) {
            schema->horizontalHeaders.clear();
            for (const auto& header : root["horizontal_headers"]) {
                schema->horizontalHeaders.append(QString::fromStdString(header.as<std::string>()));
            }
        }

        // Parse columns
        if (root["columns"] && root["columns"].IsSequence()) {
            schema->columns.clear();
            for (const auto& colNode : root["columns"]) {
                Column column;
                
                if (colNode["name"]) {
                    column.name = QString::fromStdString(colNode["name"].as<std::string>());
                }
                
                if (colNode["type"]) {
                    column.type = stringToColumnType(QString::fromStdString(colNode["type"].as<std::string>()));
                }
                
                if (colNode["is_visible"]) {
                    column.isVisible = colNode["is_visible"].as<bool>();
                }
                
                if (colNode["is_editable"]) {
                    column.isEditable = colNode["is_editable"].as<bool>();
                }
                
                if (colNode["is_primary_key"]) {
                    column.isPrimaryKey = colNode["is_primary_key"].as<bool>();
                    if (column.isPrimaryKey) {
                        schema->primaryKeyColumns.append(column.name);
                    }
                }
                
                if (colNode["tooltip"]) {
                    column.tooltip = QString::fromStdString(colNode["tooltip"].as<std::string>());
                }
                
                if (colNode["alignment"]) {
                    column.alignment = stringToAlignment(QString::fromStdString(colNode["alignment"].as<std::string>()));
                }
                
                // Parse validator
                if (colNode["validator"]) {
                    const auto& validatorNode = colNode["validator"];
                    if (validatorNode["type"]) {
                        column.validator.type = stringToValidatorType(QString::fromStdString(validatorNode["type"].as<std::string>()));
                    }
                    if (validatorNode["pattern"]) {
                        column.validator.pattern = QString::fromStdString(validatorNode["pattern"].as<std::string>());
                    }
                }
                
                schema->columns.append(column);
            }
        }

        // Parse sorting
        if (root["sorting"] && root["sorting"].IsSequence()) {
            schema->sorting.clear();
            for (const auto& sortNode : root["sorting"]) {
                SortRule sortRule;
                if (sortNode["column"]) {
                    sortRule.columnName = QString::fromStdString(sortNode["column"].as<std::string>());
                }
                if (sortNode["order"]) {
                    sortRule.order = stringToSortOrder(QString::fromStdString(sortNode["order"].as<std::string>()));
                }
                schema->sorting.append(sortRule);
            }
        }

        // Parse queries
        if (root["queries"] && root["queries"].IsMap()) {
            schema->queries.clear();
            for (const auto& queryPair : root["queries"]) {
                QString queryName = QString::fromStdString(queryPair.first.as<std::string>());
                const auto& queryNode = queryPair.second;
                
                Query query;
                if (queryNode["sql"]) {
                    query.sql = QString::fromStdString(queryNode["sql"].as<std::string>());
                }
                
                if (queryNode["on_error"]) {
                    query.onError = stringToErrorHandling(QString::fromStdString(queryNode["on_error"].as<std::string>()));
                }
                
                // Parse arguments
                if (queryNode["arguments"] && queryNode["arguments"].IsSequence()) {
                    for (const auto& argNode : queryNode["arguments"]) {
                        QueryArgument arg;
                        if (argNode["name"]) {
                            arg.name = QString::fromStdString(argNode["name"].as<std::string>());
                        }
                        if (argNode["type"]) {
                            arg.type = stringToColumnType(QString::fromStdString(argNode["type"].as<std::string>()));
                        }
                        if (argNode["is_optional"]) {
                            arg.isOptional = argNode["is_optional"].as<bool>();
                        }
                        if (argNode["default"]) {
                            // Handle different default value types
                            if (argNode["default"].IsScalar()) {
                                arg.defaultValue = QString::fromStdString(argNode["default"].as<std::string>());
                            }
                        }
                        query.arguments.append(arg);
                    }
                }
                
                schema->queries[queryName] = query;
            }
        }

        // Parse error handling
        if (root["default_error_handling"]) {
            const auto& errorNode = root["default_error_handling"];
            if (errorNode["on_error"]) {
                schema->defaultErrorHandling.onError = stringToErrorHandling(
                    QString::fromStdString(errorNode["on_error"].as<std::string>()));
            }
            if (errorNode["message"]) {
                schema->defaultErrorHandling.message = QString::fromStdString(errorNode["message"].as<std::string>());
            }
        }

        // Parse additional settings
        if (root["callback_is_required"]) {
            schema->callbackIsRequired = root["callback_is_required"].as<bool>();
        }
        
        if (root["default_row_tooltip"]) {
            schema->defaultRowTooltip = QString::fromStdString(root["default_row_tooltip"].as<std::string>());
        }
        
        if (root["show_numeration"]) {
            schema->showNumeration = root["show_numeration"].as<bool>();
        }

        return true;

    } catch (const YAML::Exception& e) {
        errors_log.append(QString("YAML parsing error: %1").arg(e.what()));
        return false;
    } catch (const std::exception& e) {
        errors_log.append(QString("Standard exception during YAML parsing: %1").arg(e.what()));
        return false;
    }
}

bool ModelCore::parseJson(const QString& content) {
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(content.toUtf8(), &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        errors_log.append(QString("JSON parsing error: %1").arg(parseError.errorString()));
        return false;
    }
    
    if (!doc.isObject()) {
        errors_log.append("JSON root is not an object");
        return false;
    }
    
    QJsonObject root = doc.object();
    
    // Parse basic information
    if (root.contains("name")) {
        schema->name = root["name"].toString();
    }
    
    if (root.contains("type")) {
        schema->type = stringToModelType(root["type"].toString());
    }
    
    if (root.contains("description")) {
        schema->description = root["description"].toString();
    }
    
    if (root.contains("source")) {
        schema->source = stringToDataSource(root["source"].toString());
    }
    
    if (root.contains("is_editable")) {
        schema->isEditable = root["is_editable"].toBool();
    }
    
    // Parse horizontal headers
    if (root.contains("horizontal_headers") && root["horizontal_headers"].isArray()) {
        schema->horizontalHeaders.clear();
        QJsonArray headersArray = root["horizontal_headers"].toArray();
        for (const QJsonValue& headerValue : headersArray) {
            schema->horizontalHeaders.append(headerValue.toString());
        }
    }

    // Parse columns
    if (root.contains("columns") && root["columns"].isArray()) {
        schema->columns.clear();
        QJsonArray columnsArray = root["columns"].toArray();
        
        for (const QJsonValue& colValue : columnsArray) {
            if (!colValue.isObject()) continue;
            
            QJsonObject colObj = colValue.toObject();
            Column column;
            
            column.name = colObj["name"].toString();
            column.type = stringToColumnType(colObj["type"].toString());
            column.isVisible = colObj.value("is_visible").toBool(true);
            column.isEditable = colObj.value("is_editable").toBool(true);
            column.isPrimaryKey = colObj.value("is_primary_key").toBool(false);
            column.tooltip = colObj.value("tooltip").toString();
            column.alignment = stringToAlignment(colObj.value("alignment").toString());
            
            if (column.isPrimaryKey) {
                schema->primaryKeyColumns.append(column.name);
            }
            
            // Parse validator
            if (colObj.contains("validator") && colObj["validator"].isObject()) {
                QJsonObject validatorObj = colObj["validator"].toObject();
                column.validator.type = stringToValidatorType(validatorObj["type"].toString());
                column.validator.pattern = validatorObj["pattern"].toString();
            }
            
            schema->columns.append(column);
        }
    }
    
    // Parse sorting
    if (root.contains("sorting") && root["sorting"].isArray()) {
        schema->sorting.clear();
        QJsonArray sortingArray = root["sorting"].toArray();
        for (const QJsonValue& sortValue : sortingArray) {
            if (!sortValue.isObject()) continue;
            
            QJsonObject sortObj = sortValue.toObject();
            SortRule sortRule;
            sortRule.columnName = sortObj["column"].toString();
            sortRule.order = stringToSortOrder(sortObj["order"].toString());
            schema->sorting.append(sortRule);
        }
    }

    // Parse queries
    if (root.contains("queries") && root["queries"].isObject()) {
        schema->queries.clear();
        QJsonObject queriesObj = root["queries"].toObject();
        
        for (auto it = queriesObj.begin(); it != queriesObj.end(); ++it) {
            QString queryName = it.key();
            QJsonObject queryObj = it.value().toObject();
            
            Query query;
            query.sql = queryObj["sql"].toString();
            query.onError = stringToErrorHandling(queryObj.value("on_error").toString());
            
            // Parse arguments
            if (queryObj.contains("arguments") && queryObj["arguments"].isArray()) {
                QJsonArray argsArray = queryObj["arguments"].toArray();
                for (const QJsonValue& argValue : argsArray) {
                    if (!argValue.isObject()) continue;
                    
                    QJsonObject argObj = argValue.toObject();
                    QueryArgument arg;
                    arg.name = argObj["name"].toString();
                    arg.type = stringToColumnType(argObj["type"].toString());
                    arg.isOptional = argObj.value("is_optional").toBool(false);
                    
                    if (argObj.contains("default")) {
                        arg.defaultValue = argObj["default"].toVariant();
                    }
                    
                    query.arguments.append(arg);
                }
            }
            
            schema->queries[queryName] = query;
        }
    }
    
    // Parse default error handling
    if (root.contains("default_error_handling") && root["default_error_handling"].isObject()) {
        QJsonObject errorObj = root["default_error_handling"].toObject();
        schema->defaultErrorHandling.onError = stringToErrorHandling(errorObj.value("on_error").toString());
        schema->defaultErrorHandling.message = errorObj.value("message").toString();
    }
    
    // Parse other settings
    if (root.contains("callback_is_required")) {
        schema->callbackIsRequired = root["callback_is_required"].toBool();
    }
    
    if (root.contains("default_row_tooltip")) {
        schema->defaultRowTooltip = root["default_row_tooltip"].toString();
    }
    
    if (root.contains("show_numeration")) {
        schema->showNumeration = root["show_numeration"].toBool();
    }

    return true;
}

QueryResult ModelCore::validateQueryParams(const QString& queryId, const QVariantMap& params) const {
    QueryResult result;
    result.ok = true;

    const Query* query = schema->findQuery(queryId);
    if (!query) {
        result.ok = false;
        result.log(QString("Query '%1' not found").arg(queryId));
        return result;
    }

    // Check required parameters
    for (const QueryArgument& arg : query->arguments) {
        if (!arg.isOptional && !params.contains(arg.name)) {
            result.ok = false;
            result.log(QString("Required parameter '%1' is missing for query '%2'")
                      .arg(arg.name, queryId));
            return result;
        }
        
        // Type validation could be added here
        if (params.contains(arg.name)) {
            const QVariant& value = params.value(arg.name);
            // Basic type checking could be implemented here based on arg.type
        }
    }

    return result;
}

} // namespace QForge::nsModel
