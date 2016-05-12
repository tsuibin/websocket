#include "websocketclientwrapper.h"

#include <QtWebSockets/QWebSocketServer>

#include "websockettransport.h"



QT_BEGIN_NAMESPACE

WebSocketClientWrapper::WebSocketClientWrapper(QWebSocketServer *server, QObject *parent)
    : QObject(parent)
    , m_server(server)
{
    connect(server, &QWebSocketServer::newConnection,
            this, &WebSocketClientWrapper::handleNewConnection);
}


void WebSocketClientWrapper::handleNewConnection()
{
    emit clientConnected(new WebSocketTransport(m_server->nextPendingConnection()));
}

QT_END_NAMESPACE
