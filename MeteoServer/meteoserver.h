#ifndef METEOSERVER_H
#define METEOSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QString>
#include <QDateTime>
#include <QList>

#define TCP_PORT 1111
#define MAX_WIND_POWER 20
#define TEMPERATURE_RANGE 100
#define MAX_VISIBILITY 500
#define MONTH 30

enum WindDirection
{
    NORTH,
    NORTHEAST,
    EAST,
    SOUTHEAST,
    SOUTH,
    SOUTHWEST,
    WEST,
    NORTHWEST,
    COUNT
};

class MeteoItem
{
public:
    int windPower;
    WindDirection windDirection;
    int temperature;
    int visibilityRange;
    QDateTime date;

    void Generate();
};

class MeteoData
{
    QList<MeteoItem> data;
public:
    //Generate meteo data
    void Generate();
    //Get max interval
    QString GetInterval();
    //Get data for interval
    QString GetData(QDate date1, QDate date2);
};

class MeteoServer : public QObject
{
    Q_OBJECT
private:
    MeteoData* meteoData;
    QTcpServer* tcpServer;
    QTcpSocket* tcpSocket;
    quint16     nextBlockSize;

    QDate getDate1(QString str);
    QDate getDate2(QString str);

    void Send(QString str);

public:
    MeteoServer();
    ~MeteoServer();

    public slots:
    virtual void slotNewConnection();
            void slotReceiveInterval();
};

#endif // METEOSERVER_H
