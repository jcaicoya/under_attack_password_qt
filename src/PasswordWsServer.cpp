#include "PasswordWsServer.h"
#include <QWebSocket>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>

PasswordWsServer::PasswordWsServer(quint16 port, QObject* parent)
    : QObject(parent)
    , m_server(new QWebSocketServer("password", QWebSocketServer::NonSecureMode, this))
{
    if (!m_server->listen(QHostAddress::LocalHost, port)) {
        qWarning() << "[WS] Failed to listen on port" << port;
        return;
    }
    qDebug() << "[WS] Listening on localhost:" << port;

    connect(m_server, &QWebSocketServer::newConnection, this, [this]() {
        auto* ws = m_server->nextPendingConnection();
        m_clients.append(ws);
        emit clientConnected();

        connect(ws, &QWebSocket::textMessageReceived, this, [this](const QString& msg) {
            const auto obj = QJsonDocument::fromJson(msg.toUtf8()).object();
            if (obj.value("type").toString() == "password") {
                emit passwordReceived(obj.value("value").toString());
            }
        });
        connect(ws, &QWebSocket::disconnected, this, [this, ws]() {
            m_clients.removeAll(ws);
            ws->deleteLater();
            emit clientDisconnected();
        });
    });
}

PasswordWsServer::~PasswordWsServer() {
    for (auto* ws : m_clients) ws->close();
    m_server->close();
}

bool PasswordWsServer::isListening() const {
    return m_server->isListening();
}

void PasswordWsServer::sendVerdict(bool cracked, const QString& password) {
    QJsonObject obj;
    obj["type"]    = "verdict";
    obj["cracked"] = cracked;
    if (cracked) obj["password"] = password;
    const QString msg = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    for (auto* ws : m_clients) ws->sendTextMessage(msg);
}
