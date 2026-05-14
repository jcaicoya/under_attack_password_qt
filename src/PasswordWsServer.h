#pragma once

#include <QDateTime>
#include <QObject>
#include <QTimer>
#include <QWebSocket>
#include <QWebSocketServer>

class PasswordWsServer : public QObject {
    Q_OBJECT

public:
    explicit PasswordWsServer(QObject* parent = nullptr);
    ~PasswordWsServer() override;

    bool    isClientConnected() const { return m_client != nullptr; }
    quint16 port()              const { return m_port; }

    void sendVerdict(bool cracked, const QString& password = {});

signals:
    void clientConnected(bool connected);
    void passwordReceived(const QString& password);

private slots:
    void onNewConnection();
    void onTextMessageReceived(const QString& msg);
    void onClientDisconnected();
    void onHeartbeatTick();

private:
    void disconnectClient();

    quint16           m_port        = 8767;
    QWebSocketServer* m_server      = nullptr;
    QWebSocket*       m_client      = nullptr;
    QTimer            m_heartbeatTimer;
    qint64            m_lastPongMs  = 0;
};
