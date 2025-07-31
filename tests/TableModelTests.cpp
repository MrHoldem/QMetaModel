#include "TableModelTests.h"
#include <QtTest>
#include <QDir>
#include <QFile>

void TableModelTests::initTestCase()
{
    qDebug() << "Начинаем тестирование TableModel";
}

void TableModelTests::cleanupTestCase()
{
    qDebug() << "Тестирование завершено";
}

void TableModelTests::testExample()
{
    QVERIFY2(true, "Пример успешного теста");
}

// ========== НОВЫЕ ТЕСТЫ ==========

void TableModelTests::testAlbumModelYaml()
{
    qDebug() << "Тестирование загрузки AlbumModel.yml";
    
    // Создаем ожидаемую схему
    ModelSchema expected = createExpectedAlbumSchema();
    
    // Загружаем схему из YAML файла
    QString yamlPath = getProjectRoot() + "/examples/AlbumModel.yml";
    ModelCore core(yamlPath, dummyQueryHandler);
    
    if (!core.isValid()) {
        qDebug() << "YAML загрузка не удалась. Ошибки:";
        QStringList errors = core.getErrors();
        for (const QString& error : errors) {
            qDebug() << error;
        }
        qDebug() << "Путь к файлу:" << yamlPath;
        qDebug() << "Корень проекта:" << getProjectRoot();
        qDebug() << "Файл существует:" << QFile::exists(yamlPath);
    }
    
    QVERIFY2(core.isValid(), "ModelCore должен успешно загрузить YAML файл");
    
    const ModelSchema& actual = core.getSchema();
    
    // Сравниваем схемы
    compareSchemas(expected, actual);
}

void TableModelTests::testAlbumModelJson()
{
    qDebug() << "Тестирование загрузки AlbumModel.json";
    
    // Создаем ожидаемую схему - теперь JSON совпадает с YAML
    ModelSchema expected = createExpectedAlbumSchema();
    
    // Загружаем схему из JSON файла
    QString jsonPath = getProjectRoot() + "/examples/AlbumModel.json";
    ModelCore core(jsonPath, dummyQueryHandler);
    
    if (!core.isValid()) {
        qDebug() << "JSON загрузка не удалась. Ошибки:";
        QStringList errors = core.getErrors();
        for (const QString& error : errors) {
            qDebug() << error;
        }
        qDebug() << "Путь к файлу:" << jsonPath;
        qDebug() << "Корень проекта:" << getProjectRoot();
        qDebug() << "Файл существует:" << QFile::exists(jsonPath);
    }
    
    QVERIFY2(core.isValid(), "ModelCore должен успешно загрузить JSON файл");
    
    const ModelSchema& actual = core.getSchema();
    
    // Сравниваем схемы
    compareSchemas(expected, actual);
}

void TableModelTests::testModelSchemaValidation()
{
    qDebug() << "Тестирование валидации ModelSchema";
    
    // Тест валидной схемы
    ModelSchema validSchema = createExpectedAlbumSchema();
    QStringList errors = validSchema.validate();
    QVERIFY2(errors.isEmpty(), "Валидная схема не должна иметь ошибок");
    
    // Тест невалидной схемы - без имени
    ModelSchema invalidSchema;
    errors = invalidSchema.validate();
    QVERIFY2(!errors.isEmpty(), "Схема без имени должна иметь ошибки валидации");
    QVERIFY2(errors.join(" ").contains("name"), "Ошибка должна содержать информацию об отсутствии имени");
}

void TableModelTests::testModelCoreExecution()
{
    qDebug() << "Тестирование выполнения запросов через ModelCore";
    
    // Загружаем валидную схему
    QString yamlPath = getProjectRoot() + "/examples/AlbumModel.yml";
    ModelCore core(yamlPath, dummyQueryHandler);
    
    if (!core.isValid()) {
        qDebug() << "ModelCore execution test - YAML загрузка не удалась. Ошибки:";
        QStringList errors = core.getErrors();
        for (const QString& error : errors) {
            qDebug() << error;
        }
        qDebug() << "Путь к файлу:" << yamlPath;
        qDebug() << "Корень проекта:" << getProjectRoot();
        qDebug() << "Файл существует:" << QFile::exists(yamlPath);
    }
    
    QVERIFY2(core.isValid(), "ModelCore должен быть валидным");
    
    // Тестируем выполнение простого запроса
    QVariantMap params;
    QueryResult result = core.execute("select_all", params);
    QVERIFY2(result.ok, "Запрос select_all должен выполниться успешно");
    
    // Тестируем запрос с параметрами
    params["id"] = "test-uuid";
    result = core.execute("select_by_id", params);
    QVERIFY2(result.ok, "Запрос select_by_id должен выполниться успешно");
    
    // Тестируем несуществующий запрос
    result = core.execute("nonexistent_query", params);
    QVERIFY2(!result.ok, "Несуществующий запрос должен вернуть ошибку");
}

