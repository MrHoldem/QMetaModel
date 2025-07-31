#pragma once

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVector>
#include <QMap>
#include <QHash>
#include <QDateTime>
#include <QColor>
#include <QFont>

namespace QForge {

// ========== ENUMS ==========

enum class ModelType {
    Table,
    Tree
};

enum class DataSource {
    Query,
    Manual
};

enum class ColumnType {
    String,
    Integer,
    Double,
    Boolean,
    DateTime,
    Date,
    Time,
    Uuid,
    Binary,
    Json,
    Array,
    Custom
};

enum class TextAlignment {
    Left,
    Center,
    Right,
    Justify
};

enum class SortOrder {
    Ascending,
    Descending
};

enum class ErrorHandling {
    ShowMessage,
    Ignore,
    Log
};

enum class ValidatorType {
    None,
    Regexp,
    Range,
    Length,
    Custom,
    Required
};

enum class HeaderType {
    Numeric,
    Alphabetic,
    Custom
};

enum class SelectionBehavior {
    SelectItems,
    SelectRows,
    SelectColumns
};

enum class SelectionMode {
    NoSelection,
    SingleSelection,
    MultiSelection,
    ExtendedSelection,
    ContiguousSelection
};

enum class EditTrigger {
    NoEditTriggers,
    CurrentChanged,
    DoubleClicked,
    SelectedClicked,
    EditKeyPressed,
    AnyKeyPressed,
    AllEditTriggers
};

// ========== HELPER STRUCTURES ==========

struct Range {
    QVariant min;
    QVariant max;
    bool includeMin = true;
    bool includeMax = true;
};

struct LengthConstraint {
    int minLength = 0;
    int maxLength = INT_MAX;
};

struct Validator {
    ValidatorType type = ValidatorType::None;
    QString pattern;
    Range range;
    LengthConstraint length;
    QString customValidatorName;
    QString errorMessage;
    bool isRequired = false;
};

struct QueryArgument {
    QString name;
    ColumnType type = ColumnType::String;
    QVariant defaultValue;
    bool isOptional = false;
    QString description;
    Validator validator;
};

struct Query {
    QString sql;
    QVector<QueryArgument> arguments;
    ErrorHandling onError = ErrorHandling::ShowMessage;
    QString errorMessage;
    QString description;
    int timeoutMs = 30000;
    bool isTransactional = false;
    bool isReadOnly = true;
};

struct SortRule {
    QString columnName;
    SortOrder order = SortOrder::Ascending;
    int priority = 0;
};

struct StyleSettings {
    QColor backgroundColor;
    QColor foregroundColor;
    QColor alternateBackgroundColor;
    QColor selectionBackgroundColor;
    QColor selectionForegroundColor;
    QFont font;
    int rowHeight = -1;
    int columnWidth = -1;
    bool showGrid = true;
    QColor gridColor;
};

struct HeaderSettings {
    HeaderType type = HeaderType::Numeric;
    QStringList customLabels;
    int startIndex = 1;
    QString startLetter = "A";
    StyleSettings style;
    QString tooltip;
    bool isEnumerated = false; // For numeric headers

    HeaderSettings& operator=(const QString& value);
};

struct ErrorHandlingSettings {
    ErrorHandling onError = ErrorHandling::ShowMessage;
    QString message = "An error occurred: ${last_error}";
    QString logFormat;
    bool showStackTrace = false;
};

struct PerformanceSettings {
    bool lazyLoading = false;
    int batchSize = 1000;
    bool enableCaching = true;
    int cacheSize = 10000;
    bool asyncOperations = false;
    int maxConcurrentQueries = 3;
};

struct SecuritySettings {
    bool enableSqlInjectionProtection = true;
    QStringList allowedOperations;
    QStringList forbiddenKeywords;
    bool enableInputSanitization = true;
    bool requireAuthentication = false;
    QStringList requiredRoles;
};

struct Column {
    QString name;
    ColumnType type = ColumnType::String;
    QString displayName;
    QString tooltip;
    
    // Editing settings
    bool isEditable = true;
    bool isPrimaryKey = false;
    bool isAutoIncrement = false;
    bool isUnique = false;
    bool isIndexed = false;
    
    // Display settings
    TextAlignment alignment = TextAlignment::Left;
    int width = -1;
    int minWidth = 50;
    int maxWidth = INT_MAX;
    bool isResizable = true;
    bool isSortable = true;
    
    // Data formatting
    QString format;
    QString nullDisplayText = "";
    QVariant defaultValue;
    
    // Validation
    Validator validator;
    
    // Custom properties
    QHash<QString, QVariant> customProperties;
    
    // Style
    StyleSettings style;
    
    // Relationship settings (for foreign keys)
    QString referenceTable;
    QString referenceColumn;
    QString displayColumn;
    
