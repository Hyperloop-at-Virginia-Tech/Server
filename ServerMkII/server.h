#ifndef SERVER_H
#define SERVER_H

#include <QDialog>
#include <QDataStream>
#include <QTcpServer>
#include <QLabel>
#include <QNetworkSession>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QTcpSocket>
#include <QDebug>


class Server : public QDialog
{
    Q_OBJECT

public:
    explicit Server(QWidget *parent = nullptr);

private slots:
    void sessionOpened();
    void sendData();
    void generateData();

private:
    QLabel *statusLabel;
    QTcpServer *server;
    QVector<QString> data;
    QNetworkSession *networkSession = nullptr;
};
#endif // SERVER_H
