#include <QApplication>
#include "ProductWidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    ProductWidget widget;
    widget.show();
    
    return app.exec();
}