// ========== ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ ==========

QString TableModelTests::getProjectRoot()
{
    QString projectRoot = QDir::currentPath();
    // Находим корень проекта, поднимаясь по каталогам до QMetaModel.pro
    while (!projectRoot.isEmpty() && !QFile::exists(projectRoot + "/QMetaModel.pro")) {
        QDir dir(projectRoot);
        if (!dir.cdUp()) break;
        projectRoot = dir.absolutePath();
    }
    return projectRoot;
}

ModelSchema TableModelTests::createExpectedAlbumSchema()
{
    ModelSchema schema;
    
    // Основные настройки
    schema.name = "AlbumModel";
    schema.type = ModelType::Table;
    schema.description = "Модель для работы с целевыми альбомами.";
    schema.source = DataSource::Query;
    schema.isEditable = true;
    
    // Заголовки
    schema.horizontalHeaders = {"Название", "Цель", "Дата создания"};
    
    // Колонки
    Column idColumn;
    idColumn.name = "id";
    idColumn.type = ColumnType::Uuid;
    idColumn.isPrimaryKey = true;
    idColumn.isVisible = false;
    schema.columns.append(idColumn);
    schema.primaryKeyColumns.append("id");
    
    Column titleColumn;
    titleColumn.name = "title";
    titleColumn.type = ColumnType::String;
    titleColumn.isVisible = true;
    titleColumn.isEditable = true;
    titleColumn.tooltip = "Название альбома";
    schema.columns.append(titleColumn);
    
    Column goalColumn;
    goalColumn.name = "goal";
    goalColumn.type = ColumnType::String;
    goalColumn.isVisible = true;
    goalColumn.isEditable = true;
    goalColumn.tooltip = "Цель альбома";
    goalColumn.validator.type = ValidatorType::Regexp;
    goalColumn.validator.pattern = "^[A-Za-zА-Яа-яЁё\\s]+$";
    schema.columns.append(goalColumn);
    
    Column createdAtColumn;
    createdAtColumn.name = "created_at";
    createdAtColumn.type = ColumnType::DateTime;
    createdAtColumn.isVisible = true;
    createdAtColumn.isEditable = false;
    createdAtColumn.tooltip = "Дата создания альбома";
    createdAtColumn.alignment = TextAlignment::Center;
    schema.columns.append(createdAtColumn);
    
    // Сортировка
    SortRule sortByDate;
    sortByDate.columnName = "created_at";
    sortByDate.order = SortOrder::Descending;
    schema.sorting.append(sortByDate);
    
    SortRule sortByTitle;
    sortByTitle.columnName = "title";
    sortByTitle.order = SortOrder::Ascending;
    schema.sorting.append(sortByTitle);
    
    // Запросы (добавляем все 4 запроса из YAML файла)
    Query selectAllQuery;
    selectAllQuery.sql = "SELECT * FROM ap.target_albums";
    selectAllQuery.onError = ErrorHandling::Callback;
    schema.queries["select_all"] = selectAllQuery;
    
    Query selectByIdQuery;
    QueryArgument idArg;
    idArg.name = "id";
    idArg.type = ColumnType::Uuid;
    idArg.isOptional = false;
    selectByIdQuery.arguments.append(idArg);
    
    QueryArgument fieldsArg;
    fieldsArg.name = "fields";
    fieldsArg.type = ColumnType::Array;
    fieldsArg.isOptional = true;
    fieldsArg.defaultValue = QStringList{"*"};
    selectByIdQuery.arguments.append(fieldsArg);
    
    selectByIdQuery.sql = "SELECT ${fields} FROM ap.target_albums WHERE id = ${id}";
    schema.queries["select_by_id"] = selectByIdQuery;
    
    Query insertQuery;
    QueryArgument insertIdArg;
    insertIdArg.name = "id";
    insertIdArg.type = ColumnType::Uuid;
    insertIdArg.isOptional = true;
    insertIdArg.defaultValue = "gen_random_uuid()";
    insertQuery.arguments.append(insertIdArg);
    
    QueryArgument titleArg;
    titleArg.name = "title";
    titleArg.type = ColumnType::String;
    titleArg.isOptional = false;
    insertQuery.arguments.append(titleArg);
    
    QueryArgument goalArg;
    goalArg.name = "goal";
    goalArg.type = ColumnType::String;
    goalArg.isOptional = true;
    insertQuery.arguments.append(goalArg);
    
    insertQuery.sql = "INSERT INTO ap.target_albums (title, goal)\nVALUES (${title}, ${goal})\nRETURNING id\n";
    schema.queries["insert"] = insertQuery;
    
    Query removeQuery;
    QueryArgument removeIdArg;
    removeIdArg.name = "id";
    removeIdArg.type = ColumnType::Uuid;
    removeIdArg.isOptional = false;
    removeQuery.arguments.append(removeIdArg);
    
    removeQuery.sql = "DELETE FROM ap.target_albums WHERE id = ${id}";
    schema.queries["remove"] = removeQuery;
    
    // Обработка ошибок
    schema.defaultErrorHandling.onError = ErrorHandling::ShowMessage;
    schema.defaultErrorHandling.message = "Произошла ошибка при выполнении запроса: ${last_error}";
    
    // Дополнительные настройки
    schema.callbackIsRequired = true;
    schema.defaultRowTooltip = "Альбом: ${title} (id: ${id})";
    schema.showNumeration = true;
    
    return schema;
}