    // Calculated field settings
    bool isCalculated = false;
    QString calculationExpression;
    QStringList dependentColumns;
};

struct HookSettings {
    QString preExecuteCallback;
    QString postExecuteCallback;
    QString rowMappedCallback;
    QString errorCaughtCallback;
    QString dataChangedCallback;
    QString selectionChangedCallback;
    QString beforeEditCallback;
    QString afterEditCallback;
};

struct LocalizationSettings {
    QString locale = "en_US";
    QHash<QString, QString> translations;
    QString dateFormat = "yyyy-MM-dd";
    QString timeFormat = "hh:mm:ss";
    QString datetimeFormat = "yyyy-MM-dd hh:mm:ss";
    QString numberFormat;
    QString currencySymbol;
};

struct ExportSettings {
    QStringList supportedFormats = {"csv", "json", "xml"};
    QString defaultFormat = "csv";
    QString csvDelimiter = ",";
    QString csvQuoteChar = "\"";
    bool csvIncludeHeaders = true;
    bool jsonPrettyPrint = true;
    QString xmlRootElement = "data";
    QString xmlRowElement = "row";
};

struct ImportSettings {
    QStringList supportedFormats = {"csv", "json", "xml"};
    QString defaultFormat = "csv";
    QString csvDelimiter = ",";
    QString csvQuoteChar = "\"";
    bool csvHasHeaders = true;
    bool validateOnImport = true;
    bool replaceExistingData = false;
    int batchSize = 1000;
};

// ========== MAIN SCHEMA CLASS ==========

class ModelSchema {
public:
    // Basic model information
    QString name;
    ModelType type = ModelType::Table;
    QString description;
    QString version = "1.0";
    QDateTime createdAt;
    QDateTime modifiedAt;
    QString author;
    
    // Data source configuration
    DataSource source = DataSource::Query;
    QString loadQuery = "select_all";
    bool isEditable = true;
    bool isReadOnly = false;
    
    // Columns definition
    QVector<Column> columns;
    QStringList primaryKeyColumns;
    
    // Headers configuration
    QStringList horizontalHeaders;
    HeaderSettings verticalHeaders;
    HeaderSettings horizontalHeaderSettings;
    
    // Sorting configuration
    QVector<SortRule> sorting;
    bool isSortingEnabled = true;
    bool isMultiColumnSortingEnabled = false;
    
    // Queries definition
    QHash<QString, Query> queries;
    QString defaultQuery = "select_all";
    
    // Error handling
    ErrorHandlingSettings defaultErrorHandling;
    
    // UI settings
    QString defaultRowTooltip;
    bool showNumeration = true;
    SelectionBehavior selectionBehavior = SelectionBehavior::SelectRows;
    SelectionMode selectionMode = SelectionMode::SingleSelection;
    EditTrigger editTriggers = EditTrigger::DoubleClicked;
    
    // Style and appearance
    StyleSettings style;
    bool alternatingRowColors = true;
    bool showVerticalHeader = true;
    bool showHorizontalHeader = true;
    
    // Advanced features
    HookSettings hooks;
    PerformanceSettings performance;
    SecuritySettings security;
    LocalizationSettings localization;
    ExportSettings exportSettings;
    ImportSettings importSettings;
    
    // Pagination settings
    bool enablePagination = false;
    int pageSize = 100;
    int currentPage = 1;
    bool showPageControls = true;
    
    // Filter settings
    bool enableFiltering = false;
    QHash<QString, QVariant> defaultFilters;
    bool caseSensitiveFiltering = false;
    
    // Custom properties
    QHash<QString, QVariant> customProperties;
    
    // Metadata
    QHash<QString, QString> metadata;
    
    // Constructor with defaults
    ModelSchema() {
        createdAt = QDateTime::currentDateTime();
        modifiedAt = createdAt;
        
        // Setup default vertical headers
        verticalHeaders.type = HeaderType::Numeric;
        verticalHeaders.startIndex = 1;
        
        // Setup default horizontal headers
        horizontalHeaderSettings.type = HeaderType::Custom;
        
        // Default error handling
        defaultErrorHandling.onError = ErrorHandling::ShowMessage;
        defaultErrorHandling.message = "An error occurred: ${last_error}";
        
        // Default performance settings
        performance.batchSize = 1000;
        performance.enableCaching = true;
        performance.cacheSize = 10000;
        performance.maxConcurrentQueries = 3;
        
        // Default security settings
        security.enableSqlInjectionProtection = true;
        security.enableInputSanitization = true;
        
        // Default localization
        localization.locale = "en_US";
        localization.dateFormat = "yyyy-MM-dd";
        localization.timeFormat = "hh:mm:ss";
        localization.datetimeFormat = "yyyy-MM-dd hh:mm:ss";
    }
    
    // Utility methods
    Column* findColumn(const QString& columnName);
    const Column* findColumn(const QString& columnName) const;
    Query* findQuery(const QString& queryName);
    const Query* findQuery(const QString& queryName) const;
    
    bool isValid() const;
    QStringList validate() const;
    
    void addColumn(const Column& column);
    void removeColumn(const QString& columnName);
    void addQuery(const QString& name, const Query& query);
    void removeQuery(const QString& name);
    
    QStringList getEditableColumns() const;
    QStringList getPrimaryKeyColumns() const;
};

} // namespace QForge
