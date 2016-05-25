#include "meteowindow.h"
#include "ui_meteowindow.h"

#include <QtXml/QDomElement>
#include <QLocale>

MeteoWindow::MeteoWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MeteoWindow),
    nextBlockSize(0)
{
    ui->setupUi(this);
    QObject::connect(ui->dateEdit, SIGNAL(dateChanged(QDate)),
                    this, SLOT(slotDate1Changed(QDate)));
    QObject::connect(ui->dateEdit_2, SIGNAL(dateChanged(QDate)),
                    this, SLOT(slotDate2Changed(QDate)));


    tcpSocket = new QTcpSocket(this);

    connect(tcpSocket, SIGNAL(readyRead()),
            this,      SLOT(slotReadyRead()));

    tcpSocket->connectToHost("localhost", TCP_PORT);
}

MeteoWindow::~MeteoWindow()
{
    delete ui;
}

void MeteoWindow::setInterval(QDate &date1, QDate &date2)
{
    ui->dateEdit->setDateRange(date1,date2);
    ui->dateEdit->setDate(date1);
    ui->dateEdit_2->setDateRange(date1,date2);
    ui->dateEdit_2->setDate(date2);
}

void MeteoWindow::requestInterval(QDate &date1, QDate &date2)
{
    QLocale locale  = QLocale(QLocale::English, QLocale::UnitedStates);

    QDomText domText;

    QDomDocument doc("MeteoInterval");
    QDomElement  domElement = doc.createElement("MeteoInterval");
    doc.appendChild(domElement);

    QDomElement date1Element = doc.createElement("Date1");
    domText = doc.createTextNode(locale.toString(date1, "dd.MM.yyyy"));
    date1Element.appendChild(domText);
    domElement.appendChild(date1Element);

    QDomElement date2Element = doc.createElement("Date2");
    domText = doc.createTextNode(locale.toString(date2, "dd.MM.yyyy"));
    date2Element.appendChild(domText);
    domElement.appendChild(date2Element);

    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_6);
    out << quint16(0) << doc.toString();

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    tcpSocket->write(arrBlock);

}

void MeteoWindow::slotDate1Changed(QDate date1)
{
    if (date1 <= ui->dateEdit_2->date())
    {
        requestInterval(date1, ui->dateEdit_2->date());
    }
    else
    {
        ui->tableWidget->setRowCount(0);
        ui->tableWidget->setColumnCount(0);
    }
}
void MeteoWindow::slotDate2Changed(QDate date2)
{
    if (ui->dateEdit->date() <= date2)
    {
        requestInterval(ui->dateEdit->date(), date2);
    }
    else
    {
        ui->tableWidget->setRowCount(0);
        ui->tableWidget->setColumnCount(0);
    }
}

void MeteoWindow::slotReadyRead()
{
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_5_6);
    for (;;) {
        if (!nextBlockSize)
        {
            if (tcpSocket->bytesAvailable() < sizeof(quint16))
            {
                break;
            }
            in >> nextBlockSize;
        }

        if (tcpSocket->bytesAvailable() < nextBlockSize) {
            break;
        }
        QString str;
        in >> str;

        QDomDocument domDoc;
        domDoc.setContent(str);
        QDomElement domElement= domDoc.documentElement();
        if (domElement.tagName() == "MeteoInterval")
        {
            QDate date1;
            QDate date2;
            QDomNode domNode = domElement.firstChild();
            QDomElement domDate1Element = domNode.toElement();
            date1 = QDate::fromString(domDate1Element.text());

            domNode = domNode.nextSibling();
            QDomElement domDate2Element = domNode.toElement();
            date2 = QDate::fromString(domDate2Element.text());

            setInterval(date1, date2);
        }
        else if ( domElement.tagName() == "MeteoData")
        {
            QTableWidgetItem *newItem;
            ui->tableWidget->setRowCount(5);

            QDomNode domNode = domElement.firstChild();

            int nodeCount = 0;
            for(domNode; !domNode.isNull(); domNode = domNode.nextSibling())
            {
                nodeCount++;
            }

            ui->tableWidget->setColumnCount(nodeCount);

            QTableWidgetItem *tableWidgetItem = new QTableWidgetItem("Date");
            ui->tableWidget->setVerticalHeaderItem(0, tableWidgetItem);

            tableWidgetItem = new QTableWidgetItem("Wind power");
            ui->tableWidget->setVerticalHeaderItem(1, tableWidgetItem);

            tableWidgetItem = new QTableWidgetItem("Wind direction");
            ui->tableWidget->setVerticalHeaderItem(2, tableWidgetItem);

            tableWidgetItem = new QTableWidgetItem("Temperature");
            ui->tableWidget->setVerticalHeaderItem(3, tableWidgetItem);

            tableWidgetItem = new QTableWidgetItem("Visibility");
            ui->tableWidget->setVerticalHeaderItem(4, tableWidgetItem);

            int i = 0;
            for(domNode = domElement.firstChild(); !domNode.isNull(); domNode = domNode.nextSibling())
            {
                QDomElement domItemElement = domNode.toElement();

                QDomNode domParameterNode = domItemElement.firstChild();
                newItem = new QTableWidgetItem(domParameterNode.toElement().text() + " m/s");
                ui->tableWidget->setItem(1, i, newItem);

                domParameterNode = domParameterNode.nextSibling();
                switch (domParameterNode.toElement().text().toInt())
                {
                case 0:
                    newItem = new QTableWidgetItem("North");
                    break;
                case 1:
                    newItem = new QTableWidgetItem("North-East");
                    break;
                case 2:
                    newItem = new QTableWidgetItem("East");
                    break;
                case 3:
                    newItem = new QTableWidgetItem("South-East");
                    break;
                case 4:
                    newItem = new QTableWidgetItem("South");
                    break;
                case 5:
                    newItem = new QTableWidgetItem("South-West");
                    break;
                case 6:
                    newItem = new QTableWidgetItem("West");
                    break;
                case 7:
                    newItem = new QTableWidgetItem("North-West");
                    break;
                default:
                    newItem = new QTableWidgetItem("");
                    break;
                }
                ui->tableWidget->setItem(2, i, newItem);

                domParameterNode = domParameterNode.nextSibling();
                newItem = new QTableWidgetItem(domParameterNode.toElement().text() + " C");
                ui->tableWidget->setItem(3, i, newItem);

                domParameterNode = domParameterNode.nextSibling();
                newItem = new QTableWidgetItem(domParameterNode.toElement().text() + " m");
                ui->tableWidget->setItem(4, i, newItem);

                domParameterNode = domParameterNode.nextSibling();
                newItem = new QTableWidgetItem(domParameterNode.toElement().text());
                ui->tableWidget->setItem(0, i, newItem);

                i++;
            }
        }

        nextBlockSize = 0;
    }

}