void TableModelTests::compareSchemas(const ModelSchema& expected, const ModelSchema& actual)
{
    // Сравниваем основные поля
    QCOMPARE(actual.name, expected.name);
    QCOMPARE(actual.type, expected.type);
    QCOMPARE(actual.description, expected.description);
    QCOMPARE(actual.source, expected.source);
    QCOMPARE(actual.isEditable, expected.isEditable);
    
    // Сравниваем заголовки
    QCOMPARE(actual.horizontalHeaders, expected.horizontalHeaders);
    
    // Сравниваем количество колонок
    QCOMPARE(actual.columns.size(), expected.columns.size());
    
    // Сравниваем каждую колонку
    for (int i = 0; i < expected.columns.size(); ++i) {
        const Column& expectedCol = expected.columns[i];
        const Column& actualCol = actual.columns[i];
        
        QCOMPARE(actualCol.name, expectedCol.name);
        QCOMPARE(actualCol.type, expectedCol.type);
        QCOMPARE(actualCol.isVisible, expectedCol.isVisible);
        QCOMPARE(actualCol.isEditable, expectedCol.isEditable);
        QCOMPARE(actualCol.isPrimaryKey, expectedCol.isPrimaryKey);
        QCOMPARE(actualCol.tooltip, expectedCol.tooltip);
        
        if (expectedCol.validator.type != ValidatorType::None) {
            QCOMPARE(actualCol.validator.type, expectedCol.validator.type);
            QCOMPARE(actualCol.validator.pattern, expectedCol.validator.pattern);
        }
    }
    
    // Сравниваем сортировку
    QCOMPARE(actual.sorting.size(), expected.sorting.size());
    for (int i = 0; i < expected.sorting.size(); ++i) {
        QCOMPARE(actual.sorting[i].columnName, expected.sorting[i].columnName);
        QCOMPARE(actual.sorting[i].order, expected.sorting[i].order);
    }
    
    // Сравниваем запросы
    QCOMPARE(actual.queries.size(), expected.queries.size());
    for (auto it = expected.queries.begin(); it != expected.queries.end(); ++it) {
        QString queryName = it.key();
        QVERIFY2(actual.queries.contains(queryName), 
                QString("Запрос '%1' должен присутствовать").arg(queryName).toLatin1());
        
        const Query& expectedQuery = it.value();
        const Query& actualQuery = actual.queries[queryName];
        
        QCOMPARE(actualQuery.sql, expectedQuery.sql);
        QCOMPARE(actualQuery.onError, expectedQuery.onError);
        QCOMPARE(actualQuery.arguments.size(), expectedQuery.arguments.size());
    }
    
    // Сравниваем обработку ошибок
    QCOMPARE(actual.defaultErrorHandling.onError, expected.defaultErrorHandling.onError);
    QCOMPARE(actual.defaultErrorHandling.message, expected.defaultErrorHandling.message);
    
    // Сравниваем дополнительные настройки
    QCOMPARE(actual.callbackIsRequired, expected.callbackIsRequired);
    QCOMPARE(actual.defaultRowTooltip, expected.defaultRowTooltip);
    QCOMPARE(actual.showNumeration, expected.showNumeration);
}

QueryResult TableModelTests::dummyQueryHandler(const QueryContext& context)
{
    QueryResult result;
    result.ok = true;
    result.log(QString("Выполнен запрос: %1").arg(context.queryName));
    
    // Эмулируем результат запроса
    if (context.queryName == "select_all") {
        // Возвращаем пустой результат для простоты
        result.rows = QList<QVariantMap>();
    } else if (context.queryName == "select_by_id") {
        // Проверяем наличие обязательного параметра
        if (!context.bindings.contains("id")) {
            result.ok = false;
            result.log("Отсутствует обязательный параметр 'id'");
        }
    }
    
    return result;
}
