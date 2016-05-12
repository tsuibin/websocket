#include "qwebchannel.h"

#include <QApplication>
#include <QDialog>
#include <QVariantMap>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QFileInfo>
#include <QtWebSockets/QWebSocketServer>

#include "websocketclientwrapper.h"
#include "websockettransport.h"

#include "ui_dialog.h"

#include <QDebug>
/*!
    An instance of this class gets published over the WebChannel and is then accessible to HTML clients.
*/
class Dialog : public QObject
{
    Q_OBJECT

public:
    explicit Dialog(QObject *parent = 0)
        : QObject(parent)
    {
        ui.setupUi(&dialog);
        dialog.show();

        connect(ui.send, SIGNAL(clicked()), SLOT(clicked()));
    }

    void displayMessage(const QString &message)
    {
        ui.output->appendPlainText(message);
    }

signals:
    /*!
        This signal is emitted from the C++ side and the text displayed on the HTML client side.
    */
    void sendText(const QString &text);

public slots:
    /*!
        This slot is invoked from the HTML client side and the text displayed on the server side.
    */
    void receiveText(const QString &text)
    {
        emit sendText(text);
        displayMessage(tr("Received message: %1").arg(text));
    }

private slots:
    /*!
        Note that this slot is private and thus not accessible to HTML clients.
    */
    void clicked()
    {
        const QString text = ui.input->text();

        if (text.isEmpty()) {
            return;
        }

        emit sendText(text);
        displayMessage(tr("Sent message: %1").arg(text));

        ui.input->clear();
    }

private:
    QDialog dialog;
    Ui::Dialog ui;
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

//    QFileInfo jsFileInfo(QDir::currentPath() + "/qwebchannel.js");

//    if (!jsFileInfo.exists())
//        QFile::copy(":/qtwebchannel/qwebchannel.js",jsFileInfo.absoluteFilePath());

    // setup the QWebSocketServer
    QWebSocketServer server(QStringLiteral("Station WebSocket Server 0.0.1"), QWebSocketServer::NonSecureMode);
    if (!server.listen(QHostAddress::Any, 12345)) {
        qFatal("Failed to open web socket server.");
        return 1;
    }

    // wrap WebSocket clients in QWebChannelAbstractTransport objects
    WebSocketClientWrapper clientWrapper(&server);

    // setup the channel
    QWebChannel channel;
    QObject::connect(&clientWrapper, &WebSocketClientWrapper::clientConnected,
                     &channel, &QWebChannel::connectTo);

    // setup the dialog and publish it to the QWebChannel
    Dialog dialog;
    channel.registerObject(QStringLiteral("dialog"), &dialog);

    // open a browser window with the client HTML page
    QUrl url = QUrl::fromLocalFile(BUILD_DIR "/index.html");
//    QDesktopServices::openUrl(url);

    dialog.displayMessage(QObject::tr("Initialization complete, opening browser at %1.").arg(url.toDisplayString()));

    return app.exec();
}

#include "main.moc"
