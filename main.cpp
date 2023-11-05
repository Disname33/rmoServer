#include <QtCore/QCoreApplication>
#include <QtCore/qglobal.h>
#include "rmoserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    RmoServer server;
    return a.exec();
}
