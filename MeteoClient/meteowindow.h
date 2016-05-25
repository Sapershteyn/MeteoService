#ifndef METEOWINDOW_H
#define METEOWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QDateTime>

#define TCP_PORT 1111

namespace Ui {
class MeteoWindow;
}

class MeteoWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MeteoWindow(QWidget *parent = 0);
    ~MeteoWindow();
    void setInterval(QDate &date1, QDate &date2);
    void requestInterval(QDate &date1, QDate &date2);

private:
    Ui::MeteoWindow *ui;
    QTcpSocket* tcpSocket;

    quint16     nextBlockSize;

public slots:
    void slotDate1Changed(QDate date1);
    void slotDate2Changed(QDate date2);
    void slotReadyRead();
};

#endif // METEOWINDOW_H
