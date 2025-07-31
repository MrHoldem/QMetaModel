#pragma once

#include <QObject>

class TableModelTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();     // Вызывается перед всеми тестами
    void cleanupTestCase();  // После всех тестов

    void testExample();      // Сам тест
};
