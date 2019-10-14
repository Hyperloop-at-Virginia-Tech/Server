#include "server.h"
#include "ui_server.h"

#include <QtWidgets>
#include <QtNetwork>
#include <QtCore>
/**
 * @brief Server::Server This is the server class that we will use
 * @param parent Gets the QWidget parent that we want
 */
Server::Server(QWidget *parent)
    : QDialog(parent)
    , statusLabel(new QLabel)
{
    //sets up the status label
    statusLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

    //This sets up the manager
    QNetworkConfigurationManager manager;
    //Searches for a config that a user perfers if it is available
    if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired) {
        QSettings settings(QSettings::UserScope, QLatin1String("Hyperloop"));
        settings.beginGroup(QLatin1String("Hyperloop Server"));
        const QString id = settings.value(QLatin1String("DefaultConfig")).toString();
        settings.endGroup();

        //if this discovers a config to use it makes it the default
        QNetworkConfiguration config = manager.configurationFromIdentifier(id);
        if ((config.state() & QNetworkConfiguration::Discovered) !=
                QNetworkConfiguration::Discovered) {
            config = manager.defaultConfiguration();
        }

        //Starts the network session with the used config
        networkSession = new QNetworkSession(config, this);
        connect(networkSession, &QNetworkSession::opened, this, &Server::sessionOpened);

        //Opens the connection
        statusLabel->setText(tr("Opening connection"));
        networkSession->open();

    //If nothing else works than start opening the session
    } else {
        sessionOpened();
    }

    //This sets up the button you see on the screen
    auto quitButton = new QPushButton(tr("QUIT"));
    quitButton->setAutoDefault(false);
    //connects the button to functions needed
    connect(quitButton, &QAbstractButton::clicked, this, &QWidget::close);

    //When a new connection is detected start to send data
    connect(server, &QTcpServer::newConnection, this, &Server::sendData);

    //Puts the button on the layout
    auto buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(quitButton);
    buttonLayout->addStretch(1);

    //Loads the entire very simple GUI that displays info
    QVBoxLayout *mainLayout = nullptr;
    if (QGuiApplication::styleHints()->showIsFullScreen() || QGuiApplication::styleHints()->showIsMaximized()) {
        auto outerVerticalLayout = new QVBoxLayout(this);
        outerVerticalLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::MinimumExpanding));
        auto outerHorizontalLayout = new QHBoxLayout;
        outerHorizontalLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Ignored));            auto groupBox = new QGroupBox(QGuiApplication::applicationDisplayName());
        mainLayout = new QVBoxLayout(groupBox);
        outerHorizontalLayout->addWidget(groupBox);
        outerHorizontalLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Ignored));            outerVerticalLayout->addLayout(outerHorizontalLayout);
        outerVerticalLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::MinimumExpanding));
    } else {
        mainLayout = new QVBoxLayout(this);
    }

    //Adds all the widgets to the GUI
    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(buttonLayout);

    //Gives the window a title
    setWindowTitle(QGuiApplication::applicationDisplayName());
}

/**
 * @brief Server::sessionOpened
 * This handles making sure the connection is going to run smoothly
 */
void Server::sessionOpened()
{
    //Starts to configure the network session
    if (networkSession) {
        QNetworkConfiguration config = networkSession->configuration();
        QString id;
        if (config.type() == QNetworkConfiguration::UserChoice) {
            id = networkSession->sessionProperty(QLatin1String("UserChoiceConfig")).toString();
        } else {
            id = config.identifier();
        }

        QSettings settings(QSettings::UserScope, QLatin1String("Hyperloop"));
        settings.beginGroup(QLatin1String("HyperloopNetwork"));
        settings.setValue(QLatin1String("DefaultConfig"), id);
        settings.endGroup();
    }

    //Server is listeneing for connections here
    server = new QTcpServer(this);
    if (!server->listen()) {
        QMessageBox::critical(this, tr("Hyperloop test server"),
                              tr("Unable to start server: %1").arg(server->errorString()));
        close();
        return;
    }


    //THIS CODE DISPLAYS THE IP TO CONNECT TO
    const QHostAddress &localHost = QHostAddress(QHostAddress::LocalHost);
    QString ipAddress;
    for (const QHostAddress &address: QNetworkInterface::allAddresses()) {
        if(address.protocol() == QAbstractSocket::IPv4Protocol && address != localHost) {
            ipAddress = address.toString();
        }
    }

    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();

    statusLabel->setText(tr("The server is running on\n\nIP: %1\nport: %2\n\nRun client now")
                         .arg(ipAddress).arg(server->serverPort()));
    statusLabel->resize(500, 500);
    statusLabel->update();
}

/**
 * @brief Server::sendData
 * This handles sending the data that we want to send to the client
 */
void Server::sendData()
{
    //creates the array we need to send
    QByteArray block;
    //defines what we are going to send out
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_13);
    //This function call generates the data we will send
    generateData();

    //Tells the computer what to send
    out << data[0];

    //This sets up the connection to the client
    QTcpSocket *clientConnection = server->nextPendingConnection();
    connect(clientConnection, &QAbstractSocket::disconnected,
            clientConnection, &QObject::deleteLater);
    clientConnection->write(block);
    clientConnection->disconnectFromHost();
    //clears the data we got because it is not needed anymore
    data.clear();
}

/**
 * @brief Server::generateData
 * This just generates random numbers to be the data
 */
void Server::generateData()
{
    int voltage = QRandomGenerator::global()->bounded(70) + 50;
    int amps = QRandomGenerator::global()->bounded(100) + 50;
    int speed = QRandomGenerator::global()->bounded(250) + amps/4;
    int rpm = QRandomGenerator::global()->bounded(2000) + 1000;
    data << tr("Voltage: %1\nAmps: %2\nSpeed: %3\nRPM: %4")
            .arg(voltage).arg(amps).arg(speed).arg(rpm);
}

