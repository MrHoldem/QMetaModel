#include "TableModelTests.h"
#include <QtTest>

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
