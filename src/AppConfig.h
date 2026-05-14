#pragma once

struct AppConfig {
    int wsPort              = 8767;
    int udpBeaconPort       = 8768;
    int heartbeatIntervalMs = 5000;
    int pongTimeoutMs       = 10000;
    int beaconIntervalMs    = 2000;

    static AppConfig& instance();
};
