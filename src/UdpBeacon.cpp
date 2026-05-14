#include "UdpBeacon.h"
#include "AppConfig.h"

#include <QAbstractSocket>
#include <QDebug>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkInterface>

UdpBeacon::UdpBeacon(quint16 wsPort, QObject* parent)
    : QObject(parent), m_wsPort(wsPort)
{
    m_socket.bind(QHostAddress::AnyIPv4, 0, QUdpSocket::ShareAddress);
    m_socket.setSocketOption(QAbstractSocket::MulticastLoopbackOption, 0);
    connect(&m_timer, &QTimer::timeout, this, &UdpBeacon::sendBeacon);
}

UdpBeacon::~UdpBeacon() { stop(); }

void UdpBeacon::start() {
    sendBeacon();
    m_timer.start(AppConfig::instance().beaconIntervalMs);
    qDebug() << "[Beacon] Started on UDP port" << AppConfig::instance().udpBeaconPort;
}

void UdpBeacon::stop() {
    m_timer.stop();
}

void UdpBeacon::sendBeacon() {
    QString ip;
    for (const auto& iface : QNetworkInterface::allInterfaces()) {
        if (!(iface.flags() & QNetworkInterface::IsUp)) continue;
        if (iface.flags() & QNetworkInterface::IsLoopBack) continue;
        for (const auto& entry : iface.addressEntries()) {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                ip = entry.ip().toString();
                break;
            }
        }
        if (!ip.isEmpty()) break;
    }
    if (ip.isEmpty()) return;

    QJsonObject obj;
    obj["type"] = "beacon";
    obj["ip"]   = ip;
    obj["port"] = m_wsPort;
    const QByteArray payload = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    m_socket.writeDatagram(payload, QHostAddress::Broadcast,
                           static_cast<quint16>(AppConfig::instance().udpBeaconPort));
}
