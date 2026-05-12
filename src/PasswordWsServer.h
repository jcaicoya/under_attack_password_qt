#pragma once
#include <QObject>
#include <QWebSocketServer>
#include <QList>

class QWebSocket;

class PasswordWsServer : public QObject {
    Q_OBJECT
public:
    explicit PasswordWsServer(quint16 port, QObject* parent = nullptr);
    ~PasswordWsServer() override;

    bool isListening() const;
    void sendVerdict(bool cracked, const QString& password = {});

signals:
    void passwordReceived(const QString& password);
    void clientConnected();
    void clientDisconnected();

private:
    QWebSocketServer* m_server;
    QList<QWebSocket*> m_clients;
};
