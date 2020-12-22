#include "PhotoSync.h"
#include <QApplication>
#include "NodeContainer.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qRegisterMetaType<NodeContainer>();

    PhotoSync window;
    window.show();
    return app.exec();
}
