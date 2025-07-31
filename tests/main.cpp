#include <QCoreApplication>
#include <QtTest>

#include "TableModelTests.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    int status = 0;
    status |= QTest::qExec(new TableModelTests, argc, argv);

    return status;
}
