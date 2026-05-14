#include "PasswordWsServer.h"
#include "AppConfig.h"

#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

PasswordWsServer::PasswordWsServer(QObject* parent)
    : QObject(parent)
    , m_server(new QWebSocketServer("password", QWebSocketServer::NonSecureMode, this))
{
    m_port = static_cast<quint16>(AppConfig::instance().wsPort);
    if (m_server->listen(QHostAddress::Any, m_port)) {
        qDebug() << "[WS] Server listening on port" << m_port;
        connect(m_server, &QWebSocketServer::newConnection,
                this, &PasswordWsServer::onNewConnection);
    } else {
        qWarning() << "[WS] Failed to start server:" << m_server->errorString();
    }

    connect(&m_heartbeatTimer, &QTimer::timeout, this, &PasswordWsServer::onHeartbeatTick);
}

PasswordWsServer::~PasswordWsServer() {
    disconnectClient();
    m_server->close();
}

void PasswordWsServer::sendVerdict(bool cracked, const QString& password) {
    if (!m_client) return;
    QJsonObject obj;
    obj["type"]    = "verdict";
    obj["cracked"] = cracked;
    if (cracked) obj["password"] = password;
    m_client->sendTextMessage(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void PasswordWsServer::onNewConnection() {
    QWebSocket* pending = m_server->nextPendingConnection();
    if (!pending) return;

    qDebug() << "[WS] Incoming connection from" << pending->peerAddress().toString();

    if (m_client) {
        qDebug() << "[WS] Replacing stale client";
        disconnectClient();
        emit clientConnected(false);
    }

    m_client = pending;
    connect(m_client, &QWebSocket::textMessageReceived,
            this, &PasswordWsServer::onTextMessageReceived);
    connect(m_client, &QWebSocket::disconnected,
            this, &PasswordWsServer::onClientDisconnected);

    m_lastPongMs = QDateTime::currentMSecsSinceEpoch();
    m_heartbeatTimer.start(AppConfig::instance().heartbeatIntervalMs);
    qDebug() << "[WS] Client accepted";
    emit clientConnected(true);
}

void PasswordWsServer::onTextMessageReceived(const QString& msg) {
    const QJsonObject obj = QJsonDocument::fromJson(msg.toUtf8()).object();
    const QString type = obj.value("type").toString();

    if (type == "pong") {
        m_lastPongMs = QDateTime::currentMSecsSinceEpoch();
        return;
    }
    if (type == "password") {
        emit passwordReceived(obj.value("value").toString());
        return;
    }
    qDebug() << "[WS] Unknown message:" << msg;
}

void PasswordWsServer::onClientDisconnected() {
    qDebug() << "[WS] Client disconnected";
    disconnectClient();
    emit clientConnected(false);
}

void PasswordWsServer::onHeartbeatTick() {
    if (!m_client) return;

    const qint64 silent = QDateTime::currentMSecsSinceEpoch() - m_lastPongMs;
    if (silent > AppConfig::instance().pongTimeoutMs) {
        qDebug() << "[WS] Pong timeout, disconnecting client";
        disconnectClient();
        emit clientConnected(false);
        return;
    }

    m_client->sendTextMessage(R"({"type":"ping"})");
}

void PasswordWsServer::disconnectClient() {
    if (!m_client) return;
    m_heartbeatTimer.stop();
    m_client->disconnect(this);
    m_client->close();
    m_client->deleteLater();
    m_client = nullptr;
}
