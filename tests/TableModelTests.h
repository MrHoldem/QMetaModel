#pragma once

#include <QObject>
#include <QString>
#include <QDebug>
#include <QtTest>

// Подключаем наши классы для тестирования
#include "private/ModelCore.h"
#include "private/ModelSchema.h"
#include "QueryResult.hpp"

// Используем полные имена для избежания конфликтов
using QForge::nsModel::ModelCore;
using QForge::nsModel::QueryResult;
using QForge::nsModel::QueryContext;
using QForge::ModelSchema;
using QForge::Column;
using QForge::Query;
using QForge::QueryArgument;
using QForge::SortRule;
using QForge::ModelType;
using QForge::DataSource;
using QForge::ColumnType;
using QForge::TextAlignment;
using QForge::SortOrder;
using QForge::ErrorHandling;
using QForge::ValidatorType;

class TableModelTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();     // Вызывается перед всеми тестами
    void cleanupTestCase();  // После всех тестов

    void testExample();      // Сам тест
    
    // Тесты для ModelSchema и ModelCore
    void testAlbumModelYaml();     // Тест загрузки AlbumModel.yml
    void testAlbumModelJson();     // Тест загрузки AlbumModel.json
    void testModelSchemaValidation(); // Тест валидации схемы
    void testModelCoreExecution();    // Тест выполнения запросов

private:
    // Вспомогательные методы
    ModelSchema createExpectedAlbumSchema();
    ModelSchema createExpectedProjectSchema();
    void compareSchemas(const ModelSchema& expected, const ModelSchema& actual);
    QString getProjectRoot();
    
    // Dummy query handler для тестов
    static QueryResult dummyQueryHandler(const QueryContext& context);
};
