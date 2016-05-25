#include "meteowindow.h"

#include <QApplication>
#include <QObject>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MeteoWindow meteoWindow;

    meteoWindow.show();

    return a.exec();
}
