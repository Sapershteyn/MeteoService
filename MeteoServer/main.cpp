#include <QCoreApplication>

#include "meteoserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    MeteoServer* meteoServer = new MeteoServer();

    return a.exec();
}
