#include "PhotoSync.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PhotoSync w;
    w.show();
    return a.exec();
}
