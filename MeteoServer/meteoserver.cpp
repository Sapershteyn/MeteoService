#include "meteoserver.h"

#include <QDataStream>
#include <QtXml/QDomDocument>
#include <QLocale>

MeteoServer::MeteoServer() :
    nextBlockSize(0)
{
    tcpServer = new QTcpServer(this);

    tcpServer->setMaxPendingConnections(1);

    if (!tcpServer->listen(QHostAddress::Any, TCP_PORT))
    {
        printf("Unable to start the meteo server\n");
        tcpServer->close();
        return;
    }
    connect(tcpServer, SIGNAL(newConnection()),
            this,      SLOT(slotNewConnection())
            );

    meteoData = new MeteoData();
    meteoData->Generate();
}

MeteoServer::~MeteoServer()
{
    delete meteoData;
    tcpServer->close();
}

void MeteoServer::slotNewConnection()
{
    tcpSocket = tcpServer->nextPendingConnection();
    connect(tcpSocket, SIGNAL(disconnected()),
            tcpSocket, SLOT(deleteLater())
           );
    connect(tcpSocket, SIGNAL(readyRead()),
            this,      SLOT(slotReceiveInterval())
           );

    Send(meteoData->GetInterval());
}

void MeteoServer::slotReceiveInterval()
{
    tcpSocket = (QTcpSocket*)sender();
    QDataStream in(tcpSocket);

    in.setVersion(QDataStream::Qt_5_6);
    for (;;) {
        if (!nextBlockSize) {
            if (tcpSocket->bytesAvailable() < sizeof(quint16)) {
                break;
            }
            in >> nextBlockSize;
        }

        if (tcpSocket->bytesAvailable() < nextBlockSize) {
            break;
        }
        QString str;
        in >> str;

        nextBlockSize = 0;

        QDate date1 = getDate1(str);
        QDate date2 = getDate2(str);

        QString intervalData = meteoData->GetData(date1, date2);
        Send(intervalData);
    }
}

QDate MeteoServer::getDate1(QString str)
{
    QDomDocument domDoc;
    domDoc.setContent(str);

    QDomElement domElement= domDoc.documentElement();
    QDomNode domNode = domElement.firstChild();
    QDomElement domSubElement = domNode.toElement();

    QDate date;
    date = QDate::fromString(domSubElement.text(), "dd.MM.yyyy");
    return date;
}

QDate MeteoServer::getDate2(QString str)
{
    QDomDocument domDoc;
    domDoc.setContent(str);

    QDomElement domElement= domDoc.documentElement();
    QDomNode domNode = domElement.firstChild();
    domNode = domNode.nextSibling();
    QDomElement domSubElement = domNode.toElement();

    QDate date;
    date = QDate::fromString(domSubElement.text(), "dd.MM.yyyy");
    return date;
}

void MeteoServer::Send(QString str)
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_6);
    out << quint16(0) << str;

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    int res = tcpSocket->write(arrBlock);
}

void MeteoData::Generate()
{
    int dayNumber = qrand() % MONTH + MONTH;
    QDate date1 = QDate(2016, 6, 1);
    QDate date2 = date1.addDays(dayNumber);

    QDate date = date1;
    while (date < date2)
    {
        MeteoItem meteoItem0;
        meteoItem0.date.setDate(date);
        meteoItem0.date.setTime(QTime(0,0));
        meteoItem0.Generate();
        data.push_back(meteoItem0);

        MeteoItem meteoItem12;
        meteoItem12.date.setDate(date);
        meteoItem12.date.setTime(QTime(12,0));
        meteoItem12.Generate();
        data.push_back(meteoItem12);

        date = date.addDays(1);
    }
}

QString MeteoData::GetInterval()
{
    QDomText domText;

    QDomDocument doc("MeteoInterval");
    QDomElement  domElement = doc.createElement("MeteoInterval");
    doc.appendChild(domElement);

    QDomElement date1Element = doc.createElement("Date1");
    domText = doc.createTextNode(data[0].date.date().toString());
    date1Element.appendChild(domText);
    domElement.appendChild(date1Element);

    QDomElement date2Element = doc.createElement("Date2");
    domText = doc.createTextNode(data[data.size() - 1].date.date().toString());
    date2Element.appendChild(domText);
    domElement.appendChild(date2Element);

    return doc.toString();
}

QString MeteoData::GetData(QDate date1, QDate date2)
{
    QDomText domText;
    QLocale locale  = QLocale(QLocale::English, QLocale::UnitedStates);
    QDomDocument doc("MeteoData");
    QDomElement  domElement = doc.createElement("MeteoData");
    doc.appendChild(domElement);
    for (int i = 0; i < data.size(); i++)
    {
        if (data[i].date.date() < date1 || data[i].date.date() > date2 )
            continue;

        QString elemName = "item" + QString::number(i);
        QDomElement domSubElement = doc.createElement(elemName);

        QDomElement windPowerElem = doc.createElement("WindPower");
        domText = doc.createTextNode(QString::number(data[i].windPower));
        windPowerElem.appendChild(domText);
        domSubElement.appendChild(windPowerElem);

        QDomElement windDirectionElem = doc.createElement("WindDirection");
        domText = doc.createTextNode(QString::number(data[i].windDirection));
        windDirectionElem.appendChild(domText);
        domSubElement.appendChild(windDirectionElem);

        QDomElement temperatureElem = doc.createElement("Temperature");
        domText = doc.createTextNode(QString::number(data[i].temperature));
        temperatureElem.appendChild(domText);
        domSubElement.appendChild(temperatureElem);

        QDomElement visibilityElem = doc.createElement("Visibility");
        domText = doc.createTextNode(QString::number(data[i].visibilityRange));
        visibilityElem.appendChild(domText);
        domSubElement.appendChild(visibilityElem);

        QDomElement dateElem = doc.createElement("Date");
        domText = doc.createTextNode(locale.toString(data[i].date, "dd.MM.yyyy hh:mm"));
        dateElem.appendChild(domText);
        domSubElement.appendChild(dateElem);

        domElement.appendChild(domSubElement);
    }
    return doc.toString();
}

void MeteoItem::Generate()
{
    windPower = qrand() % MAX_WIND_POWER;
    windDirection =  (WindDirection)(qrand() % WindDirection::COUNT);
    temperature = qrand() % TEMPERATURE_RANGE - TEMPERATURE_RANGE / 2;
    visibilityRange = qrand() % MAX_VISIBILITY;
}
