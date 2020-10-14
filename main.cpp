#include "PhotoSync.h"
#include <QtWidgets/QApplication>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    PhotoSync window;
    window.show();
    return app.exec();
}
