#pragma once

#include <QObject>
#include <QTimer>
#include <QUdpSocket>

// Broadcasts a UDP beacon so Android can auto-discover the WebSocket server.
// Packet: {"type":"beacon","ip":"<ip>","port":<wsPort>}
class UdpBeacon : public QObject {
    Q_OBJECT

public:
    explicit UdpBeacon(quint16 wsPort, QObject* parent = nullptr);
    ~UdpBeacon() override;

    void start();
    void stop();

private slots:
    void sendBeacon();

private:
    QUdpSocket m_socket;
    QTimer     m_timer;
    quint16    m_wsPort;
};
